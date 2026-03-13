//
//  Keyboard Window.hpp
//  
//
//  Created by Billy James on 13/03/26.
//

#pragma once

namespace sketchbook
{

class KeyboardWindow : public juce::DocumentWindow
{
public:
    
    class KeyboardComponent : public juce::MidiKeyboardComponent
    {
    public:
        KeyboardComponent(juce::MidiKeyboardState& state, Orientation orientation);
        
        void drawBlackNote(int /*midiNoteNumber*/, juce::Graphics& g, juce::Rectangle<float> area,
                           bool isDown, bool isOver, juce::Colour noteFillColour) override;
    };
    
    class WindowContent : public juce::Component
    {
    public:
        WindowContent(Context& ctx);
        
        void resized() override;
        
        void paint(juce::Graphics& g) override;
        
    private:
        KeyboardComponent keyboardComponent;
        juce::Label m_label;
    };
    
    KeyboardWindow(Context& ctx);
    
private:
    WindowContent windowContent;
};

} //end namespace KeyboardWindow
