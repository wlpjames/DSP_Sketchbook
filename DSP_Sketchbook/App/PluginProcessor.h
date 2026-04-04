/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../UI/LookAndFeel.h"
#include "PluginEditor.h"

#if JUCE_WINDOWS
#include "dwmapi.h"
#endif

//==============================================================================
/**
*/
template <typename VoiceModules,
          typename FxModules,
          typename ModulationSources,
          typename Presets>
class DSPSketchbookAudioProcessor  : public juce::AudioProcessor
{
public:

    //==============================================================================
    DSPSketchbookAudioProcessor(juce::String projectName = "")
    #ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
    #endif
    , m_projectName(projectName)
    {
        //TODO: this should change - not be a pointer
        context.audioBufferQueue = &audioBufferQueue;
        context.parameterData = audioEngine.getPluginData();
        context.projectName = m_projectName;
        
        context.getLatestPlayingModuleByName = [this] (juce::String name)
        {
            return audioEngine.getLatestPlayingModuleByName(name);
        };
        
        context.midiKeyboardState.addListener(&context.midiMessageCollector);
    }

    ~DSPSketchbookAudioProcessor()
    {
        sketchbook::Style::deleteInstance();
    }

    //==============================================================================
    const juce::String getName() const
    {
        return JucePlugin_Name;
    }

    bool acceptsMidi() const
    {
       #if JucePlugin_WantsMidiInput
        return true;
       #else
        return false;
       #endif
    }

    bool producesMidi() const
    {
       #if JucePlugin_ProducesMidiOutput
        return true;
       #else
        return false;
       #endif
    }

    bool isMidiEffect() const
    {
       #if JucePlugin_IsMidiEffect
        return true;
       #else
        return false;
       #endif
    }

    double getTailLengthSeconds() const
    {
        return 0.0;
    }

    int getNumPrograms()
    {
        return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                    // so this should be at least 1, even if you're not really implementing programs.
    }

    int getCurrentProgram()
    {
        return 0;
    }

    void setCurrentProgram (int index)
    {
        juce::ignoreUnused(index);
    }

    const juce::String getProgramName (int index)
    {
        juce::ignoreUnused(index);
        return {};
    }

    void changeProgramName (int index, const juce::String& newName)
    {
        juce::ignoreUnused(index, newName);
    }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        audioEngine.prepare(float(sampleRate), samplesPerBlock);
        context.midiMessageCollector.reset (float(sampleRate));
    }

    void releaseResources()
    {
        // When playback stops, you can use this as an opportunity to free up any
        // spare memory, etc.
    }

    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const
    {
      #if JucePlugin_IsMidiEffect
        juce::ignoreUnused (layouts);
        return true;
      #else
        // This is the place where you check if the layout is supported.
        // In this template code we only support mono or stereo.
        // Some plugin hosts, such as certain GarageBand versions, will only
        // load plugins that support stereo bus layouts.
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
         && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        // This checks if the input layout matches the output layout
       #if ! JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
       #endif

        return true;
      #endif
    }
    #endif

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;

        context.midiMessageCollector.removeNextBlockOfMessages (midiMessages, buffer.getNumSamples());

        for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());

        audioEngine.process(buffer, midiMessages, 0, buffer.getNumSamples());
        scopeDataCollector.process (buffer.getReadPointer (0), (size_t) buffer.getNumSamples());
    }

    //==============================================================================
    bool hasEditor() const
    {
        return true; // (change this to false if you choose to not supply an editor)
    }

    juce::AudioProcessorEditor* createEditor()
    {
        auto editor = new DSPSketchbookAudioProcessorEditor(*this, context, lookAndFeel);
        if(wrapperType == wrapperType_Standalone)
        {
            for (int i = 0; i < juce::TopLevelWindow::getNumTopLevelWindows(); i++)
            {
                auto* w = juce::TopLevelWindow::getTopLevelWindow(i);
                w->setUsingNativeTitleBar(true);
                
#if JUCE_WINDOWS
            //title bar colour change from https://forum.juce.com/t/nativetitlebar-color-change/53608/2
            auto hwnd = w->getPeer()->getNativeHandle();
            BOOL USE_DARK_MODE = true;
            auto result = DwmSetWindowAttribute((HWND)hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE, sizeof(USE_DARK_MODE));
#endif
            }
        }
        
        return editor;
    }

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData)
    {
        juce::MemoryOutputStream stream(destData, true);
        stream.writeString(context.parameterData.toXmlString());
    }

    void setStateInformation (const void* data, int sizeInBytes)
    {
        juce::MemoryBlock block(data, sizeInBytes);
        juce::MemoryInputStream stream(block, false);
        juce::String xmlString = stream.readEntireStreamAsString();
        juce::ValueTree valueTree = juce::ValueTree::fromXml(xmlString);
        
        if (valueTree.isValid())
            sketchbook::loadPreviousPluginState(context, valueTree);
    }

    sketchbook::AudioEngine<VoiceModules, FxModules, ModulationSources> audioEngine;
    
private:
    //==============================================================================
    sketchbook::ScopeDataCollector<float> scopeDataCollector { audioBufferQueue };
    sketchbook::AppLookAndFeel lookAndFeel;
    sketchbook::AudioBufferQueue audioBufferQueue;
    juce::String m_projectName;
    
protected:
    friend class DSPSketchbookAudioProcessorEditor;
    sketchbook::Context context;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DSPSketchbookAudioProcessor)
};
