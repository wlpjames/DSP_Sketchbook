/*
  ==============================================================================

    FX.h
    Created: 27 Apr 2025 8:10:18pm
    Author:  William James

  ==============================================================================
*/

#pragma once
//==============================================================================

//==============================================================================
class Distortion : public sketchbook::Module
{
public:
    //==============================================================================
    Distortion()
    {
        auto& waveshaper = processorChain.template get<waveshaperIndex>();
        waveshaper.functionToUse = [] (float x)
        {
            return std::tanh (x);
        };

        auto& preGain = processorChain.template get<preGainIndex>();
        auto& postGain = processorChain.template get<postGainIndex>();
        auto& filter = processorChain.template get<filterIndex>();
        
        setModuleParameters(
        {
            Parameter::Float("In Gain", [&] (juce::var value)
            {
                preGain.setGainDecibels (float(value));
            }, 20.0, 0.0, 30.0),
            
            Parameter::Float("Out Gain", [&] (juce::var value)
            {
                postGain.setGainDecibels (float(value));
            }, 20.0, -30.0, 30.0),
            
            Parameter::Float("Tone", [&] (juce::var value)
            {
                normToneValue = float(value);
                filter.state = FilterCoefs::makeFirstOrderHighPass (samplerate, juce::NormalisableRange<float>(500, 20000, 1.0, 2000).convertFrom0to1(normToneValue));
            }, 0.0, 0.0, 1.0),
        });
    }

    //==============================================================================
    void prepareToPlay (float samplerate, int buffersize) override
    {
        samplerate = samplerate;
        auto& filter = processorChain.template get<filterIndex>();
        filter.state = FilterCoefs::makeFirstOrderHighPass (samplerate, juce::NormalisableRange<float>(500, 20000, 1.0, 2000).convertFrom0to1(normToneValue));

        juce::dsp::ProcessSpec spec = {samplerate, juce::uint32(buffersize), 2};
        processorChain.prepare(spec);
    }

    //==============================================================================
    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        processorChain.process(ctx);
    }

    //==============================================================================
    void reset() noexcept override
    {
        processorChain.reset();
    }
    
    juce::String getName() override
    {
        return "Distortion";
    }

private:
    //==============================================================================
    enum
    {
        filterIndex,
        preGainIndex,
        waveshaperIndex,
        postGainIndex
    };

    float samplerate=44100.f;
    float normToneValue=1.f;
    
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<float>;

    juce::dsp::ProcessorChain<juce::dsp::ProcessorDuplicator<Filter, FilterCoefs>,
                              juce::dsp::Gain<float>, juce::dsp::WaveShaper<float>, juce::dsp::Gain<float>> processorChain;
};
