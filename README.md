# DSP Sketchbook

UNDER CONSTRUCTION, PLEASE CHECK BACK WITH US SOON
 
<img src="https://github.com/wlpjames/DSP_Sketchbook/blob/main/Images/dsp_sketchbook_ui.png" alt="Alt Text" width="50%" height="50%">

## Get started with the DSP_Sketchbook_Template Repo
- [Template Repo](https://github.com/wlpjames/DSP_Sketchbook_Template)

## Prototyping DSP as simply as:

```cpp

#include <JuceHeader.h>

class DSPModule : public sketchbook::Module
{
public:
    //==============================================================================
    DSPModule()
    {   
        ///provide an array of parameters
        setModuleParameters
        ({
            Parameter::Float("Float Param", [&] (float value)
            {
                return;
            }, 0.0, 0.0, 1.0),
        });
    }
    
    void prepareToPlay(float samplerate, int buffersize) override
    {
    }
    
    void noteOn(const sketchbook::NoteOnEvent& event) override
    {

    }
    
    void noteOff(bool) override
    {

    }
    
    void reset() override
    {

    }
    
    void processSample(float* sample) override
    {

    }
    
    juce::String getName() override
    {
        return "Module Name";
    }
};

using VoiceModules 			= sketchbook::ModuleList<CustomModule, sketchbook::Sampler, sketchbook::SimpleOsc>;
using PostProscessEffects 	= sketchbook::ModuleList<sketchbook::Reverb, sketchbook::Delay>;
using ModulationSources 	= sketchbook::ModuleList<sketchbook::EnvelopeModule, sketchbook::LfoModule>;

/// Decare the app here!
/// The first argument is the title that will be displayed at the top of the UI
SKETCHBOOK_DECLARE_APP("Sketchbook Template",
                       VoiceModules,         /* list of modules in voice    */
                       PostProscessEffects,  /* list of post process effects*/
                       ModulationSources)    /* list of modulation sources  */

```
