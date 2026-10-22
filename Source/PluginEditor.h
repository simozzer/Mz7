// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"

#include <memory>
#include <vector>

//
// The panel: everything a DX7 hides behind its membrane buttons and two-line display, laid
// out at once. Six operator rows on the right; the voice's own settings - algorithm, pitch
// envelope, LFO - and the FUNCTION settings on the left; the cartridge along the top.
//

class PanelLook final : public juce::LookAndFeel_V4
{
public:
    PanelLook();

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;

    juce::Font getTextButtonFont(juce::TextButton&, int) override { return juce::FontOptions(12.0f, juce::Font::bold); }
    juce::Font getComboBoxFont(juce::ComboBox&) override        { return juce::FontOptions(13.0f, juce::Font::bold); }
    juce::Font getPopupMenuFont() override                      { return juce::FontOptions(13.0f); }

    // the palette
    static const juce::Colour background, panel, edge, text, dim, value, carrier, box;
};

// One number: a caption over a box you drag up and down, showing the value as the DX7 would.
class Field final : public juce::Component
{
public:
    Field(Mz7Processor&, const juce::String& paramId, const juce::String& caption, const juce::String& hint = {});

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int captionHeight = 13, boxHeight = 22;

private:
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::String caption;
};

// An envelope, drawn from its four rates and levels.
class EnvelopeView final : public juce::Component
{
public:
    EnvelopeView(Mz7Processor&, std::array<juce::String, 8> ids, bool pitch);
    void paint(juce::Graphics&) override;

private:
    Mz7Processor& processor;
    std::array<std::atomic<float>*, 8> values {};
    bool pitch;
};

// The algorithm, drawn: carriers along the bottom, modulators stacked above them.
class AlgorithmView final : public juce::Component
{
public:
    explicit AlgorithmView(Mz7Processor&);
    void paint(juce::Graphics&) override;

private:
    Mz7Processor& processor;
};

class OperatorRow final : public juce::Component
{
public:
    OperatorRow(Mz7Processor&, int op);

    void paint(juce::Graphics&) override;
    void resized() override;
    void refresh();         // the frequency readout and the carrier tag

private:
    Mz7Processor& processor;
    int op;
    juce::ToggleButton onSwitch;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onAttachment;
    std::vector<std::unique_ptr<Field>> fields;
    EnvelopeView envelope;
    juce::String frequency;
    bool carrier = false;

    Field& field(const char* suffix, const char* caption, const char* hint = "");
};

// A titled group of fields.
class Group final : public juce::Component
{
public:
    explicit Group(const juce::String& title) : title(title) {}
    void paint(juce::Graphics&) override;
    juce::Rectangle<int> content() const { return getLocalBounds().reduced(8).withTrimmedTop(16); }

private:
    juce::String title;
};

class Panel final : public juce::Component, private juce::Timer
{
public:
    explicit Panel(Mz7Processor&);
    ~Panel() override;

    static constexpr int designWidth = 1400, designHeight = 900;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshHeader();
    Field& add(const juce::String& id, const juce::String& caption, const juce::String& hint = {});

    void chooseCartridge();
    void loadSyx();
    void saveMenu();
    void storeMenu();

    Mz7Processor& processor;
    PanelLook look;
    juce::TooltipWindow tooltips { this, 600 };

    // header
    juce::ComboBox cartridge, programs;
    juce::TextButton loadButton { "LOAD .SYX" }, saveButton { "SAVE .SYX" }, storeButton { "STORE" }, initButton { "INIT" };
    juce::TextEditor nameEditor;
    juce::HyperlinkButton link;
    std::unique_ptr<juce::FileChooser> chooser;
    int seenChanges = -1;

    // the voice's own settings
    Group voiceGroup { "VOICE" }, pitchGroup { "PITCH EG" }, lfoGroup { "LFO" }, functionGroup { "FUNCTION" },
          controllerGroup { "CONTROLLERS" };
    AlgorithmView algorithm;
    EnvelopeView pitchEnvelope;
    std::vector<std::unique_ptr<Field>> fields;
    std::vector<std::unique_ptr<OperatorRow>> operators;

    juce::MidiKeyboardComponent keyboard;
};

class Mz7Editor final : public juce::AudioProcessorEditor
{
public:
    explicit Mz7Editor(Mz7Processor&);
    void resized() override;

private:
    Panel panel;
};
