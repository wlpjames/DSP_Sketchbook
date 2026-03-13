/*
  ==============================================================================

    CustomModule.h
    Created: 35 Jan 2026 12:19:10pm
    Author:  William James

  ==============================================================================
*/
#include <JuceHeader.h>

/**
    An example module that can be added as a genorator or effect on each voice,
    as a post effect or and a modulation source (see the included modules for further usage examples)
 */
class CustomModule : public sketchbook::Module
{
public:
    //==============================================================================
    CustomModule()
    {
        ///Voice monitoring can be either adsr or silence detection
        ///controls for the adsr envelope will be added to the UI
        ///automaticaly if required by any of the modules
        setVoiceMonitorType(adsr);
        
        ///provide an array of parameters
        setModuleParameters({
            Parameter::Float("Float Param", [&] (float value)
            {
                juce::ignoreUnused(value);
            }, 0.0, 0.0, 1.0),
            
            Parameter::Boolean("Boolean Param", [this] (bool value)
            {
                DBG(juce::String("Test Boolean : ") + (value ? juce::String("True") : juce::String("False")));
            }, true),
            
            Parameter::Choice("Choice Param", [this] (juce::String value)
            {
                DBG(juce::String("Test Choice : " + value));
            }, {"Item 1", "Item 2", "Item 3"}, "Item 1"),
            
            Parameter::Integer("Integer Param", [this] (int value)
            {
                DBG(juce::String("Integer Value : ") + juce::String(value));
            }, 1, 0, 10),
            
            Parameter::File("File Parameter", [this] (juce::String fileName)
            {
                DBG("File Param Value : " + fileName);
            }, "None Selected"),
        });
    }
    
    /// provides upaded samplerate and maximum buffer sizes
    void prepareToPlay(float samplerate, int buffersize) override
    {
        m_samplerate = samplerate;
    }
    
    /// Called on a note on event, legato message will also be sent here though these may be ignored
    /// and pitch updates can be taken from overriding pitchUpdated(float)
    void noteOn(const sketchbook::NoteOnEvent& event) override
    {
        if (!event.isLegatoNoteOn)
        {
            m_phase = 0;
            m_freqHz = juce::MidiMessage::getMidiNoteInHertz(event.midiMessage.getNoteNumber());
            m_phaseInc = 1.f / (m_samplerate / m_freqHz);
        }
    }
    
    /// Called when a note off event recieved and release stage stated
    void noteOff(bool) override
    {
        return;
    }
    
    /// Called after processing on a voice finished,
    void reset() override
    {
        m_phase = 0;
        return;
    }
    
    /// This function must be overloaded in order to respond to poratamento, though a simple processor can
    /// just calculate the desired frequency from the NoteOnEvent (see above)
    void pitchUpdated(float pitchHz) override
    {
        m_freqHz = pitchHz;
        m_phaseInc = 1.f / (m_samplerate / m_freqHz);
    }
    
    
    void processSample(float* sample) override
    {
        *sample += std::sin(m_phase * juce::MathConstants<float>::twoPi);
        
        //increment and wrap
        m_phase += m_phaseInc;
        if (m_phase >= 1.f)
            m_phase -= 1.f;
    }
    
    /// This name will be displayed in the UI for your module
    juce::String getName() override
    {
        return "Example Module";
    }

private:
    
    float m_samplerate = 0;
    float m_freqHz = 0;
    float m_phase = 0;
    float m_phaseInc = 0;
};

/// The processing chain can be defined here, the engine will instantiate multiple instances of each voice module and modulation source, one for each voice
using VoiceModules = sketchbook::ModuleList<CustomModule, sketchbook::SamplerModule, sketchbook::SimpleOsc>;

using PostProscessEffects = sketchbook::ModuleList<sketchbook::ReverbModule, sketchbook::Delay>;

using ModulationSources = sketchbook::ModuleList<sketchbook::EnvelopeModule,
                                                 sketchbook::EnvelopeModule,
                                                 sketchbook::LfoModule,
                                                 sketchbook::LfoModule>;
/// Decare the app here!
/// The first argument is the title that will be displayed at the top of the UI
SKETCHBOOK_DECLARE_APP("Sketchbook Template",
                       VoiceModules,         /* list of modules in voice    */
                       PostProscessEffects,  /* list of post process effects*/
                       ModulationSources)    /* list of modulation sources  */
