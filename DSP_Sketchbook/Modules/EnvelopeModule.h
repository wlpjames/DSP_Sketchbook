/*
  ==============================================================================

    Envelope.h
    Created: 11 Oct 2022 11:40:22am
    Author:  William James

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../Engine/Module.h"

namespace sketchbook
{
//MARK: Envelope
class EnvelopeModule : public sketchbook::Module
{
    public:
    
    enum envState {
        env_idle = 0,
        env_attack,
        env_decay,
        env_sustain,
        env_release,
        env_quick_release
    } state;
    
    EnvelopeModule();
    
    ~EnvelopeModule();
    
    void prepareToPlay(float samplerate, int buffersize) override;
    
    void noteOn(const sketchbook::NoteOnEvent& event) override;
    
    void noteOff(bool) override;
    
    void process(juce::AudioBuffer<float>& buffer) override;
    
    void processSample(float* sample) override;
    
    void reset() override;
    
    bool isActive();
    
    juce::String getName() override;
    
    private:
    
    void setAttackRate(double rate);
    
    void setDecayRate(double rate);
    
    void setReleaseRate(double rate);
    
    void setSustainLevel(double level);
    
    void setVelocityMod(double velMod);
    
    void setSustainPedal(bool val);
    
    envState getState();
    
    /**
     Will calculate the appropriate minimal attack and release for a given pitch
     
     @param pitch floating point pitch in HZ
     */
    void setMinimalAttackRelease(float pitch);
    
    private:
    
    void setTargetRatioA(double targetRatio);
    
    void setTargetRatioDR(double targetRatio);
    
    double calcCoef(double rate, double targetRatio);
    
    //MARK: members
    bool enabled;
    float sends = 1.0; //0 to 1.0 value
    bool sustainPedalOn=false;
    float  minimalRelease = 15;
    
    //rate in num samples
    float attackS;
    float decayS;
    float releaseS;
    double attackRate;
    double decayRate;
    double releaseRate;
    double attackCoef;
    double decayCoef;
    double releaseCoef;
    double quickReleaseCoef;
    double sustainLevel;
    double targetRatioA;
    double targetRatioDR;
    float currValue;
    float velocityMod=1.f;
    float noteVelocity= 1.f;
    
    //exponents aim to these value so as they pass through 0 and 1
    double attackBase;
    double decayBase;
    double releaseBase;
    double quickReleaseBase;
    float samplerate;
};
} //end namespace sketchbook
