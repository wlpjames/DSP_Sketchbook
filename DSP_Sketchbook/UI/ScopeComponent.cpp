//
//  ScopeComponent.cpp
//  
//
//  Created by Billy James on 13/03/26.
//

namespace sketchbook
{

ScopeComponent::ScopeComponent (Queue& queueToUse)
: audioBufferQueue (queueToUse)
{
    sampleData.fill (0.f);
    setFramesPerSecond (30);
    showScope(freq);
    
    //init spectrumData to 1.f
    for (float& v : spectrumData)
        v = 1.f;
}

void ScopeComponent::setFramesPerSecond (int framesPerSecond)
{
    jassert (framesPerSecond > 0 && framesPerSecond < 1000);
    startTimerHz (framesPerSecond);
}

void ScopeComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    // Spectrum
    auto scopeRect = area.toFloat();
    g.setColour(Style::getInstance()->backgroundColour);
    g.fillRoundedRectangle(scopeRect, 5);
    
    scopeRect.reduce(10, 10);
    
    switch (currScope) {
            
        case osc:
        {
            g.setColour (Style::getInstance()->themeColour);
            plot (sampleData.data(), sampleData.size(), g, scopeRect, 1.f, scopeRect.getHeight() / 2);
            break;
        }
            
        case freq:
            g.setColour (Style::getInstance()->themeColour);
            plot (spectrumData.data(), spectrumData.size() / 4, g, scopeRect.reduced(10));
            break;
            
        default:
            break;
    }
}

void ScopeComponent::showScope(scopeToShow scope)
{
    currScope = scope;
}

void ScopeComponent::timerCallback()
{
    /// the code bellow prived by claude.ai
    audioBufferQueue.pop (sampleData.data());
    juce::FloatVectorOperations::copy (spectrumData.data(), sampleData.data(), (int) sampleData.size());

    auto fftSize = (size_t) fft.getSize();

    jassert (spectrumData.size() == 2 * fftSize);
    windowFun.multiplyWithWindowingTable (spectrumData.data(), fftSize);
    fft.performFrequencyOnlyForwardTransform (spectrumData.data(), true);

    static constexpr auto mindB = -160.f;
    static constexpr auto maxdB = 0.f;

    for (auto& s : spectrumData)
        s = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (s) - juce::Decibels::gainToDecibels (float(fftSize))), mindB, maxdB, 0.f, 1.f);

    // --- Remap linear bins into log-scaled output in-place ---
    static constexpr float minFreq = 30.f;
    static constexpr float maxFreq = 20000.f;
    auto numBins = fftSize / 2 + 1;
    auto numOutputBins = numBins; // same size, remapped
    float sampleRate = 44100;

    std::vector<float> logMapped (numOutputBins);

    for (size_t i = 0; i < numOutputBins; ++i)
    {
        float normX = (float) i / (float) (numOutputBins - 1);
        float freq  = minFreq * std::pow (maxFreq / minFreq, normX);
        float binF  = freq / sampleRate * (float) fftSize;
        
        int   bin0  = (int) binF;
        float frac  = binF - (float) bin0;
        
        bin0 = juce::jlimit (0, (int) numBins - 2, bin0);
        
        // Interpolate between neighbouring bins
        logMapped[i] = spectrumData[bin0] + frac * (spectrumData[bin0 + 1] - spectrumData[bin0]);
    }

    juce::FloatVectorOperations::copy(spectrumData.data(), logMapped.data(), (int)numOutputBins);

    repaint();
}

void ScopeComponent::plot(const float* data, size_t numSamples, juce::Graphics& g,
                                 juce::Rectangle<float> rect, float scaler, float offset)
{
    auto w = rect.getWidth();
    auto h = rect.getHeight();
    auto right = rect.getRight();
    
    auto center = rect.getBottom() - offset;
    auto gain = h * scaler;
    
    for (size_t i = 1; i < numSamples; ++i)
        g.drawLine ({ juce::jmap (float(i - 1), 0.f, float(numSamples - 1), float(right - w), float(right)),
                      center - gain * data[i - 1],
                      juce::jmap (float(i), 0.f, float(numSamples - 1), float(right - w), float(right)),
                      center - gain * data[i] });
}

} //end namespace sketchbook
