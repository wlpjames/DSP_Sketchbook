/*
  ==============================================================================

    Delay.h
    Created: 27 Jan 2026 12:18:30pm
    Author:  William James

  ==============================================================================
*/
namespace sketchbook
{

class DelayModule : public Module
{
public:
    
    DelayModule()
    {
        setIsDefaultEnabled(false);
        
        //init arrays
        delayBufferL = new float[nativeSR];
        delayBufferR = new float[nativeSR];
        for(int i = 0; i < nativeSR; i++)
        {
            delayBufferL[i] = 0.f;
            delayBufferR[i] = 0.f;
        }
        
        setModuleParameters({
            
            Parameter::Float("Rate Hz", [&] (juce::var value)
            {
                setDelayTime(1 / float(value));
                
            }, 1.5f, 0.5f, 10.f),
            
            Parameter::Float("Decay", [&] (juce::var value)
            {
                setDecay(float(value));
            }, 0.3f, 0.f, 1.f),
            
            Parameter::Float("Tone", [&] (juce::var value)
            {
                //TODO: this parameter
                
            }, 1.f, 0.f, 1.f),
            
            Parameter::Float("Wet", [&] (juce::var value)
            {
                wetGain = float(value);
                
            }, 1.f, 0.f, 1.f),
            
            Parameter::Float("Dry", [&] (juce::var value)
            {
                dryGain = float(value);
                
            }, 1.f, 0.f, 1.f),
        });
    }
    
    ~DelayModule()
    {
        delete[] delayBufferL;
        delete[] delayBufferR;
    }
    
    juce::String getName() override
    {
        return "Delay";
    }

    //==============================================================================
    void prepareToPlay (float _samplerate, int _maxBufferSize) override
    {
        samplerate = _samplerate;
        wetBuffer.setSize(2, _maxBufferSize);
    }

    //==============================================================================
    void process (juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = 2;
        jassert(buffer.getNumChannels() == 2);
        
        for (int i = 0; i < 2; i++)
            wetBuffer.copyFrom(i, 0, buffer, i, 0, buffer.getNumSamples());
        
        float* signalL = wetBuffer.getWritePointer(0);
        float* signalR = wetBuffer.getWritePointer(1);
        
        //calc resampling rate
        float r =  (float(nativeSR) / delayTimeSec) / samplerate;
        
        // expected output size // risk of rounding errors?
        int numSamplesToWrite = int(numSamples * r);
        const float speedRatio = float(numSamples) / float(numSamplesToWrite);
        
        //single buffer
        juce::AudioBuffer<float> rs(2, numSamplesToWrite);
        float* inputResizedL = rs.getWritePointer(0);
        float* inputResizedR = rs.getWritePointer(1);
        
        //interpolate samples to there new length in the delay line
        readInterpL.process(speedRatio, signalL, inputResizedL, numSamplesToWrite, numSamples, 0);
        readInterpR.process(speedRatio, signalR, inputResizedR, numSamplesToWrite, numSamples, 0);
        
        //write resampled to tape, read from tape, and proccess
        //read samples are written back the the "samples to write provided"
        processToDelayLine(inputResizedL, inputResizedR, numSamplesToWrite);
        
        //resample to this sample rate
        writeInterpL.process(((nativeSR / delayTimeSec) / samplerate), inputResizedL, signalL, numSamples, numSamplesToWrite, 0);
        writeInterpR.process(((nativeSR / delayTimeSec) / samplerate), signalR, inputResizedR, numSamplesToWrite, numSamples, 0);
        
        //write back to the buffer with a mix of wet and dry
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < numSamples; j++)
            {
                buffer.getWritePointer(i)[j] = buffer.getWritePointer(i)[j]    * dryGain
                                             + wetBuffer.getWritePointer(i)[j] * wetGain;
            }
        }
    }

    //==============================================================================
    void reset() noexcept override
    {
        
        for (int i = 0; i < nativeSR; i++)
        {
            delayBufferL[i] = 0.f;
            delayBufferR[i] = 0.f;
        }
        
        readInterpL.reset();
        writeInterpL.reset();
    }

private:
    
    inline void processToDelayLine(float* signalL, float* signalR, int sigLen)
    {
        for (int i = 0; i < sigLen; i++)
        {
            float inputL = signalL[i];
            float inputR = signalR[i];
            
            //read the tape and mix with signal
            readSignal(signalL + i, signalR + i);
            
            //enter into array from write point
            writeSignal(inputL, inputR);
        }
    }
    
    inline void writeSignal(float signalL, float signalR)
    {
        delayBufferL[write_ind] = (delayBufferL[write_ind] * decay) + signalL;
        delayBufferR[write_ind] = (delayBufferR[write_ind] * decay) + signalR;
        
        write_ind++;
        if (write_ind >= nativeSR)
            write_ind = 0;
    }
    
    inline void readSignal(float* l, float* r)
    {
        *l = delayBufferL[read_ind];
        *r = delayBufferR[read_ind];
        
        read_ind++;
        if (read_ind >= nativeSR)
            read_ind = 0;
    }
    
    void setDecay(float val)
    {
        decay = val;
    }
    
    void setDelayTime(float sec)
    {
        jassert(sec > 0.f);
        delayTimeSec = sec;
    }
    
private:
    
    int write_ind = 0;
    int read_ind = 0;
    
    float decay=0.3f;
    float delayTimeSec = 0.1f;
    
    float wetGain = 1.f;
    float dryGain = 1.f;
    
    //a sensible default value for the samplerate
    float samplerate=44100;
    
    juce::AudioBuffer<float> wetBuffer;
    
    //native sample rate is a constant that
    //is used to interpolate input and output signal to
    //the delay buffer - to emulate the change in speed
    //of a traditional tape rotation
    const int nativeSR = 88200;
    
    float* delayBufferL=nullptr;
    float* delayBufferR=nullptr;
    
    juce::LagrangeInterpolator readInterpL;
    juce::LagrangeInterpolator readInterpR;
    
    juce::LagrangeInterpolator writeInterpL;
    juce::LagrangeInterpolator writeInterpR;
};
}
