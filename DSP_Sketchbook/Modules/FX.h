/*
  ==============================================================================

    FX.h
    Created: 27 Apr 2025 8:10:18pm
    Author:  William James

  ==============================================================================
*/

#pragma once
//==============================================================================

/*
//==============================================================================
template <typename Type, size_t maxNumChannels = 2>
class Delay : public sketchbook::Module
{
public:
    //==============================================================================
    Delay()
    {
        setMaxDelayTime (2.0f);
        setDelayTime (0, 0.7f);
        setDelayTime (1, 0.5f);
        setWetLevel (0.8f);
        setFeedback (0.5f);
        
        setModuleParameters({
            
            Parameter::Float("Delay Time 1", [&] (var value)
            {
                setDelayTime(0, float(value));
            }, 0.2, 0.0, 2.0),
            
            Parameter::Float("Delay Time 2", [&] (var value)
            {
                setDelayTime(1, float(value));
            }, 0.5, 0.0, 2.0),
            
            Parameter::Float("Feedback", [&] (var value)
            {
                setFeedback(float(value));
            }, 0.5, 0.0, 1.0),
            
            Parameter::Float("Wet Level", [&] (var value)
            {
                setWetLevel(float(value));
            }, 0.5, 0.0, 1.0),
        });
    }
    
    String getName() override
    {
        return "Delay";
    }

    //==============================================================================
    void prepareToPlay(float samplerate, int buffersize) noexcept override
    {
        sampleRate = samplerate;
        updateDelayLineSize();
        updateDelayTime();

        filterCoefs = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass (sampleRate, Type (1e3));

        
        juce::dsp::ProcessSpec procSpec = {double(samplerate), uint32(buffersize), uint32(2)};
        for (auto& f : filters)
        {
            f.prepare (procSpec);
            f.coefficients = filterCoefs;
        }
    }

    //==============================================================================
    void reset() noexcept override
    {
        for (auto& f : filters)
            f.reset();

        for (auto& dline : delayLines)
            dline.clear();
    }

    //==============================================================================
    size_t getNumChannels() const noexcept
    {
        return delayLines.size();
    }

    //==============================================================================
    void setMaxDelayTime (Type newValue)
    {
        jassert (newValue > Type (0));
        maxDelayTime = newValue;
        updateDelayLineSize(); // [1]
    }

    //==============================================================================
    void setFeedback (Type newValue) noexcept
    {
        jassert (newValue >= Type (0) && newValue <= Type (1));
        feedback = newValue;
    }

    //==============================================================================
    void setWetLevel (Type newValue) noexcept
    {
        jassert (newValue >= Type (0) && newValue <= Type (1));
        wetLevel = newValue;
    }

    //==============================================================================
    void setDelayTime (size_t channel, Type newValue)
    {
        if (channel >= getNumChannels())
        {
            jassertfalse;
            return;
        }
        jassert (newValue >= Type (0));
        delayTimes[channel] = newValue;
        updateDelayTime(); // [3]
    }

    //==============================================================================
    void process(AudioBuffer<float>& buffer) override
    {
        auto numSamples = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& dline = delayLines[ch];
            auto delayTime = delayTimesSample[ch];
            auto& filter = filters[ch];
            
            for (int i = 0; i < numSamples; ++i)
            {
                auto delayedSample = filter.processSample (dline.get (delayTime)); // [1]
                auto inputSample = buffer.getWritePointer(ch)[i];
                auto dlineInputSample = std::tanh (inputSample + feedback * delayedSample);
                dline.push (dlineInputSample);
                auto outputSample = inputSample + wetLevel * delayedSample;
                buffer.getWritePointer(ch)[i] = outputSample;
            }
        }
    }

private:
    //==============================================================================
    std::array<DelayLine<Type>, maxNumChannels> delayLines;
    std::array<size_t, maxNumChannels> delayTimesSample;
    std::array<Type, maxNumChannels> delayTimes;
    Type feedback { Type (0) };
    Type wetLevel { Type (0) };

    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> filters;
    typename juce::dsp::IIR::Coefficients<Type>::Ptr filterCoefs;

    Type sampleRate   { Type (44.1e3) };
    Type maxDelayTime { Type (2) };

    //==============================================================================
    void updateDelayLineSize()
    {
        auto delayLineSizeSamples = (size_t) std::ceil (maxDelayTime * sampleRate);
        for (auto& dline : delayLines)
            dline.resize (delayLineSizeSamples); // [2]
    }

    //==============================================================================
    void updateDelayTime() noexcept
    {
        for (size_t ch = 0; ch < maxNumChannels; ++ch)
            delayTimesSample[ch] = (size_t) juce::roundToInt (delayTimes[ch] * sampleRate);
    }
};
*/
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
        
        setModuleParameters({
            
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
