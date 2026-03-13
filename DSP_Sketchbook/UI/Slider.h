/*
  ==============================================================================

    StyledSlider.h
    Created: 16 May 2025 12:27:12pm
    Author:  William James

  ==============================================================================
*/

#pragma once


namespace sketchbook
{
class ModableWidget : juce::ValueTree::Listener
{
    public:
    
    ModableWidget(juce::Component* _comp, Context& _ctx);
    
    virtual ~ModableWidget();
    
    //this needs access to the modified paramter
    //be put on a callback list
    //and called from the calculate function
    //but not at audio speed - needs to be animated with vBlank system
    //and will draw whater is the latest animation
    
    void setModulationValue(float value);
    
    float getModulatedValue();
    
    void setDisplayModulation (bool _shouldDisplay);
    
    bool shouldDisplayModulation();
    
    void setData(juce::ValueTree data);
    
    void valueTreeChildAdded(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenAdded) override;
    
    void valueTreeChildRemoved(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
    
    private:
    
    juce::Component* comp;
    std::unique_ptr<juce::VBlankAttachment> vBlank;
    juce::ValueTree data;
    juce::Identifier moduleName = "none";
    juce::Identifier paramName = "none";
    bool  shouldDisplay = false;
    float modulatedValue = 1.f;
    bool  isCentered = false;
    Context& ctx;
};

class StyledSlider : public juce::Slider, public ModableWidget
{
    juce::String name;
    
    public:
    
    static const int SLIDER_WIDTH = 35;
    static const int SLIDER_LABEL_HEIGHT = 12;
    static const int SLIDER_HEIGHT = SLIDER_WIDTH + SLIDER_LABEL_HEIGHT;
    
    StyledSlider(Context& _ctx)
    : ModableWidget(this, _ctx)
    {
        setSliderStyle(SliderStyle::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(NoTextBox, false, 0, 0);
    }
    
    ~StyledSlider() override
    {
    }
    
    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StyledSlider)
};

class StyledLinearSlider : public juce::Slider, public ModableWidget
{
    public:
    StyledLinearSlider(Context& _ctx)
    : ModableWidget(this, _ctx)
    {
        setSliderStyle(SliderStyle::LinearHorizontal);
        setTextBoxStyle(NoTextBox, false, 0, 0);
    }
    
    ~StyledLinearSlider() override
    {
    }
    
    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StyledLinearSlider)
};
} //end namespace sketchbook
