/*
  ==============================================================================

    LookAndFeel.h
    Created: 16 May 2025 12:19:10pm
    Author:  William James

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "StyledSlider.h"

namespace sketchbook
{

class Style
{
public:
    
    Style()
    {
        themeFont.setTypefaceName("Inter Medium");
    }
    
    juce::Colour backgroundColour = {22, 22, 22};
    juce::Colour themeColour      = juce::Colour(juce::uint8(215), juce::uint8(195), juce::uint8(186), 0.4f);
    juce::Colour highlightColour  = juce::Colours::whitesmoke.withAlpha(0.4f);
    juce::Font   themeFont;
    
    JUCE_DECLARE_SINGLETON_INLINE(Style, false)
};

class AppLookAndFeel : public juce::LookAndFeel_V4
{
    public:
    
    AppLookAndFeel()
    {
        setDefaultSansSerifTypefaceName(Style::getInstance()->themeFont.getTypefaceName());
        
        setColour(juce::ResizableWindow::backgroundColourId, Style::getInstance()->backgroundColour);
        
        setColour(juce::PopupMenu::backgroundColourId,       Style::getInstance()->backgroundColour);
        
        setColour(juce::TextButton::textColourOnId,          Style::getInstance()->themeColour.brighter());
        setColour(juce::TextButton::textColourOffId,         Style::getInstance()->themeColour);
        
        setColour(juce::ScrollBar::ColourIds::thumbColourId, Style::getInstance()->themeColour);
        
        setColour(juce::Label::textColourId,                 Style::getInstance()->themeColour);
        
        setColour(juce::ComboBox::ColourIds::textColourId,   Style::getInstance()->themeColour);
        setColour(juce::ComboBox::ColourIds::arrowColourId,  Style::getInstance()->themeColour);
        
        setColour(juce::ListBox::backgroundColourId,         Style::getInstance()->backgroundColour);
        setColour(juce::ListBox::outlineColourId,            Style::getInstance()->themeColour);
        setColour(juce::ListBox::textColourId,               Style::getInstance()->themeColour);
        
        setColour(juce::ToggleButton::textColourId,          Style::getInstance()->themeColour);
        setColour(juce::ToggleButton::tickColourId,          Style::getInstance()->highlightColour);
        setColour(juce::ToggleButton::tickDisabledColourId,  Style::getInstance()->themeColour);
        
        setColour(juce::Slider::backgroundColourId,          juce::Colours::transparentBlack);
    }
    
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        return {slider.getLocalBounds().reduced(2, 0), {}};
    }
    
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        
        g.fillAll (slider.findColour (juce::Slider::backgroundColourId));
        
        if (style != juce::Slider::LinearHorizontal)
        {
            juce::LookAndFeel_V3::drawLinearSlider(g, x, y, width, height,
                                                   sliderPos, minSliderPos,
                                                   maxSliderPos, style, slider);
            return;
        }
        
        auto ss = dynamic_cast<StyledLinearSlider*>(&slider);
        if (!ss)
            return;
        
        //draw track line
        const int thumbHeight = 6;
        g.setColour(Style::getInstance()->themeColour);
        g.drawLine(x, (height / 2) + 0.5f, x + width, (height / 2) + 0.5f, 1.5f);
        
        //draw the modulation amount semi transparent above
        if (ss->shouldDisplayModulation())
        {
            g.setColour(Style::getInstance()->highlightColour);
            float modValue = ss->getModulatedValue();
            float modulatedSliderPos = width * modValue;
            if (modulatedSliderPos > sliderPos)
                g.fillRect (int(sliderPos), height / 2, int(modulatedSliderPos - sliderPos), thumbHeight / 2);
            else
                g.fillRect (int(modulatedSliderPos), height / 2, int(sliderPos - modulatedSliderPos), thumbHeight / 2);
        }
        
        //draw thumb
        g.setColour(Style::getInstance()->themeColour.withAlpha(.65f));
        g.drawLine(sliderPos, (height / 2) - (1 + (thumbHeight / 2)), sliderPos, (height / 2) + thumbHeight, 2.f);
    }
    
    //MARK: POPUP Menu (right click)
    void drawPopupMenuBackground (juce::Graphics &g, int width, int height) override
    {
        g.setColour(Style::getInstance()->backgroundColour);
        g.fillRoundedRectangle(juce::Rectangle<int>(width, height).toFloat(), 6.0);
    }
    
    void drawPopupMenuItem (juce::Graphics &g,  const juce::Rectangle< int > &area,
                            bool isSeparator,   bool isActive,
                            bool isHighlighted, bool isTicked,
                            bool hasSubMenu,    const juce::String &text,
                            const juce::String &shortcutKeyText,
                            const juce::Drawable *icon,
                            const juce::Colour *textColour) override
    {
        if (isSeparator)
        {
            auto a = area.reduced(5);
            g.setColour(Style::getInstance()->themeColour);
            g.drawLine(a.getX(), a.getHeight() / 2, a.getWidth(), a.getHeight() / 2, 2.0);
        }
        else
        {
            if (isHighlighted)
            {
                g.setColour(Style::getInstance()->highlightColour);
            }
            else
            {
                g.setColour(Style::getInstance()->themeColour);
            }
            
            g.drawText(text, area, juce::Justification::centred);
        }
    }
    
    int getPopupMenuBorderSize() override
    {
        return 0;
    }
    
    void preparePopupMenuWindow(juce::Component& c) override
    {
        c.setOpaque(false);
    }
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour &backgroundColour,
                              bool  shouldDrawButtonAsHighlighted,
                              bool  shouldDrawButtonAsDown) override
    {
        auto area = button.getLocalBounds().toFloat();
        g.setColour(button.getToggleState() ? Style::getInstance()->themeColour.brighter() : Style::getInstance()->themeColour);
        area = area.reduced(1.5);
        g.drawRoundedRectangle(area, area.getHeight() / 2, 1.f);
    }
    
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return Style::getInstance()->themeFont.withHeight(buttonHeight / 2.5);
    }
    
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& comboBox) override
    {
        auto area = comboBox.getLocalBounds().toFloat();

        g.setColour(Style::getInstance()->themeColour);
        g.drawRoundedRectangle(area.reduced(1.5f), 3.5, 1.0f);
        
        juce::Rectangle<int> arrowZone (width - 25, 0, 18, height);
        juce::Path path;
        path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
        path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
        path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);
        
        g.setColour(Style::getInstance()->themeColour.withAlpha((comboBox.isEnabled() ? 0.9f : 0.2f)));
        g.strokePath(path, juce::PathStrokeType (2.0f));
    }
    
    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return Style::getInstance()->themeFont.withHeight(12);
    }
    
    void drawCallOutBoxBackground (juce::CallOutBox&, juce::Graphics& g, const juce::Path&, juce::Image&) override
    {
        g.fillAll(juce::Colours::transparentBlack);
    }
    
    int getCallOutBoxBorderSize (const juce::CallOutBox&) override
    {
        return 0;
    }
    
    float getCallOutBoxCornerSize (const juce::CallOutBox&) override
    {
        return 0;
    }
    
    int getDefaultScrollbarWidth() override
    {
        return 5;
    }
    
    void drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, [[maybe_unused]] bool isMouseDown) override
    {
        juce::Rectangle<float> thumbBounds;
        
        if (isScrollbarVertical)
            thumbBounds = juce::Rectangle<int>(x, thumbStartPosition, width, thumbSize).toFloat();
        else
            thumbBounds = juce::Rectangle<int>(thumbStartPosition, y, thumbSize, height).toFloat();
        
        thumbBounds.reduce(2.75, 0);

        auto c = scrollbar.findColour (juce::ScrollBar::ColourIds::thumbColourId);
        g.setColour (isMouseOver ? c.brighter (0.25f) : c);
        g.fillRoundedRectangle (thumbBounds.toFloat(), thumbBounds.getWidth() / 2);
    }
};

} //end namespace sketchbook
