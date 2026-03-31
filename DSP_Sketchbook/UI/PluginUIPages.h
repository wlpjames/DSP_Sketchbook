/*
  ==============================================================================

    PluginUi.h
    Created: 27 Apr 2025 8:06:57pm
    Author:  William James

  ==============================================================================
*/

#pragma once
#include "ParamaterPages.h"
#include "KeyboardWindow.h"
#include "ScopeComponent.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

namespace sketchbook
{

class HeaderComponent : public juce::Component
{
    public:
    HeaderComponent(sketchbook::Context& ctx);
    
    void resized() override;
    
    private:
    juce::Label titleLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderComponent)
};

class PageMenu : public juce::Component
{
    class MenuButton : public juce::TextButton
    {
        public:
        MenuButton() {}
        
        void paint(juce::Graphics& g) override;
        
        int getWidthOfButtonText();
        
        static const int fontHeight = 14;
    };
    
    public:
    
    PageMenu() {}
    
    ~PageMenu();
    
    void resized();
    
    void addOptions(juce::StringArray options);
    
    void select(int index);
    
    std::function<void(int)> onSelectionFunc;
    
    private:
    juce::Array<MenuButton*> buttons;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PageMenu)
};

class FooterComponent : public juce::Component
{
public:
    
    FooterComponent(Context& ctx, ScopeComponent* scopeComp);
    
    void resized() override;
    
private:
    
    juce::TextButton     m_settingsButton;
    juce::TextButton     m_keyboardButton;
    juce::DrawableButton m_oscDisplayButton;
    juce::TextButton     m_saveStateButton;
    juce::TextButton     m_loadStateButton;
    juce::Label          m_label;
    sketchbook::Context& context;
    std::unique_ptr<juce::Drawable> m_toggleImageOn;
    std::unique_ptr<juce::Drawable> m_toggleImageOff;
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FooterComponent)
};

class MainPanelComponent : public juce::Component
{
    public:
    MainPanelComponent(sketchbook::Context& _context);
    
    ~MainPanelComponent();
    
    void resized();
    
    void paint(juce::Graphics& g);
    
    private:
    juce::MidiKeyboardState midiKeyboardState;
    
    HeaderComponent header;
    ScopeComponent scopeComponent;
    PageMenu pageMenu;
    sketchbook::Pages pages;
    FooterComponent footer;
    
    sketchbook::Context& context;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPanelComponent)
};

} //end namespace sketchbook
