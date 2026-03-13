//
//  ParameterLoading.h
//  
//
//  Created by Billy James on 13/03/26.
//

#pragma once

namespace sketchbook
{

    ///runs through currently loaded modulation sources to see if there is one by this name
    static bool isValidModulationSourceName(juce::String name, juce::ValueTree valueTree)
    {
        using params = Module::ParamIdents;
        for (auto source : valueTree.getChildWithName(params::MODULATION_SOURCES))
            if (source[params::NAME].toString() == name)
                return true;
            
        return false;
    }

    static void attemptToLoadModuleList(juce::ValueTree currModuleList, juce::ValueTree moduleListToLoad, Context& context)
    {
        using params = Module::ParamIdents;
        
        for (auto currModule : currModuleList)
        {
            if (currModule.getType() != params::MODULE)
                continue;
            
            auto moduleToLoad = moduleListToLoad.getChildWithProperty(params::NAME, currModule[params::NAME].toString());
            
            if (!moduleToLoad.isValid())
            {
                DBG(juce::String("Unknown module '{m}' in loaded parameter state")
                    .replace("{m}", currModule[params::NAME].toString()));
                continue;
            }
            
            //load module meta
            if (currModule.hasProperty(params::ENABLED))
            {
                currModule.setProperty(params::ENABLED, moduleToLoad[params::ENABLED], &context.undoManager);
            }
            
            for (auto currParam : currModule.getChildWithName(params::PARAMETERS))
            {
                //find new parameter
                auto paramToLoad = moduleToLoad.getChildWithName(params::PARAMETERS)
                                                .getChildWithProperty(params::PARAMETER_NAME,
                                                                      currParam[params::PARAMETER_NAME].toString());
                
                if (!paramToLoad.isValid())
                {
                    DBG(juce::String("Unknown parameter '{p}' in module '{m}' of loaded parameter state")
                        .replace("{p}", currParam[params::PARAMETER_NAME].toString())
                        .replace("{m}", currModule[params::NAME].toString()));
                    
                    continue;
                }
                
                if (currParam.getType() == params::PARAMETER_FLOAT
                    || currParam.getType() == params::PARAMETER_INTEGER)
                {
                    float clampedValue = std::clamp(paramToLoad[params::VALUE],
                                                    paramToLoad[params::MIN],
                                                    paramToLoad[params::MAX]);
                    
                    currParam.setProperty(params::VALUE, clampedValue, &context.undoManager);
                }
                else if (currParam.getType() == params::PARAMETER_BOOL)
                {
                    currParam.setProperty(params::VALUE, paramToLoad[params::VALUE], &context.undoManager);
                }
                else if (currParam.getType() == params::PARAMETER_CHOICE)
                {
                    juce::StringArray options = juce::StringArray::fromTokens(currParam[params::PARAMETER_OPTIONS].toString(), ";", "");
                    juce::String valueToLoad = paramToLoad[params::VALUE];
                    
                    if (options.contains(valueToLoad))
                        currParam.setProperty(params::VALUE, paramToLoad[params::VALUE], &context.undoManager);
                    else
                        DBG("Attempting to load unknow options in param");
                }
                else if (currParam.getType() == params::PARAMETER_FILE)
                {
                    currParam.setProperty(params::VALUE, paramToLoad[params::VALUE], &context.undoManager);
                }
                else
                {
                    /// unknown parameter type -- somthing went wrong
                    jassertfalse;
                }
                
                //then copy the modulations -- have to check the inputs and outputs still exist
                currParam.removeAllChildren(&context.undoManager);
                for (auto mapping : paramToLoad)
                {
                    if (mapping.getType() != params::MODULATION)
                        continue;
                    
                    if (isValidModulationSourceName(mapping[params::MODULATION_SOURCE].toString(), context.parameterData))
                    {
                        currParam.addChild(mapping.createCopy(), -1, &context.undoManager);
                    }
                    else
                    {
                        DBG(juce::String("Unable to map param '{p}' to modulation source '{m}'")
                            .replace("{p}", currParam[params::PARAMETER_NAME].toString())
                            .replace("{m}", mapping[params::MODULATION_SOURCE].toString()));
                    }
                }
            }
        }
    }

    static inline void loadPreviousPluginState(sketchbook::Context& context, juce::ValueTree stateToLoad)
    {
        using params = Module::ParamIdents;
        
        context.undoManager.beginNewTransaction("Preset_Change");
        
        //copy over new meta data
        
        
        //load data from modules and parameters
        attemptToLoadModuleList(context.parameterData.getChildWithName(params::MODULES),
                                stateToLoad.getChildWithName(params::MODULES),
                                context);
        
        attemptToLoadModuleList(context.parameterData.getChildWithName(params::MODULATION_SOURCES),
                                stateToLoad.getChildWithName(params::MODULATION_SOURCES),
                                context);
        
        attemptToLoadModuleList(context.parameterData.getChildWithName(params::EFFECT_FILTERS),
                                stateToLoad.getChildWithName(params::EFFECT_FILTERS),
                                context);
    }

} // end namespace sketchbook
