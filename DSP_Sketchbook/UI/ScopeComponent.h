//
//  ScopeComponent.h
//  
//
//  Created by Billy James on 13/03/26.
//

#pragma once

namespace sketchbook
{

class ScopeComponent  : public juce::Component, private juce::Timer
{
    public:
    using Queue = AudioBufferQueue;
    
    enum scopeToShow
    {
        osc, freq
    };
    
    ScopeComponent (Queue& queueToUse);
    
    void setFramesPerSecond (int framesPerSecond);
    
    void paint (juce::Graphics& g) override;
    
    void showScope(scopeToShow scope);
    
private:
    
    void timerCallback() override;
    
    static void plot (const float* data,
                      size_t numSamples,
                      juce::Graphics& g,
                      juce::Rectangle<float> rect,
                      float scaler = 1.f,
                      float offset = 0.f);
    
    private:
    scopeToShow currScope = osc;
    
    Queue& audioBufferQueue;
    std::array<float, Queue::bufferSize> sampleData;
    
    juce::dsp::FFT fft { Queue::order };
    using WindowFun = juce::dsp::WindowingFunction<float>;
    WindowFun windowFun { (size_t) fft.getSize(), WindowFun::hann };
    std::array<float, 2 * Queue::bufferSize> spectrumData;
};
} //end namespace sketchbook
