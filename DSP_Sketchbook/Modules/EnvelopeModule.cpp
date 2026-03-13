/*
  ==============================================================================

    Envelope.cpp
    Created: 11 Oct 2022 11:46:26am
    Author:  William James

  ==============================================================================
*/

#include "EnvelopeModule.h"

namespace sketchbook
{

EnvelopeModule::EnvelopeModule()
{
    reset();
    
    //set some default values
    setAttackRate(0.1);
    setDecayRate(0.1);
    setReleaseRate(0.2);
    setSustainLevel(1.0);
    setTargetRatioA(0.3);
    setTargetRatioDR(0.0001);
    sends = 1.0f;
    
    //addParameters
    setModuleParameters({
        
        Parameter::Float("Attack", [this](juce::var value)
                         {
            setAttackRate(value);
        }, 0.01f, 0.001f, 10.0f),
        
        Parameter::Float("Decay", [this](juce::var value)
                         {
            setDecayRate(value);
        }, 0.1f, 0.001f, 10.0f),
        
        Parameter::Float("Sustain", [this](juce::var value)
                         {
            setSustainLevel(value);
        }, 0.8f, 0.0f, 1.0f),
        
        Parameter::Float("Release", [this](juce::var value)
                         {
            setReleaseRate(value);
        }, 0.5f, 0.001f, 10.0f),
        
        Parameter::Float("Velocity", [this](juce::var value)
                         {
            setVelocityMod(value);
        }, 1.0f, 0.0f, 1.0f)
    });
}

EnvelopeModule::~EnvelopeModule() {}

void EnvelopeModule::prepareToPlay( float _samplerate, int buffersize)
{
    samplerate = _samplerate;
    
    //re - calc values
    setAttackRate( attackS);
    setDecayRate( decayS);
    setReleaseRate( releaseS);
}

void EnvelopeModule::reset()
{
    Module::reset();
    state = env_idle;
    currValue = 0.0;
}

juce::String EnvelopeModule::getName()
{
    return "ADSR";
}

/**
 Processes a buffer through the envelope apllying gain
 */
void EnvelopeModule::process(juce::AudioBuffer<float>& inputBuffer)
{
    for (int i = 0; i < inputBuffer.getNumSamples(); i++) {
        float tempL = 1.f;
        float tempR = 1.f;
        processSample(&tempL, &tempR);
        internalBuffer.data.getWritePointer(0)[i] = tempL;
    }
    
    for (int i = 0; i < inputBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < inputBuffer.getNumSamples(); j++)
        {
            inputBuffer.getWritePointer(i)[j] *= internalBuffer.data.getWritePointer(0)[j];
        }
    }
}

void EnvelopeModule::noteOn(const sketchbook::NoteOnEvent& event)
{
    //set minimal attack and release values reletive to the length of a maximal waveform
    //setMinimalAttackRelease( Scale::naturalFreqFromMidiNumber(event.message.getNoteNumber()));
    if ((!event.isLegatoNoteOn) || state == env_release )
    {
        reset();
        state = env_attack;
        noteVelocity = event.midiMessage.getFloatVelocity();
    }
}

void EnvelopeModule::noteOff(bool isHardNoteOff)
{
    if (state == env_idle)
        return;
    
    state = env_release;
}

bool EnvelopeModule::isActive()
{
    return state != env_idle;
}

void EnvelopeModule::setSustainPedal(bool val)
{
    sustainPedalOn = val;
}

EnvelopeModule::envState EnvelopeModule::getState()
{
    return state;
}

void EnvelopeModule::setAttackRate(double rate)
{
    attackS = rate;
    attackRate = rate * samplerate;
    attackCoef = calcCoef(attackRate, targetRatioA);
    attackBase = (1.0 + targetRatioA) * (1.0 - attackCoef);
}

void EnvelopeModule::setDecayRate(double rate)
{
    decayS = rate;
    decayRate = rate * samplerate;
    decayCoef = calcCoef(decayRate, targetRatioDR);
    decayBase = (sustainLevel - targetRatioDR) * (1.0 - decayCoef);
}

void EnvelopeModule::setReleaseRate(double rate)
{
    releaseS = rate;
    releaseRate = rate * samplerate;
    releaseCoef = calcCoef(releaseRate, targetRatioDR);
    releaseBase = -targetRatioDR * (1.0 - releaseCoef);
}

void EnvelopeModule::setVelocityMod(double velMod)
{
    velocityMod = velMod;
}

double EnvelopeModule::calcCoef(double rate, double targetRatio)
{
    return (rate <= 0) ? 0.0 : exp(-log((1.0 + targetRatio) / targetRatio) / rate);
}

void EnvelopeModule::setSustainLevel(double level)
{
    sustainLevel = level;
    decayBase = (sustainLevel - targetRatioDR) * (1.0 - decayCoef);
}

void EnvelopeModule::setTargetRatioA(double targetRatio)
{
    if (targetRatio < 0.000000001)
        targetRatio = 0.000000001;  // -180 dB
    
    targetRatioA = targetRatio;
    attackCoef = calcCoef(attackRate, targetRatioA);
    attackBase = (1.0 + targetRatioA) * (1.0 - attackCoef);
}

void EnvelopeModule::setTargetRatioDR(double targetRatio)
{
    if (targetRatio < 0.000000001)
        targetRatio = 0.000000001;  // -180 dB
    
    targetRatioDR = targetRatio;
    decayCoef = calcCoef(decayRate, targetRatioDR);
    releaseCoef = calcCoef(releaseRate, targetRatioDR);
    decayBase = (sustainLevel - targetRatioDR) * (1.0 - decayCoef);
    releaseBase = -targetRatioDR * (1.0 - releaseCoef);
}

void EnvelopeModule::processSample(float* sampleL, float* sampleR)
{
    switch (state) {
            
        case env_idle:
            break;
            
        case env_attack:
            currValue = attackBase + currValue * attackCoef;
            if (currValue >= 1.0) {
                currValue = 1.0;
                state = env_decay;
            }
            break;
            
        case env_decay:
            currValue = decayBase + currValue * decayCoef;
            if (currValue <= sustainLevel) {
                currValue = sustainLevel;
                state = env_sustain;
            }
            break;
            
        case env_sustain:
            break;
            
        case env_release:
            
            if (sustainPedalOn) break;
            
            currValue = releaseBase + currValue * releaseCoef;
            if (currValue <= 0.0) {
                currValue = 0.0;
                state = env_idle;
            }
            break;
            
        case env_quick_release:
            
            currValue = releaseBase + currValue * quickReleaseCoef;
            if (currValue <= 0.0) {
                currValue = 0.0;
                state = env_idle;
            }
            break;
            
        default:
            break;
    }
    
    float modulationValue = currValue * sends * (noteVelocity * velocityMod + (1-velocityMod));
    *sampleL *= modulationValue;
    *sampleR *= modulationValue;
    internalBuffer.appendSingleSample(modulationValue);
}

void EnvelopeModule::setMinimalAttackRelease(float pitch)
{
    minimalRelease = 1000 / pitch*2;
    
    //minimal release
    quickReleaseCoef = calcCoef(minimalRelease, targetRatioDR);
    quickReleaseBase = releaseBase = -targetRatioDR * (1.0 - releaseCoef);
}
} //end namespace sketchbook
