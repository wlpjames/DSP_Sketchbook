/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../UI/PluginUIPages.h"

//==============================================================================
/**
*/
class DSPSketchbookAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DSPSketchbookAudioProcessorEditor (juce::AudioProcessor& p,
                                       sketchbook::Context& ctx,
                                       sketchbook::AppLookAndFeel& laf);
    
    ~DSPSketchbookAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::AudioProcessor& audioProcessor;
    sketchbook::MainPanelComponent mainPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DSPSketchbookAudioProcessorEditor)
};
