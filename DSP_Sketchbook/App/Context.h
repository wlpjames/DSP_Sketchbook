//
//  Context.hpp
//  
//
//  Created by Billy James on 13/03/26.
//

#pragma once

namespace sketchbook
{

class AudioBufferQueue;
class Module;
class KeyboardWindow;

struct Context
{
    juce::MidiKeyboardState midiKeyboardState;
    juce::ValueTree parameterData;
    juce::MidiMessageCollector midiMessageCollector;
    sketchbook::AudioBufferQueue* audioBufferQueue;
    std::unique_ptr<sketchbook::KeyboardWindow> keyboardWindow;
    juce::String projectName;
    
    //TODO: this is here as a work around to acessing
    //TODO: unknown templated functions, a beter method should
    //TODO: probably be found
    std::function<sketchbook::Module*(juce::String)> getLatestPlayingModuleByName;
};

} //end namespace sketchbook
