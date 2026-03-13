/*
  ==============================================================================

    SimpleOsc.h
    Created: 9 Jul 2025 4:09:34pm
    Author:  William James

  ==============================================================================
*/

#pragma once
#include "../Engine/Module.h"

namespace sketchbook
{

class SimpleOscModule : public sketchbook::Module
{
public:
    //==============================================================================
    SimpleOscModule()
    {
        setVoiceMonitorType(adsr);
        setIsDefaultEnabled(false);
        
        setModuleParameters({
            
            Parameter::Float("Pulse", [&] (juce::var value)
            {
                m_pulseLen = 1.f - float(value);
            }, 0.0, 0.0, 0.9),
            
            Parameter::Float("Phase", [&] (juce::var value)
            {
                m_phaseDistort = float(value);
            }, 0.25, 0.25, 0.49),
            
            Parameter::Float("Gain", [&] (juce::var value)
            {
                m_gain = float(value);
            }, 1.0, 0.f, 1.0),
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
    
    void pitchUpdated(float pitchHz) override
    {
        m_freqHz = pitchHz;
        m_phaseInc = 1.f / (m_samplerate / m_freqHz);
    }
    
    void processSample(float* leftSample, float* rightSample) override
    {
        //apply the phase distortion - the wave aproaches a saw wave
        float phase = m_phase;
        if (phase < m_phaseDistort)
        {
            phase = 0.25f * (phase / m_phaseDistort);
        }
        else if (phase > 1.f - m_phaseDistort)
        {
            phase = 0.75f + (0.25f * ((m_phaseDistort - (1.f - phase)) / m_phaseDistort));
        }
        else
        {
            float space = 1.f - m_phaseDistort * 2.f;
            phase = 0.25f + (0.5f * ((phase - m_phaseDistort) / space));
        }
        
        //apply the pulse distort to the phase i.e
        //after the pulse len phase is always 0
        phase = phase > m_pulseLen ? 0.f : phase / m_pulseLen;
        
        //add output to the buffer samples
        float sample = std::sin(phase * juce::MathConstants<float>::twoPi) * m_gain;
        *leftSample  += sample;
        *rightSample += sample;
        
        //increment and wrap the phase
        m_phase += m_phaseInc;
        if (m_phase >= 1.f)
            m_phase -= 1.f;
    }
    
    juce::String getName() override
    {
        return "Simple Osc";
    }

private:
    
    float m_samplerate = 0;
    float m_freqHz = 0;
    float m_phase = 0;
    float m_phaseInc = 0;
    float m_phaseDistort = 0;
    float m_pulseLen = 0;
    float m_gain = 0;
};
} // end namespace sketchbook
