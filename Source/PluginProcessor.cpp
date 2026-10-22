// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#include "PluginProcessor.h"

#include "DX7/Banks.h"
#include "PluginEditor.h"

Mz7Processor::Mz7Processor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Mz7", params::createLayout()),
      reader(state)
{
    selectBuiltIn(0);
}

Mz7Processor::~Mz7Processor()
{
    cancelPendingUpdate();
}

void Mz7Processor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate);
    reader.readPatch(playing);
    engine.setPatch(playing);
    engine.setSettings(reader.readSettings());
    keyboardState.reset();
}

bool Mz7Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void Mz7Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer(midi, 0, n, true);

    // The voice as the knobs have it now. Held notes follow an edit, as on the DX7.
    dx7::Patch now = playing;
    reader.readPatch(now);
    if (now != playing)
    {
        playing = now;
        engine.setPatch(playing);
    }
    engine.setSettings(reader.readSettings());

    buffer.clear();
    if (buffer.getNumChannels() == 0) return;

    // The DX7 is mono: render one channel, splitting the block at each MIDI event so a note
    // lands on its sample.
    float* out = buffer.getWritePointer(0);
    int pos = 0;

    for (const auto meta : midi)
    {
        const int at = juce::jlimit(0, n, meta.samplePosition);
        if (at > pos)
        {
            engine.render(out + pos, at - pos);
            pos = at;
        }
        handle(meta.getMessage());
    }
    if (pos < n) engine.render(out + pos, n - pos);

    buffer.applyGain(0, 0, n, juce::Decibels::decibelsToGain(reader.volumeDb(), -40.0f));

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom(ch, 0, buffer, 0, 0, n);
}

void Mz7Processor::handle(const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        engine.noteOn(m.getNoteNumber(), m.getVelocity());
    else if (m.isNoteOff())
        engine.noteOff(m.getNoteNumber());
    else if (m.isPitchWheel())
        engine.pitchBend(m.getPitchWheelValue() - 8192);
    else if (m.isChannelPressure())
        engine.aftertouch(m.getChannelPressureValue());
    else if (m.isAftertouch())
        engine.aftertouch(m.getAfterTouchValue());
    else if (m.isAllNotesOff())
        engine.allNotesOff();
    else if (m.isAllSoundOff())
        engine.allSoundOff();
    else if (m.isController())
        engine.controller(m.getControllerNumber(), m.getControllerValue());
    else if (m.isProgramChange())
    {
        // the cartridge's slots, 0-31; the parameters move on the message thread
        pendingProgram = m.getProgramChangeNumber() & 31;
        triggerAsyncUpdate();
    }
    else if (m.isSysEx())
    {
        // A DX7 voice or cartridge dump, from a librarian or another DX7.
        const auto* d = m.getSysExData();
        const int size = m.getSysExDataSize();
        if (size > 4 && d[0] == 0x43)
        {
            std::unique_lock<std::mutex> lock(sysexLock, std::try_to_lock);
            if (lock.owns_lock() && pendingSysex.empty())
            {
                pendingSysex.reserve((size_t) size + 2);
                pendingSysex.push_back(0xF0);
                pendingSysex.insert(pendingSysex.end(), d, d + size);
                pendingSysex.push_back(0xF7);
                triggerAsyncUpdate();
            }
        }
    }
}

void Mz7Processor::handleAsyncUpdate()
{
    const int p = pendingProgram.exchange(-1);
    if (p >= 0) setCurrentProgram(p);

    std::vector<uint8_t> dump;
    {
        std::lock_guard<std::mutex> lock(sysexLock);
        dump.swap(pendingSysex);
    }
    if (! dump.empty())
    {
        dx7::Cartridge c = cart;
        int count = 0;
        std::string error, warning;
        if (dx7::readSysex(dump.data(), dump.size(), c, count, error, warning))
        {
            if (count == 32)
                adoptCartridge(c, "MIDI DUMP", -1);
            else
            {
                // a single voice becomes the voice being edited, as on the DX7
                params::writePatch(c.voices[0], state);
                setVoiceName(juce::String(c.voices[0].name()));
            }
        }
    }
}

// ------------------------------------------------------------------------------ programs

void Mz7Processor::setCurrentProgram(int index)
{
    if (index < 0 || index >= 32) return;
    program = index;

    const auto& v = cart.voices[(size_t) index];
    params::writePatch(v, state);
    setVoiceName(juce::String(v.name()));
    updateHostDisplay(ChangeDetails().withProgramChanged(true));
}

const juce::String Mz7Processor::getProgramName(int index)
{
    if (index < 0 || index >= 32) return {};
    return juce::String(index + 1).paddedLeft('0', 2) + " " + juce::String(cart.voices[(size_t) index].name()).trimEnd();
}

void Mz7Processor::adoptCartridge(const dx7::Cartridge& c, const juce::String& title, int builtIn)
{
    cart = c;
    cartName = title;
    bank = builtIn;
    setCurrentProgram(0);
    updateHostDisplay(ChangeDetails().withProgramChanged(true).withParameterInfoChanged(true));
    ++changes;
}

void Mz7Processor::selectBuiltIn(int index)
{
    index = juce::jlimit(0, banks::count() - 1, index);
    adoptCartridge(banks::get(index), juce::String(banks::name(index)), index);
}

bool Mz7Processor::loadSyx(const juce::File& file, juce::String& message)
{
    juce::MemoryBlock data;
    if (! file.loadFileAsData(data))
    {
        message = "could not read " + file.getFileName();
        return false;
    }

    dx7::Cartridge c;
    int count = 0;
    std::string error, warning;
    if (! dx7::readSysex(static_cast<const uint8_t*>(data.getData()), data.getSize(), c, count, error, warning))
    {
        message = juce::String(error);
        return false;
    }

    if (count == 1)
    {
        // a single voice: into the voice being edited, the cartridge untouched
        params::writePatch(c.voices[0], state);
        setVoiceName(juce::String(c.voices[0].name()));
        message = juce::String(warning);
        return true;
    }

    adoptCartridge(c, file.getFileNameWithoutExtension().toUpperCase(), -1);
    message = juce::String(warning);
    return true;
}

bool Mz7Processor::saveCartridge(const juce::File& file) const
{
    const auto s = dx7::cartridgeSysex(cart);
    return file.replaceWithData(s.data(), s.size());
}

bool Mz7Processor::saveVoice(const juce::File& file) const
{
    const auto s = dx7::voiceSysex(currentVoice());
    return file.replaceWithData(s.data(), s.size());
}

// --------------------------------------------------------------------------------- voice

juce::String Mz7Processor::voiceName() const
{
    auto& self = const_cast<Mz7Processor&>(*this);
    std::lock_guard<std::mutex> lock(self.nameLock);
    return juce::String(name, 10);
}

void Mz7Processor::setVoiceName(const juce::String& n)
{
    {
        std::lock_guard<std::mutex> lock(nameLock);
        for (int i = 0; i < 10; ++i)
        {
            const juce::juce_wchar c = i < n.length() ? n[i] : ' ';
            name[i] = (c >= 32 && c < 127) ? (char) c : ' ';
        }
    }
    ++changes;
}

dx7::Patch Mz7Processor::currentVoice() const
{
    dx7::Patch p = dx7::Patch::init();
    reader.readPatch(p);
    p.setName(voiceName().toStdString());
    return p;
}

void Mz7Processor::store(int slot)
{
    if (slot < 0 || slot >= 32) return;
    cart.voices[(size_t) slot] = currentVoice();
    program = slot;
    if (bank >= 0) cartName = cartName + " (edited)";
    bank = -1;
    updateHostDisplay(ChangeDetails().withProgramChanged(true));
    ++changes;
}

void Mz7Processor::initVoice()
{
    const auto p = dx7::Patch::init();
    params::writePatch(p, state);
    setVoiceName(juce::String(p.name()));
}

// --------------------------------------------------------------------------------- state

void Mz7Processor::getStateInformation(juce::MemoryBlock& destData)
{
    auto copy = state.copyState();

    // the cartridge rides along as its own sysex, so a project keeps an edited cartridge too
    const auto s = dx7::cartridgeSysex(cart);
    copy.setProperty("cartridge", juce::Base64::toBase64(s.data(), s.size()), nullptr);
    copy.setProperty("cartridgeName", cartName, nullptr);
    copy.setProperty("bank", bank, nullptr);
    copy.setProperty("program", program, nullptr);
    copy.setProperty("voiceName", voiceName(), nullptr);

    if (auto xml = copy.createXml())
        copyXmlToBinary(*xml, destData);
}

void Mz7Processor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName(state.state.getType())) return;

    auto tree = juce::ValueTree::fromXml(*xml);

    juce::MemoryOutputStream bytes;
    if (juce::Base64::convertFromBase64(bytes, tree.getProperty("cartridge").toString()))
    {
        dx7::Cartridge c;
        int count = 0;
        std::string error, warning;
        if (dx7::readSysex(static_cast<const uint8_t*>(bytes.getData()), bytes.getDataSize(), c, count, error, warning)
            && count == 32)
            cart = c;
    }

    cartName = tree.getProperty("cartridgeName", cartName).toString();
    bank     = tree.getProperty("bank", -1);
    program  = juce::jlimit(0, 31, (int) tree.getProperty("program", 0));

    state.replaceState(tree);
    setVoiceName(tree.getProperty("voiceName", juce::String(cart.voices[(size_t) program].name())).toString());
    updateHostDisplay(ChangeDetails().withProgramChanged(true).withParameterInfoChanged(true));
}

juce::AudioProcessorEditor* Mz7Processor::createEditor()
{
    return new Mz7Editor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Mz7Processor();
}
