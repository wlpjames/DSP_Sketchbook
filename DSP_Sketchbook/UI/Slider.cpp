/*
  ==============================================================================

    StyledSlider.cpp
    Created: 16 May 2025 12:27:12pm
    Author:  William James

  ==============================================================================
*/

#include "Slider.h"
#include "PluginUIPages.h"
#include "../Engine/Module.h"

//TODO: remove access to the main editor
#include "../App/PluginEditor.h"

namespace sketchbook
{
ModableWidget::ModableWidget(juce::Component* _comp, sketchbook::Context& _ctx)
: comp(_comp)
, ctx(_ctx)
{}

ModableWidget::~ModableWidget() {}

void ModableWidget::setData(juce::ValueTree _data)
{
    data = _data;
    data.addListener(this);
    moduleName = juce::Identifier(data.getParent().getParent()[Module::ParamIdents::NAME]);
    paramName  = juce::Identifier(data[Module::ParamIdents::PARAMETER_NAME]);
}

void ModableWidget::setModulationValue(float value)
{
    modulatedValue = value;
}

float ModableWidget::getModulatedValue()
{
    return modulatedValue;
}

void ModableWidget::setDisplayModulation (bool _shouldDisplay)
{
    shouldDisplay = _shouldDisplay;
    
    if (shouldDisplay && !vBlank)
    {
        vBlank.reset(new juce::VBlankAttachment(comp, [this] (double)
        {
            if (auto mod = ctx.getLatestPlayingModuleByName(moduleName.toString()))
            {
                if (auto param = mod->getModifiedParam(paramName))
                {
                    setModulationValue(param->getModulatedValue());
                }
            }
            
            if (shouldDisplayModulation())
                comp->repaint();
        }));
    }
    else if (!shouldDisplay)
    {
        vBlank.reset();
    }
}

bool ModableWidget::shouldDisplayModulation()
{
    return shouldDisplay;
}

void ModableWidget::valueTreeChildAdded(juce::ValueTree &parentTree, juce::ValueTree &child)
{
    if (child.getType() == Module::ParamIdents::MODULATION)
    {
        setDisplayModulation(true);
    }
}

void ModableWidget::valueTreeChildRemoved(juce::ValueTree &parentTree, juce::ValueTree &child, int indexFromWhichChildWasRemoved)
{
    if (child.getType() == Module::ParamIdents::MODULATION)
    {
        setDisplayModulation(data.getNumChildren() == 0);
    }
}
}//end namespace sketchbook
