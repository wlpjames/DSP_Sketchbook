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

//CONTEXT
#include "App/Context.h"

//BINARY DATA
#include "Resources/BinaryData/DSP_SKETCHBOOK_BINARY.h"

//ENGINE
#include "Engine/Engine.h"
#include "Engine/Module.h"
#include "Engine/Voices.h"

//MODULES
#include "Modules/DelayModule.h"
#include "Modules/ReverbModule.h"
#include "Modules/DistortionModule.h"

#include "Modules/LfoModule.h"
#include "Modules/EnvelopeModule.h"

#include "Modules/SimpleOscModule.h"
#include "Modules/SamplerModule.hpp"

//APP
#include "App/AppDecl.h"
#include "App/PluginProcessor.h"
#include "App/PluginEditor.h"

//UI
#include "UI/Slider.h"
#include "UI/LookAndFeel.h"
#include "UI/ParamaterPages.h"
#include "UI/KeyboardWindow.h"
#include "UI/ScopeComponent.h"
#include "UI/PluginUIPages.h"



