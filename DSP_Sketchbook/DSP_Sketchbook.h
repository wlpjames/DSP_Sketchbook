/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.

 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 DSP_Sketchbook
  vendor:             wlpjames
  version:            0.0.1
  name:               DSP Sketchbook
  description:        A very useful tool for prototyping musical DSP code
  website:            none
  license:            GPLv3

  dependencies:       juce_core juce_data_structures juce_gui_basics juce_audio_processors juce_audio_basics juce_audio_utils juce_dsp juce_animation

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/
#pragma once
#define DSP_SKETCHBOOK_INCLUDED

//Necesary juce includes
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_animation/juce_animation.h>

//TODO: this should live elsewhere
namespace sketchbook
{
class AudioBufferQueue;
class Module;
class KeyboardWindow;
struct Context
{
    juce::MidiKeyboardState midiKeyboardState;
    juce::ValueTree parameterData;
    juce::MidiMessageCollector midiMessageCollector;
    sketchbook::AudioBufferQueue* audioBufferQueue;
    std::unique_ptr<sketchbook::KeyboardWindow> keyboardWindow;
    
    //TODO: this is here as a work around to acessing
    //TODO: unknown templated functions, a beter method should
    //TODO: probably be found
    std::function<sketchbook::Module*(juce::String)> getLatestPlayingModuleByName;
};
}

//BINARY DATA
#include "Resources/BinaryData/DSP_SKETCHBOOK_BINARY.h"

//ENGINE
#include "Engine/Engine.h"
#include "Engine/Module.h"
#include "Engine/Voices.h"

//MODULES
#include "Modules/EnvelopeModule.h"
#include "Modules/Delay.h"
#include "Modules/Reverb.h"

#include "Modules/FX.h"
#include "Modules/ModulationSources.h"
#include "Modules/SimpleOsc.h"

//APP
#include "App/AppDecl.h"
#include "App/PluginProcessor.h"
#include "App/PluginEditor.h"

//UI
#include "UI/StyledSlider.h"
#include "UI/LookAndFeel.h"
#include "UI/ParamaterPages.h"
#include "UI/PluginUi.h"



