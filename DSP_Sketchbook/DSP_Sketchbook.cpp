/*
  ==============================================================================

    DSP_Sketchbook.cpp
    Created: 22 Jan 2026 3:01:23pm
    Author:  Billy James

  ==============================================================================
*/
#ifdef DSP_SKETCHBOOK_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif //DSP_SKETCHBOOK_INCLUDED

//include our header files
#include "DSP_Sketchbook.h"

//BINARY DATA
#include "Resources/BinaryData/DSP_SKETCHBOOK_BINARY.cpp"

//ENGINE
#include "Engine/Module.cpp"

//MODULES
#include "Modules/EnvelopeModule.cpp"
#include "Modules/LfoModule.cpp"
#include "Modules/DelayModule.cpp"

#include "Modules/ReverbModule.cpp"
#include "Modules/SamplerModule.cpp"
#include "Modules/DragonFlyReverb/DSP.cpp"

#include "Modules/DragonFlyReverb/freeverb/allpass.cpp"
#include "Modules/DragonFlyReverb/freeverb/biquad.cpp"
#include "Modules/DragonFlyReverb/freeverb/comb.cpp"
#include "Modules/DragonFlyReverb/freeverb/delay.cpp"
#include "Modules/DragonFlyReverb/freeverb/delayline.cpp"
#include "Modules/DragonFlyReverb/freeverb/earlyref.cpp"
#include "Modules/DragonFlyReverb/freeverb/efilter.cpp"
#include "Modules/DragonFlyReverb/freeverb/nrev.cpp"
#include "Modules/DragonFlyReverb/freeverb/nrevb.cpp"
#include "Modules/DragonFlyReverb/freeverb/progenitor.cpp"
#include "Modules/DragonFlyReverb/freeverb/progenitor2.cpp"
#include "Modules/DragonFlyReverb/freeverb/revbase.cpp"
#include "Modules/DragonFlyReverb/freeverb/slot.cpp"
#include "Modules/DragonFlyReverb/freeverb/strev.cpp"
#include "Modules/DragonFlyReverb/freeverb/utils.cpp"
#include "Modules/DragonFlyReverb/freeverb/zrev.cpp"
#include "Modules/DragonFlyReverb/freeverb/zrev2.cpp"

//App
#include "App/PluginEditor.cpp"

//UI
#include "UI/LookAndFeel.cpp"
#include "UI/Slider.cpp"
#include "UI/KeyboardWindow.cpp"
#include "UI/ScopeComponent.cpp"
#include "UI/PluginUIPages.cpp"
