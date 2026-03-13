//
//  Keyboard Window.cpp
//  
//
//  Created by Billy James on 13/03/26.
//

#include "KeyboardWindow.h"

namespace sketchbook
{
    
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------

KeyboardWindow::KeyboardComponent::KeyboardComponent(juce::MidiKeyboardState& state, Orientation orientation)
: MidiKeyboardComponent(state, orientation)
{
    setMidiChannel (1);
    setColour(juce::MidiKeyboardComponent::ColourIds::blackNoteColourId, {28, 28, 28});
    setColour(juce::MidiKeyboardComponent::ColourIds::whiteNoteColourId, {220, 220, 220});
}

void KeyboardWindow::KeyboardComponent::drawBlackNote(int /*midiNoteNumber*/, juce::Graphics& g, juce::Rectangle<float> area,
                   bool isDown, bool isOver, juce::Colour noteFillColour)
{
    juce::Colour colour = isDown ? juce::Colour(35, 35, 35) : juce::Colour(28, 28, 28);
    g.setColour(colour);
    g.fillRect(area);
}
  
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------

KeyboardWindow::WindowContent::WindowContent(Context& ctx)
: keyboardComponent(ctx.midiKeyboardState, juce::KeyboardComponentBase::horizontalKeyboard)
{
    addAndMakeVisible(&keyboardComponent);
    
    addAndMakeVisible(m_label);
    m_label.setText("DSP Sketchbook", juce::dontSendNotification);
    m_label.setJustificationType(juce::Justification::centredRight);
    m_label.setFont(juce::Font(juce::FontOptions("Futura", 17, juce::Font::FontStyleFlags::plain)));
    m_label.setColour(juce::Label::ColourIds::textColourId, Style::getInstance()->themeColour);
}

void KeyboardWindow::WindowContent::resized()
{
    auto area = getLocalBounds();
    keyboardComponent.setBounds(area.removeFromTop(area.getHeight() * 0.8).reduced(10, 25));
    m_label.setBounds(area.reduced(10, 0).removeFromRight(area.getWidth() / 1.5));
}

void KeyboardWindow::WindowContent::paint(juce::Graphics& g)
{
    g.fillAll(Style::getInstance()->backgroundColour);
}

//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
    
KeyboardWindow::KeyboardWindow(Context& ctx)
: juce::DocumentWindow("Sketchbook - Keyboard",
                       Style::getInstance()->backgroundColour,
                       juce::DocumentWindow::closeButton)
, windowContent(ctx)
{
    
    setContentOwned(&windowContent, true);
    setResizable (false, false);
    setUsingNativeTitleBar (true);
    toFront(true);
    setVisible(true);
    setSize(500, 175);
}

} //end namespace sketchbook
