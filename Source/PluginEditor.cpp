// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#include "PluginEditor.h"

#include "DX7/Algorithms.h"
#include "DX7/Banks.h"

#include <cmath>

// ---------------------------------------------------------------------------------- look

const juce::Colour PanelLook::background (0xff1c1917);
const juce::Colour PanelLook::panel      (0xff29231f);
const juce::Colour PanelLook::edge       (0xff4a3f38);
const juce::Colour PanelLook::text       (0xffeadfd2);
const juce::Colour PanelLook::dim        (0xff9a8b7d);
const juce::Colour PanelLook::value      (0xff4cc3b0);   // a DX-ish teal
const juce::Colour PanelLook::carrier    (0xffc8553d);   // and its maroon-red
const juce::Colour PanelLook::box        (0xff120f0e);

PanelLook::PanelLook()
{
    setColour(juce::ComboBox::backgroundColourId, box);
    setColour(juce::ComboBox::outlineColourId, edge);
    setColour(juce::ComboBox::textColourId, value);
    setColour(juce::ComboBox::arrowColourId, dim);
    setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, value.withAlpha(0.3f));
    setColour(juce::TextButton::buttonColourId, panel.brighter(0.15f));
    setColour(juce::TextButton::textColourOffId, text);
    setColour(juce::TextEditor::backgroundColourId, box);
    setColour(juce::TextEditor::textColourId, value);
    setColour(juce::TextEditor::outlineColourId, edge);
    setColour(juce::TextEditor::focusedOutlineColourId, value);
    setColour(juce::TooltipWindow::backgroundColourId, panel.brighter(0.1f));
    setColour(juce::TooltipWindow::textColourId, text);
    setColour(juce::TooltipWindow::outlineColourId, edge);
    setColour(juce::HyperlinkButton::textColourId, value);
}

// A value box, drawn in place of a knob: the number, and a hairline of how far along it is.
void PanelLook::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float, float, float,
                                 juce::Slider::SliderStyle, juce::Slider& s)
{
    const auto r = juce::Rectangle<int>(x, y, w, h).toFloat().reduced(0.5f);
    g.setColour(box);
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(s.isMouseOverOrDragging() ? value.withAlpha(0.8f) : edge);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);

    const double range = s.getMaximum() - s.getMinimum();
    const float frac = range > 0 ? (float) ((s.getValue() - s.getMinimum()) / range) : 0.0f;
    g.setColour(value.withAlpha(0.35f));
    g.fillRect(r.getX() + 2.0f, r.getBottom() - 3.0f, (r.getWidth() - 4.0f) * frac, 1.5f);

    g.setColour(value);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawFittedText(s.getTextFromValue(s.getValue()), r.toNearestInt().reduced(2, 0), juce::Justification::centred, 1, 0.55f);
}

void PanelLook::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& c, bool hi, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(down ? c.brighter(0.2f) : hi ? c.brighter(0.1f) : c);
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);
}

void PanelLook::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool hi, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced(1.0f);
    const bool on = b.getToggleState();
    g.setColour(on ? carrier : box);
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(hi ? text : edge);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);
    g.setColour(on ? text : dim);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(b.getButtonText(), r, juce::Justification::centred);
}

// --------------------------------------------------------------------------------- field

Field::Field(Mz7Processor& p, const juce::String& id, const juce::String& cap, const juce::String& hint)
    : caption(cap)
{
    // A box rather than a knob: drag up or down, or use the wheel. Double-click for the default.
    slider.setSliderStyle(juce::Slider::LinearBarVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setSliderSnapsToMousePosition(false);
    slider.setVelocityBasedMode(true);
    slider.setVelocityModeParameters(0.35, 1, 0.0, false);
    slider.setTooltip(hint);
    addAndMakeVisible(slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state, id, slider);
    if (auto* param = p.state.getParameter(id))
        slider.setDoubleClickReturnValue(true, param->convertFrom0to1(param->getDefaultValue()));
}

void Field::paint(juce::Graphics& g)
{
    g.setColour(PanelLook::dim);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawFittedText(caption, getLocalBounds().removeFromTop(captionHeight), juce::Justification::centred, 1, 0.6f);
}

void Field::resized()
{
    slider.setBounds(getLocalBounds().withTrimmedTop(captionHeight).withHeight(boxHeight));
}

// ------------------------------------------------------------------------------ envelope

EnvelopeView::EnvelopeView(Mz7Processor& p, std::array<juce::String, 8> ids, bool isPitch)
    : processor(p), pitch(isPitch)
{
    for (size_t i = 0; i < 8; ++i) values[i] = p.state.getRawParameterValue(ids[i]);
    setInterceptsMouseClicks(false, false);
}

void EnvelopeView::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(PanelLook::box);
    g.fillRoundedRectangle(r, 3.0f);
    g.setColour(PanelLook::edge);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);
    r = r.reduced(5.0f, 6.0f);

    float rate[4], level[4];
    for (int i = 0; i < 4; ++i)
    {
        rate[i]  = values[(size_t) i]->load();
        level[i] = values[(size_t) (i + 4)]->load();
    }

    // How long each stage looks: slow rates wide, fast ones narrow. A sketch, not a clock.
    auto span = [] (float r) { return 0.05f + std::pow((99.0f - r) / 99.0f, 1.5f); };
    const float hold = 0.45f;
    const float total = span(rate[0]) + span(rate[1]) + span(rate[2]) + hold + span(rate[3]);
    const float sx = r.getWidth() / total;
    auto yOf = [&] (float l) { return r.getBottom() - r.getHeight() * l / 99.0f; };

    if (pitch)
    {
        g.setColour(PanelLook::edge);
        g.drawHorizontalLine((int) yOf(50.0f), r.getX(), r.getRight());
    }

    juce::Path path;
    float x = r.getX();
    path.startNewSubPath(x, yOf(level[3]));
    for (int i = 0; i < 3; ++i)
    {
        x += span(rate[i]) * sx;
        path.lineTo(x, yOf(level[i]));
    }
    x += hold * sx;
    path.lineTo(x, yOf(level[2]));
    x += span(rate[3]) * sx;
    path.lineTo(x, yOf(level[3]));

    g.setColour(PanelLook::value);
    g.strokePath(path, juce::PathStrokeType(1.6f));

    // the key-off point
    const float off = r.getX() + (span(rate[0]) + span(rate[1]) + span(rate[2]) + hold) * sx;
    g.setColour(PanelLook::dim.withAlpha(0.5f));
    g.drawVerticalLine((int) off, r.getY(), r.getBottom());
}

// ----------------------------------------------------------------------------- algorithm

AlgorithmView::AlgorithmView(Mz7Processor& p) : processor(p)
{
    setInterceptsMouseClicks(false, false);
}

void AlgorithmView::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour(PanelLook::box);
    g.fillRoundedRectangle(area.reduced(1.0f), 4.0f);
    g.setColour(PanelLook::edge);
    g.drawRoundedRectangle(area.reduced(1.0f), 4.0f, 1.0f);

    const int alg = juce::jlimit(0, 31, (int) processor.state.getRawParameterValue("alg")->load());
    const int fb  = (int) processor.state.getRawParameterValue("fb")->load();
    const auto& a = dx7::algorithms[alg];

    // A tree: each modulator is drawn above the lowest-numbered operator it modulates; one
    // that modulates several still has a line to each.
    int owner[7] {};
    for (int m = 1; m <= 6; ++m)
        for (int t = 1; t <= 6 && ! owner[m]; ++t)
            if ((a.mods[t] >> m) & 1) owner[m] = t;

    std::function<int(int)> width = [&] (int op)
    {
        int w = 0;
        for (int c = 1; c <= 6; ++c) if (owner[c] == op) w += width(c);
        return juce::jmax(1, w);
    };

    float cx[7] {}, cy[7] {};
    int columns = 0;
    for (int op = 1; op <= 6; ++op) if ((a.carriers >> op) & 1) columns += width(op);

    const float boxW = 30.0f, boxH = 22.0f;
    const auto inner = area.reduced(14.0f, 12.0f).withTrimmedTop(14.0f).withTrimmedBottom(10.0f);
    const float colW = inner.getWidth() / (float) juce::jmax(1, columns);
    const float rowH = juce::jmin(46.0f, (inner.getHeight() - boxH) / 5.0f);

    std::function<void(int, float, int)> place = [&] (int op, float x0, int depth)
    {
        cx[op] = x0 + width(op) * colW / 2.0f;
        cy[op] = inner.getBottom() - boxH / 2.0f - depth * rowH;
        float x = x0;
        for (int c = 1; c <= 6; ++c)
            if (owner[c] == op) { place(c, x, depth + 1); x += width(c) * colW; }
    };

    float x0 = inner.getX();
    for (int op = 1; op <= 6; ++op)
        if ((a.carriers >> op) & 1) { place(op, x0, 0); x0 += width(op) * colW; }

    // lines: modulator to everything it modulates, carriers to the output bus
    g.setColour(PanelLook::dim);
    for (int t = 1; t <= 6; ++t)
        for (int m = 1; m <= 6; ++m)
            if ((a.mods[t] >> m) & 1)
                g.drawLine(cx[m], cy[m] + boxH / 2.0f, cx[t], cy[t] - boxH / 2.0f, 1.4f);

    const float busY = inner.getBottom() + 6.0f;
    float left = 1e9f, right = -1e9f;
    for (int op = 1; op <= 6; ++op)
        if ((a.carriers >> op) & 1)
        {
            g.drawLine(cx[op], cy[op] + boxH / 2.0f, cx[op], busY, 1.4f);
            left = juce::jmin(left, cx[op]); right = juce::jmax(right, cx[op]);
        }
    g.drawLine(left, busY, right, busY, 1.4f);

    // feedback: a loop from the operator fed back to the one it feeds
    {
        const int from = a.fbFrom, to = a.fbTo;
        juce::Path loop;
        const float side = juce::jmax(cx[from], cx[to]) + boxW / 2.0f + 7.0f;
        loop.startNewSubPath(cx[from], cy[from] + boxH / 2.0f);
        loop.lineTo(cx[from], cy[from] + boxH / 2.0f + 5.0f);
        loop.lineTo(side, cy[from] + boxH / 2.0f + 5.0f);
        loop.lineTo(side, cy[to] - boxH / 2.0f - 5.0f);
        loop.lineTo(cx[to], cy[to] - boxH / 2.0f - 5.0f);
        loop.lineTo(cx[to], cy[to] - boxH / 2.0f);
        g.setColour(fb > 0 ? PanelLook::value : PanelLook::edge);
        g.strokePath(loop, juce::PathStrokeType(1.4f));
    }

    for (int op = 1; op <= 6; ++op)
    {
        const bool car = (a.carriers >> op) & 1;
        const auto b = juce::Rectangle<float>(boxW, boxH).withCentre({ cx[op], cy[op] });
        g.setColour(car ? PanelLook::carrier : PanelLook::panel.brighter(0.15f));
        g.fillRoundedRectangle(b, 3.0f);
        g.setColour(car ? PanelLook::text : PanelLook::value);
        g.drawRoundedRectangle(b, 3.0f, 1.0f);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(juce::String(op), b, juce::Justification::centred);
    }

    g.setColour(PanelLook::dim);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText("ALGORITHM " + juce::String(alg + 1), area.reduced(10.0f, 6.0f), juce::Justification::topLeft);
}

// ------------------------------------------------------------------------------ operator

namespace
{
    std::array<juce::String, 8> envelopeIds(const juce::String& prefix, bool pitch)
    {
        std::array<juce::String, 8> ids;
        for (int i = 0; i < 4; ++i)
        {
            ids[(size_t) i]     = prefix + (pitch ? "PR" : "R") + juce::String(i + 1);
            ids[(size_t) i + 4] = prefix + (pitch ? "PL" : "L") + juce::String(i + 1);
        }
        return ids;
    }
}

OperatorRow::OperatorRow(Mz7Processor& p, int n)
    : processor(p), op(n), envelope(p, envelopeIds("op" + juce::String(n), false), false)
{
    onSwitch.setButtonText("ON");
    onSwitch.setTooltip("Switch this operator off to hear the voice without it. Not part of the voice.");
    addAndMakeVisible(onSwitch);
    onAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.state, "op" + juce::String(n) + "On", onSwitch);

    field("Mode", "MODE", "RATIO follows the key; FIXED stays at one frequency whatever is played.");
    field("Coarse", "COARSE", "Ratio: 0 is 0.5, then 1 to 31. Fixed: 0-3 is 1, 10, 100 or 1000 Hz.");
    field("Fine", "FINE", "Ratio: adds this percentage. Fixed: multiplies by 10^(fine/100).");
    field("Detune", "DETUNE", "A few cents either way, for a chorus between operators.");
    field("OL", "LEVEL", "Output level. On a carrier, loudness; on a modulator, brightness - every 8 halves the modulation.");
    field("KVS", "VELOCITY", "How much a harder key raises this operator's level.");
    field("AMS", "AM SENS", "How far the LFO's amplitude modulation and EG bias reach this operator.");
    field("RS", "RATE SCL", "Higher keys run this operator's envelope faster.");
    for (int i = 1; i <= 4; ++i) field(("R" + juce::String(i)).toRawUTF8(), ("R" + juce::String(i)).toRawUTF8(),
                                       "Envelope rate: 99 is instant, lower is slower.");
    for (int i = 1; i <= 4; ++i) field(("L" + juce::String(i)).toRawUTF8(), ("L" + juce::String(i)).toRawUTF8(),
                                       "Envelope level: L1-L3 while held, L4 where it rests.");
    field("BP", "BREAK PT", "Keyboard level scaling: the key either side of which the depths apply.");
    field("LD", "L DEPTH", "How far the level changes below the break point.");
    field("RD", "R DEPTH", "How far the level changes above the break point.");
    field("LC", "L CURVE", "Below the break point: falling or rising, linearly or exponentially.");
    field("RC", "R CURVE", "Above the break point: falling or rising, linearly or exponentially.");

    addAndMakeVisible(envelope);
    refresh();
}

Field& OperatorRow::field(const char* suffix, const char* caption, const char* hint)
{
    fields.push_back(std::make_unique<Field>(processor, "op" + juce::String(op) + suffix, caption, hint));
    addAndMakeVisible(*fields.back());
    return *fields.back();
}

void OperatorRow::refresh()
{
    auto v = [this] (const char* s) { return (int) processor.state.getRawParameterValue("op" + juce::String(op) + s)->load(); };
    const int coarse = v("Coarse"), fine = v("Fine");

    juce::String f;
    if (v("Mode") != 0)
    {
        const double hz = std::pow(10.0, coarse & 3) * std::pow(10.0, fine / 100.0);
        f = (hz < 100 ? juce::String(hz, 3) : hz < 1000 ? juce::String(hz, 1) : juce::String(juce::roundToInt(hz))) + " Hz";
    }
    else
        f = juce::String::charToString(0x00d7) + juce::String((coarse == 0 ? 0.5 : coarse) * (1.0 + fine / 100.0), 2);

    const int alg = juce::jlimit(0, 31, (int) processor.state.getRawParameterValue("alg")->load());
    const bool car = (dx7::algorithms[alg].carriers >> op) & 1;

    if (f != frequency || car != carrier)
    {
        frequency = f;
        carrier = car;
        repaint();
    }
    envelope.repaint();
}

void OperatorRow::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour(PanelLook::panel);
    g.fillRoundedRectangle(r, 5.0f);
    g.setColour(PanelLook::edge);
    g.drawRoundedRectangle(r.reduced(0.5f), 5.0f, 1.0f);

    g.setColour(carrier ? PanelLook::carrier : PanelLook::value);
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("OP" + juce::String(op), 10, 8, 60, 26, juce::Justification::left);
    g.setColour(PanelLook::dim);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText(carrier ? "CARRIER" : "MODULATOR", 10, 34, 70, 14, juce::Justification::left);

    g.setColour(PanelLook::text);
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText(frequency, 84, getHeight() - 36, 200, 24, juce::Justification::centredLeft);

    // group captions
    g.setColour(PanelLook::dim.withAlpha(0.8f));
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.drawText("FREQUENCY", 84, 2, 200, 12, juce::Justification::left);
    g.drawText("LEVEL", 300, 2, 200, 12, juce::Justification::left);
    g.drawText("ENVELOPE", 512, 2, 200, 12, juce::Justification::left);
    g.drawText("KEY SCALING", 846, 2, 150, 12, juce::Justification::left);
}

void OperatorRow::resized()
{
    const int fw = 48, gap = 4, row1 = 14, row2 = row1 + Field::captionHeight + Field::boxHeight + 6;
    const int fh = Field::captionHeight + Field::boxHeight;
    auto place = [&] (int index, int x, int y, int w = 48) { fields[(size_t) index]->setBounds(x, y, w, fh); };

    onSwitch.setBounds(12, getHeight() - 30, 44, 20);

    int x = 84;
    for (int i = 0; i < 4; ++i) place(i, x + i * (fw + gap), row1, i == 0 ? 52 : 48);      // mode coarse fine detune
    x = 300;
    for (int i = 0; i < 4; ++i) place(4 + i, x + i * (fw + gap), row1);                    // level velocity ams rs

    envelope.setBounds(512, row1, 116, row2 + fh - row1);
    x = 634;
    for (int i = 0; i < 4; ++i) place(8 + i, x + i * (fw + gap), row1);                    // rates
    for (int i = 0; i < 4; ++i) place(12 + i, x + i * (fw + gap), row2);                   // levels

    x = 846;
    place(16, x, row1, 48); place(17, x + 51, row1, 48); place(18, x + 102, row1, 48);    // bp ld rd
    place(19, x, row2, 48); place(20, x + 51, row2, 48);                                  // lc rc
}

// --------------------------------------------------------------------------------- group

void Group::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour(PanelLook::panel);
    g.fillRoundedRectangle(r, 5.0f);
    g.setColour(PanelLook::edge);
    g.drawRoundedRectangle(r.reduced(0.5f), 5.0f, 1.0f);
    g.setColour(PanelLook::text);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(title, 10, 5, getWidth() - 20, 13, juce::Justification::left);
}

// --------------------------------------------------------------------------------- panel

Panel::Panel(Mz7Processor& p)
    : processor(p),
      algorithm(p),
      pitchEnvelope(p, envelopeIds("", true), true),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&look);

    for (auto* c : std::initializer_list<juce::Component*> { &cartridge, &programs, &loadButton, &saveButton,
                                                             &storeButton, &initButton, &nameEditor, &link,
                                                             &voiceGroup, &pitchGroup, &lfoGroup, &functionGroup,
                                                             &controllerGroup, &keyboard })
        addAndMakeVisible(c);

    voiceGroup.addAndMakeVisible(algorithm);
    pitchGroup.addAndMakeVisible(pitchEnvelope);

    cartridge.setTooltip("The cartridge: one of the four that come with Mz7, or a .syx you loaded.");
    cartridge.onChange = [this]
    {
        const int id = cartridge.getSelectedId();
        if (id >= 1 && id <= banks::count() && id - 1 != processor.builtInBank()) processor.selectBuiltIn(id - 1);
    };

    programs.setTooltip("The cartridge's 32 voices. Choosing one discards an edit you have not stored.");
    programs.onChange = [this]
    {
        const int i = programs.getSelectedItemIndex();
        if (i >= 0) processor.setCurrentProgram(i);   // the same one again reloads it, dropping an edit
    };

    loadButton.setTooltip("Open a DX7 cartridge (32 voices) or a single voice from a .syx file.");
    loadButton.onClick = [this] { loadSyx(); };
    saveButton.setTooltip("Save the cartridge, or just this voice, as a .syx a DX7 or any DX7 editor will read.");
    saveButton.onClick = [this] { saveMenu(); };
    storeButton.setTooltip("Write this voice, edits and all, into one of the cartridge's 32 slots.");
    storeButton.onClick = [this] { storeMenu(); };
    initButton.setTooltip("Start a new voice from INIT VOICE: one operator, a sine.");
    initButton.onClick = [this] { processor.initVoice(); };

    nameEditor.setInputRestrictions(10);
    nameEditor.setJustification(juce::Justification::centred);
    nameEditor.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    nameEditor.setTooltip("The voice's name: ten characters, as on the DX7.");
    nameEditor.onTextChange = [this] { processor.setVoiceName(nameEditor.getText().toUpperCase()); };

    link.setButtonText("github.com/simozzer/Mz7  v" JucePlugin_VersionString);
    link.setURL(juce::URL("https://github.com/simozzer/Mz7"));
    link.setFont(juce::FontOptions(13.0f, juce::Font::underlined), false, juce::Justification::centredRight);

    for (int op = 1; op <= 6; ++op)
    {
        operators.push_back(std::make_unique<OperatorRow>(processor, op));
        addAndMakeVisible(*operators.back());
    }

    // VOICE
    add("alg", "ALGORITHM", "Which operators modulate which. Carriers are drawn red, at the bottom.");
    add("fb", "FEEDBACK", "How much the marked operator modulates itself: 0 none, 7 towards a sawtooth or noise.");
    add("oks", "KEY SYNC", "ON: every operator starts its wave at the same point on each note. OFF: they run free.");
    add("transpose", "TRANSPOSE", "The key that plays middle C. C3 is no transposition.");
    for (int i = 0; i < 4; ++i) voiceGroup.addAndMakeVisible(*fields[(size_t) i]);

    // PITCH EG
    for (int i = 1; i <= 4; ++i) add("PR" + juce::String(i), "R" + juce::String(i), "Pitch envelope rate.");
    for (int i = 1; i <= 4; ++i) add("PL" + juce::String(i), "L" + juce::String(i), "Pitch envelope level: 50 is the note, 0 and 99 four octaves down and up.");
    for (int i = 4; i < 12; ++i) pitchGroup.addAndMakeVisible(*fields[(size_t) i]);

    // LFO
    add("lfoWave", "WAVE", "The LFO's shape. One LFO is shared by every note, as on the DX7.");
    add("lfoSpeed", "SPEED", "LFO speed: about 0.06 Hz at 0 to 50 Hz at 99.");
    add("lfoDelay", "DELAY", "How long the LFO waits, then fades in, after a key.");
    add("lfoPmd", "PITCH", "LFO pitch modulation depth - vibrato. Reaches as far as PMS lets it.");
    add("lfoAmd", "AMP", "LFO amplitude modulation depth - tremolo, on the operators with AM SENS.");
    add("pms", "PMS", "Pitch modulation sensitivity: how far LFO and the controllers can move the pitch.");
    add("lfoSync", "SYNC", "Restart the LFO at every key.");
    for (int i = 12; i < 19; ++i) lfoGroup.addAndMakeVisible(*fields[(size_t) i]);

    // FUNCTION
    add("mono", "MODE", "POLY plays up to 16 notes; MONO one, legato notes not retriggering.");
    add("voices", "VOICES", "How many notes at once in POLY.");
    add("pbRange", "PB RANGE", "Pitch bend range in semitones.");
    add("pbStep", "PB STEP", "Bend in steps of this many semitones; OFF is smooth.");
    add("portaMode", "PORTA", "Portamento: RETAIN or FOLLOW in POLY; in MONO, RETAIN glides only between held keys.");
    add("portaGliss", "GLISS", "Glissando: the glide moves in semitone steps.");
    add("portaTime", "P TIME", "Portamento time. 0 is off.");
    add("tune", "TUNE", "Master tune, in cents.");
    for (int i = 19; i < 27; ++i) functionGroup.addAndMakeVisible(*fields[(size_t) i]);

    // CONTROLLERS
    for (const char* c : { "wheel", "foot", "breath", "at" })
    {
        const juce::String id(c);
        const juce::String name = id == "wheel" ? "WHEEL" : id == "foot" ? "FOOT" : id == "breath" ? "BREATH" : "AFTERTCH";
        add(id + "Range", name, "How far this controller reaches, 0-99.");
        add(id + "Pitch", "PITCH", "This controller adds vibrato (with the LFO, by PMS).");
        add(id + "Amp", "AMP", "This controller adds tremolo (with the LFO, on operators with AM SENS).");
        add(id + "Eg", "EG BIAS", "This controller raises the level of operators with AM SENS - quiet until it moves.");
    }
    add("volume", "VOLUME", "Output volume.");
    for (int i = 27; i < (int) fields.size(); ++i) controllerGroup.addAndMakeVisible(*fields[(size_t) i]);

    keyboard.setAvailableRange(24, 108);
    keyboard.setLowestVisibleKey(36);
    keyboard.setKeyWidth(26.0f);

    refreshHeader();
    startTimerHz(20);
}

Panel::~Panel()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

Field& Panel::add(const juce::String& id, const juce::String& caption, const juce::String& hint)
{
    fields.push_back(std::make_unique<Field>(processor, id, caption, hint));
    return *fields.back();
}

void Panel::refreshHeader()
{
    cartridge.clear(juce::dontSendNotification);
    for (int b = 0; b < banks::count(); ++b) cartridge.addItem("BANK " + juce::String(b + 1) + "  " + juce::String(banks::name(b)), b + 1);
    if (processor.builtInBank() < 0)
    {
        cartridge.addItem(processor.cartridgeName(), 100);
        cartridge.setSelectedId(100, juce::dontSendNotification);
    }
    else
        cartridge.setSelectedId(processor.builtInBank() + 1, juce::dontSendNotification);

    programs.clear(juce::dontSendNotification);
    for (int i = 0; i < 32; ++i) programs.addItem(processor.getProgramName(i), i + 1);
    programs.setSelectedItemIndex(processor.getCurrentProgram(), juce::dontSendNotification);

    if (! nameEditor.hasKeyboardFocus(true))
        nameEditor.setText(processor.voiceName().trimEnd(), false);
}

void Panel::timerCallback()
{
    if (processor.changeCount() != seenChanges)
    {
        seenChanges = processor.changeCount();
        refreshHeader();
    }
    for (auto& o : operators) o->refresh();
    algorithm.repaint();
    pitchEnvelope.repaint();
}

void Panel::loadSyx()
{
    chooser = std::make_unique<juce::FileChooser>("Open a DX7 cartridge or voice", juce::File(), "*.syx;*.SYX;*.bin");
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                         [this] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file == juce::File()) return;
        juce::String message;
        const bool ok = processor.loadSyx(file, message);
        if (! ok || message.isNotEmpty())
            juce::NativeMessageBox::showMessageBoxAsync(ok ? juce::MessageBoxIconType::InfoIcon : juce::MessageBoxIconType::WarningIcon,
                                                        ok ? "Loaded" : "Could not load that file", message, this);
        refreshHeader();
    });
}

void Panel::saveMenu()
{
    juce::PopupMenu m;
    m.addItem(1, "Save the cartridge (32 voices)...");
    m.addItem(2, "Save this voice...");
    m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&saveButton), [this] (int choice)
    {
        if (choice == 0) return;
        const bool whole = choice == 1;
        const auto suggested = (whole ? processor.cartridgeName() : processor.voiceName().trimEnd())
                                   .replaceCharacters(" &/\\:", "_____") + ".syx";
        chooser = std::make_unique<juce::FileChooser>(whole ? "Save the cartridge" : "Save the voice",
                                                      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(suggested),
                                                      "*.syx");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                             [this, whole] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File()) return;
            if (! file.hasFileExtension("syx")) file = file.withFileExtension("syx");
            const bool ok = whole ? processor.saveCartridge(file) : processor.saveVoice(file);
            if (! ok)
                juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Could not save",
                                                            "Could not write " + file.getFullPathName(), this);
        });
    });
}

void Panel::storeMenu()
{
    juce::PopupMenu m;
    for (int i = 0; i < 32; ++i)
        m.addItem(i + 1, "Store into " + processor.getProgramName(i), true, i == processor.getCurrentProgram());
    m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&storeButton), [this] (int choice)
    {
        if (choice > 0) { processor.store(choice - 1); refreshHeader(); }
    });
}

void Panel::paint(juce::Graphics& g)
{
    g.fillAll(PanelLook::background);

    g.setColour(PanelLook::text);
    g.setFont(juce::FontOptions(34.0f, juce::Font::bold));
    g.drawText("Mz7", 16, 8, 90, 40, juce::Justification::left);
    g.setColour(PanelLook::dim);
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.drawText("DIGITAL FM SYNTHESIZER", 98, 14, 160, 14, juce::Justification::left);
    g.drawText("6 OPERATORS  32 ALGORITHMS", 98, 28, 160, 14, juce::Justification::left);

    g.setColour(PanelLook::value);
    g.fillRect(0, 54, getWidth(), 2);
}

void Panel::resized()
{
    // header
    cartridge.setBounds(270, 13, 220, 28);
    loadButton.setBounds(496, 13, 84, 28);
    saveButton.setBounds(584, 13, 84, 28);
    programs.setBounds(690, 13, 220, 28);
    nameEditor.setBounds(916, 13, 130, 28);
    storeButton.setBounds(1052, 13, 66, 28);
    initButton.setBounds(1122, 13, 52, 28);
    link.setBounds(1180, 13, 208, 28);

    const int fh = Field::captionHeight + Field::boxHeight;
    auto row = [&] (int from, int to, juce::Rectangle<int> area, int width = -1)
    {
        const int n = to - from;
        const int w = width > 0 ? width : (area.getWidth() - (n - 1) * 4) / n;
        for (int i = 0; i < n; ++i) fields[(size_t) (from + i)]->setBounds(area.getX() + i * (w + 4), area.getY(), w, fh);
    };

    // left column
    voiceGroup.setBounds(12, 64, 360, 280);
    algorithm.setBounds(voiceGroup.content().withHeight(200));
    row(0, 4, voiceGroup.content().withTrimmedTop(212).withHeight(fh));

    pitchGroup.setBounds(12, 352, 360, 150);
    pitchEnvelope.setBounds(pitchGroup.content().withHeight(70));
    row(4, 12, pitchGroup.content().withTrimmedTop(80).withHeight(fh));

    lfoGroup.setBounds(12, 510, 360, 70);
    row(12, 19, lfoGroup.content().withHeight(fh));

    functionGroup.setBounds(12, 588, 360, 70);
    row(19, 27, functionGroup.content().withHeight(fh));

    // operators, right
    const int top = 64, rowH = 94, gap = 5;
    for (int i = 0; i < 6; ++i)
        operators[(size_t) i]->setBounds(384, top + i * (rowH + gap), 1004, rowH);

    // controllers, under the operators and across the bottom of the left column
    controllerGroup.setBounds(12, 666, 1376, 70);
    {
        auto c = controllerGroup.content().withHeight(fh);
        int x = c.getX();
        for (int s = 0; s < 4; ++s)
        {
            for (int k = 0; k < 4; ++k)
            {
                fields[(size_t) (27 + s * 4 + k)]->setBounds(x, c.getY(), k == 0 ? 70 : 56, fh);
                x += (k == 0 ? 70 : 56) + 4;
            }
            x += 22;
        }
        fields.back()->setBounds(c.getRight() - 90, c.getY(), 90, fh);
    }

    keyboard.setBounds(12, 744, 1376, 144);
}

// -------------------------------------------------------------------------------- editor

Mz7Editor::Mz7Editor(Mz7Processor& p) : AudioProcessorEditor(p), panel(p)
{
    addAndMakeVisible(panel);
    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio((double) Panel::designWidth / Panel::designHeight);
    setResizeLimits(Panel::designWidth / 2, Panel::designHeight / 2, Panel::designWidth * 2, Panel::designHeight * 2);
    setSize(1120, 720);
}

void Mz7Editor::resized()
{
    const float scale = (float) getWidth() / (float) Panel::designWidth;
    panel.setBounds(0, 0, Panel::designWidth, Panel::designHeight);
    panel.setTransform(juce::AffineTransform::scale(scale));
}
