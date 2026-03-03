/*
  ==============================================================================

    CustomModule.h
    Created: 35 Jan 2026 12:19:10pm
    Author:  William James

  ==============================================================================
*/
#include <JuceHeader.h>

class CustomModule : public sketchbook::Module
{
public:
    //==============================================================================
    CustomModule()
    {
        setVoiceMonitorType(adsr);
        
        setModuleParameters({
            
            Parameter::Float("Float Param", [&] (juce::var value)
            {
                juce::ignoreUnused(value);
            }, 0.0, 0.0, 1.0),
            
            Parameter::Boolean("Test Boolean", [this] (bool value)
            {
                DBG(juce::String("Test Boolean : ") + (value ? juce::String("True") : juce::String("False")));
            }, true),
            
            Parameter::Choice("Text Choice", [this] (juce::String value)
            {
                DBG(juce::String("Test Choice : " + value));
            }, {"Item 1", "Item 2", "Item 3"}, "Item 1"),
            
            Parameter::Integer("Integer Test", [this] (int value)
            {
                DBG(juce::String("Integer Value : ") + juce::String(value));
            }, 1, 0, 10)
        });
    }
    
    void prepareToPlay(float samplerate, int buffersize) override
    {
        m_samplerate = samplerate;
    }
    
    void noteOn(const sketchbook::NoteOnEvent& event) override
    {
        if (!event.isLegatoNoteOn)
        {
            m_phase = 0;
            m_freqHz = juce::MidiMessage::getMidiNoteInHertz(event.midiMessage.getNoteNumber());
            m_phaseInc = 1.f / (m_samplerate / m_freqHz);
        }
    }
    
    void noteOff(bool) override
    {
        return;
    }
    
    void reset() override
    {
        return;
    }
    
    void pitchUpdated(float pitchHz) override
    {
        m_freqHz = pitchHz;
        m_phaseInc = 1.f / (m_samplerate / m_freqHz);
    }
    
    void processSample(float* sample) override
    {
        const float alteredPhase = m_phase > m_pulseLen ? 0.f : m_phase * 1 / m_pulseLen;
        *sample += std::sin(alteredPhase * juce::MathConstants<float>::twoPi) * m_gain;
        *sample += std::sin((alteredPhase + m_phaseOffset) * juce::MathConstants<float>::twoPi) * m_gain;
        
        //increment and wrap
        m_phase += m_phaseInc;
        if (m_phase >= 1.f)
            m_phase -= 1.f;
    }
    
    juce::String getName() override
    {
        return "New Custom Module";
    }

private:
    
    float m_samplerate = 0;
    float m_freqHz = 0;
    float m_phase = 0;
    float m_phaseInc = 0;
    float m_phaseOffset=0;
    float m_pulseLen = 0;
    float m_gain = 0;
};

using voiceModules = sketchbook::ModuleList<CustomModule, sketchbook::SimpleOsc>;
using postProscessEffects = sketchbook::ModuleList<sketchbook::Reverb, sketchbook::Delay>;
using mosulationSources = sketchbook::ModuleList<>;

SKETCHBOOK_DECLARE_APP("Sketchbook Template",
                       /* list of modules in voice    */voiceModules,
                       /* list of post process effects*/postProscessEffects,
                       /* list of modulation sources  */mosulationSources)
