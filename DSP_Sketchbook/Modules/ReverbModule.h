/*
  ==============================================================================

    Reverb.h
    Created: 27 Jan 2026 12:18:30pm
    Author:  William James

  ==============================================================================
*/
#include "DragonFlyReverb/DistrhoPluginInfo.h"
#include "DragonFlyReverb/DSP.hpp"
#define LIBFV3_FLOAT

namespace sketchbook
{
class ReverbModule : public Module
{
public:
    ReverbModule()
    : dragonFlyReverb(samplerate)
    {
        setModuleParameters({
            
            Parameter::Choice("Preset", [this] (juce::String value)
            {
                setPresetByName(value);
            }, getPresetNames(), "Bright Room"),
            
            Parameter::Float("wet", [&] (juce::var value)
            {
                wet = float(value);
            }, 0.5f, 0.f, 2.5f),

        });
    }
    
    juce::String getName() override
    {
        return "Dragon Fly Hall Reverb";
    }

    //==============================================================================
    void prepareToPlay (float _samplerate, int _maxBufferSize) override
    {
        samplerate = _samplerate;
        dragonFlyReverb.sampleRateChanged(_samplerate);
        tmpBuffer.setSize(2, _maxBufferSize);
    }

    //==============================================================================
    void process (juce::AudioBuffer<float>& buffer) noexcept override
    {
        //run the reverb algo
        dragonFlyReverb.run(buffer.getArrayOfWritePointers(), tmpBuffer.getArrayOfWritePointers(), buffer.getNumSamples());
        
        //copy back to the buffer with wet dry mix
        for (int i = 0; i < buffer.getNumChannels(); i++)
            for (int j = 0; j < buffer.getNumSamples(); j++)
                buffer.getWritePointer(i)[j] += tmpBuffer.getWritePointer(i)[j] * wet;
    }

    //==============================================================================
    void reset() noexcept override
    {
        
    }
    
private:
    
    float samplerate = 44100;
    dragonfly::DragonflyReverbDSP dragonFlyReverb;
    juce::AudioBuffer<float> tmpBuffer;
    float wet = 1.f;
    
    // ===========================================================================
    // The following is an implementation of the Dragonfly Hall reverb parameters
    // ===========================================================================
    const dragonfly::Preset* getPresetByName(juce::String name)
    {
        for (int i = 0; i < dragonfly::NUM_BANKS; i++)
            for (int j = 0; j < dragonfly::PRESETS_PER_BANK; j++)
                if (juce::String(dragonfly::banks[i].presets[j].name) == name)
                    return &dragonfly::banks[i].presets[j];
        
        return nullptr;
    }
    
    void setPresetByName(juce::String name)
    {
        if (auto* preset = getPresetByName(name))
        {
            for (uint32_t i = 0; i < dragonfly::Parameters::paramCount; i++)
            {
                dragonFlyReverb.setParameterValue(i, preset->params[i]);
            }
        }
    }
    
    static const juce::StringArray getPresetNames()
    {
        juce::StringArray output;
        for (int i = 0; i < dragonfly::NUM_BANKS; i++)
            for (int j = 0; j < dragonfly::PRESETS_PER_BANK; j++)
                output.add(juce::String(dragonfly::banks[i].presets[j].name));
        
        return output;
    }
};
} //end namespace sketchbook
