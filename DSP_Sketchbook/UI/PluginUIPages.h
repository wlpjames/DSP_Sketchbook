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
    HeaderComponent(sketchbook::Context& ctx)
    {
        addAndMakeVisible(titleLabel);
        titleLabel.setFont(Style::getInstance()->themeFont.withHeight(26));
        titleLabel.setColour(juce::Label::ColourIds::textColourId, Style::getInstance()->themeColour);
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setText(ctx.projectName, juce::dontSendNotification);
    }
    
    void resized() override
    {
        titleLabel.setBounds(getLocalBounds());
    }
    
    private:
    juce::Label titleLabel;
};

class PageMenu : public juce::Component
{
    class MenuButton : public juce::TextButton
    {
        public:
        
        void paint(juce::Graphics& g) override
        {
            //just draw text
            g.setColour(Style::getInstance()->themeColour);
            juce::Font font(Style::getInstance()->themeFont.withHeight(fontHeight));
            
            if (getToggleState())
                font.setUnderline(true);
            
            g.setFont(font);
            g.drawText(getButtonText(), getLocalBounds(), juce::Justification::centredBottom);
        }
        
        int getWidthOfButtonText()
        {
            juce::Font font(Style::getInstance()->themeFont.withHeight(fontHeight));
            juce::GlyphArrangement ga;
            ga.addLineOfText(font, getButtonText(), 0, 0);
            return (int)std::ceil(ga.getBoundingBox(0, -1, true).getWidth());
        }
        
        static const int fontHeight = 14;
    };
    
    public:
    ~PageMenu()
    {
        for (auto& b : buttons)
            if (b)
                delete b;
    }
    
    void resized()
    {
        //arrange the buttons to have equal spacing between the text, considering the of the text
        auto area = getLocalBounds();
        int totalTextWidth = 0;

        for (auto& b : buttons)
            totalTextWidth += b->getWidthOfButtonText();
        
        int spacing = (getWidth() - totalTextWidth) / (buttons.size());
        int x = 0;//spacing / 2;
        
        for (auto& b : buttons)
        {
            int w = b->getWidthOfButtonText() + spacing;
            b->setBounds(x, area.getY(), w, area.getHeight());
            x += w;
        }
    }
    
    void addOptions(juce::StringArray options)
    {
        //not tested for run time changes
        jassert(buttons.size() == 0);
        
        for (int i = 0; i < options.size(); i++)
        {
            auto* button = new MenuButton();
            addAndMakeVisible(button);
            button->setButtonText(options[i]);
            button->setRadioGroupId(1);
            button->setClickingTogglesState(true);
            button->onClick = [sp = SafePointer<PageMenu>(this), i] ()
            {
                if (!sp) return;
                
                if (sp->onSelectionFunc && sp->buttons[i]->getToggleState())
                    sp->onSelectionFunc(i);
            };
            buttons.add(button);
        }
    }
    
    void select(int index)
    {
        if (index >= 0 && index < buttons.size())
        {
            buttons[index]->setToggleState(true, juce::sendNotification);
        }
    }
    
    std::function<void(int)> onSelectionFunc;
    
    private:
    juce::Array<MenuButton*> buttons;
};

class FooterComponent : public juce::Component
{
public:
    
    FooterComponent(Context& ctx, ScopeComponent* scopeComp)
    : context(ctx)
    , m_oscDisplayButton("Scope Toggle", juce::DrawableButton::ButtonStyle::ImageFitted)
    {
        addAndMakeVisible(m_settingsButton);
        m_settingsButton.setButtonText("Settings");
        m_settingsButton.setClickingTogglesState(false);
        m_settingsButton.onClick = [sp = SafePointer<FooterComponent>(this)] ()
        {
            if (auto* w = sp->findParentComponentOfClass <juce::StandaloneFilterWindow>())
            {
                w->getPluginHolder()->showAudioSettingsDialog();
                
                //set the look and feel
                auto& desktop = juce::Desktop::getInstance();
                for (int i = 0; i < desktop.getNumComponents(); ++i)
                {
                    auto* comp = desktop.getComponent(i);
                    
                    if (auto* dw = dynamic_cast<juce::DialogWindow*>(comp))
                    {
                        dw->setLookAndFeel(&sp->getLookAndFeel());
                        // Also propagate to content component
                        if (auto* content = dw->getContentComponent())
                            content->setLookAndFeel(&sp->getLookAndFeel());
                        break;
                    }
                }
            }
        };
        
        addAndMakeVisible(m_keyboardButton);
        m_keyboardButton.setButtonText("Keys");
        m_keyboardButton.setClickingTogglesState(false);
        m_keyboardButton.onClick = [sp = SafePointer<FooterComponent>(this)] ()
        {
            if (!sp) return;
            
            if (!sp->context.keyboardWindow)
                sp->context.keyboardWindow.reset(new KeyboardWindow(sp->context));
            else
                sp->context.keyboardWindow.reset();
                
            return;
        };
        
        //TODO: setup images for this button
        addAndMakeVisible(m_oscDisplayButton);
        m_oscDisplayButton.setButtonStyle(juce::DrawableButton::ImageOnButtonBackground);
        m_oscDisplayButton.setClickingTogglesState(true);
        m_oscDisplayButton.setToggleState(true, juce::dontSendNotification);
        m_oscDisplayButton.onClick = [sp = SafePointer<FooterComponent>(this), scopeComp] ()
        {
            if (!sp) return;
            scopeComp->showScope(sp->m_oscDisplayButton.getToggleState()
                                        ? ScopeComponent::scopeToShow::freq
                                        : ScopeComponent::scopeToShow::osc);
        };
        
        m_toggleImageOn = juce::Drawable::createFromImageData(DSP_SKETCHBOOK_BINARY::Scope_Toggle_On_svg,
                                                               DSP_SKETCHBOOK_BINARY::Scope_Toggle_On_svgSize);
        m_toggleImageOff = juce::Drawable::createFromImageData(DSP_SKETCHBOOK_BINARY::Scope_Toggle_Off_svg,
                                                               DSP_SKETCHBOOK_BINARY::Scope_Toggle_Off_svgSize);
        m_oscDisplayButton.setImages(m_toggleImageOff.get(),
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     m_toggleImageOn.get());
        
        
        
        
        addAndMakeVisible(m_label);
        m_label.setText("DSP Sketchbook", juce::dontSendNotification);
        m_label.setJustificationType(juce::Justification::centredRight);
        m_label.setFont(juce::Font(juce::FontOptions("Futura", 17, juce::Font::FontStyleFlags::plain)));
        m_label.setColour(juce::Label::ColourIds::textColourId, Style::getInstance()->themeColour);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        int buttonWidth = area.getWidth() / 6.5;
        m_settingsButton.setBounds(area.removeFromLeft(buttonWidth));
        area.removeFromLeft(5);
        m_keyboardButton.setBounds(area.removeFromLeft(buttonWidth));
        area.removeFromLeft(5);
        m_oscDisplayButton.setBounds(area.removeFromLeft(buttonWidth));
        m_label.setBounds(area.removeFromRight(buttonWidth*3));
    }
    
private:
    
    juce::TextButton     m_settingsButton;
    juce::TextButton     m_keyboardButton;
    juce::DrawableButton m_oscDisplayButton;
    juce::Label          m_label;
    sketchbook::Context& context;
    std::unique_ptr<juce::Drawable> m_toggleImageOn;
    std::unique_ptr<juce::Drawable> m_toggleImageOff;
};

class MainPanelComponent : public juce::Component
{
    public:
    MainPanelComponent(sketchbook::Context& _context)
    : header(_context)
    , scopeComponent(*_context.audioBufferQueue)
    , pages(_context)
    , footer(_context, &scopeComponent)
    , context(_context)
    {
        addAndMakeVisible(scopeComponent);
        
        midiKeyboardState.addListener(&context.midiMessageCollector);
        
        addAndMakeVisible(pages);
        pages.setData(context.parameterData);
        
        addAndMakeVisible(pageMenu);
        pageMenu.onSelectionFunc = [sp = SafePointer<MainPanelComponent>(this)] (int index)
        {
            if (!sp)
                return;
            
            sp->pages.showPage(index);
        };
        pageMenu.addOptions({"VOICE", "EFFECTS", "MOD SOURCES", "MATRIX"});
        pageMenu.select(0);
        
        addAndMakeVisible(header);
        addAndMakeVisible(footer);
    }
    
    ~MainPanelComponent()
    {
        midiKeyboardState.removeListener(&context.midiMessageCollector);
    }
    
    void resized()
    {
        //TODO: these areas should be percentages for resizing
        
        const int height = getHeight();
        const int width  = getWidth();
        auto area = getLocalBounds().reduced(width * 0.04, 0);
        
        header.setBounds(area.removeFromTop(height * 0.18).reduced(0, height * 0.05));
        scopeComponent.setBounds(area.removeFromTop(height * 0.1).reduced(width * 0.05, 0));
        area.removeFromTop(height * 0.05);
        pageMenu.setBounds(area.removeFromTop(height * 0.045));
        area.removeFromTop(height * 0.02);
        footer.setBounds(area.removeFromBottom(height * 0.057).reduced(0, height * 0.011));
        pages.setBounds(area);
    }
    
    void paint(juce::Graphics& g)
    {
        g.fillAll(Style::getInstance()->backgroundColour);
    }
    
    private:
    juce::MidiKeyboardState midiKeyboardState;
    
    HeaderComponent header;
    ScopeComponent scopeComponent;
    PageMenu pageMenu;
    sketchbook::Pages pages;
    FooterComponent footer;
    
    sketchbook::Context& context;
};

} //end namespace sketchbook
