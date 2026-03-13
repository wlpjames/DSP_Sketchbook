/*
  ==============================================================================

    ModulationSources.h
    Created: 16 May 2025 12:18:30pm
    Author:  William James

  ==============================================================================
*/

#pragma once
#include "../Engine/Module.h"

namespace sketchbook
{

class LfoModule : public sketchbook::Module
{
    public:
    
    LfoModule()
    : frequency(0.5f)
    , depth(1.0f)
    , phase(0.0f)
    , sampleRate(44100.0f)
    {
        setModuleParameters(
                            {
                                Parameter::Float("Rate Hz", [this](juce::var value)
                                                 {
                                    frequency = static_cast<float>(value);
                                    updatePhaseIncrement();
                                }, 0.5, 0.01f, 10.0),
                                
                                Parameter::Float("Depth", [this](juce::var value)
                                                 {
                                    depth = static_cast<float>(value);
                                }, 1.0, 0.0f, 1.0)
                            });
    }
    
    virtual ~LfoModule() {}
    
    void prepareToPlay(float samplerate, int buffersize) override {
        sampleRate = samplerate;
        updatePhaseIncrement();
    }
    
    void noteOn(const sketchbook::NoteOnEvent& event) override {
        phase = 0.0f;
    }
    
    void noteOff(bool) override
    {
    }
    
    void reset() override {
        phase = 0.0f;
    }
    
    void applyMidi(const juce::MidiMessage& message) override
    {
    }
    
    void processSample(float* sample) override
    {
        float lfoValue = depth * std::sin(phase);
        *sample *= (1.f + lfoValue) / 2.f;  // Amplitude modulation
        
        phase += phaseIncrement;
        if (phase >= 2.0f * M_PI)
            phase -= 2.0f * M_PI;
        
        internalBuffer.appendSingleSample(*sample);
    }
    
    juce::String getName() override
    {
        return "LFO";
    }
    
    private:
    
    void updatePhaseIncrement()
    {
        phaseIncrement = 2.0f * M_PI * (frequency / sampleRate);
    }
    
    float frequency;
    float depth;
    float phase;
    float phaseIncrement;
    float sampleRate;
};

} //end namespace sketchbook
