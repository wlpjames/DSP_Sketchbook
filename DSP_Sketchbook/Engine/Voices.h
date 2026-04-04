/*
  ==============================================================================

    Voices.h
    Created: 12 Jan 2026 11:01:09am
    Author:  William James

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Module.h"
#include "../Modules/EnvelopeModule.h"

namespace sketchbook
{

class PortamentoController
{
public:
    
    void prepare(float samplerate)
    {
        m_samplerate = samplerate;
        m_increment =  1.f / (m_samplerate * m_timeInSeconds);
    }
    
    void setPortamentoTime(float time)
    {
        m_timeInSeconds = time;
        m_increment =  1.f / (m_samplerate * m_timeInSeconds);
    }
    
    void noteOn(NoteOnEvent event)
    {
        if (event.glideNote.isNoteOn())
        {
            setStartEnd(event.glideNote.getNoteNumber(), event.midiMessage.getNoteNumber());
        }
        else
        {
            setStartEnd(event.midiMessage.getNoteNumber(), event.midiMessage.getNoteNumber());
        }
    }
    
    void reset()
    {
        m_progress01 = 1.f;
        m_centDetune = 0.f;
    }
    
    float getNextPitch(int samplesToSkip = 1)
    {
        if (m_progress01 < 1.f)
        {
            m_centDetune = m_distance * (1.f-m_progress01);
            m_progress01 += m_increment * samplesToSkip;
        }
        else
        {
            m_centDetune=0;
        }
        
        return getPitchHertz(float(m_glideEndNum) + (m_centDetune / 100.f));
    }
    
private:
    
    float getPitchHertz(float noteNumber)
    {
        return 440.f * std::pow (2.f, (noteNumber - 69.f) / 12.f);
    }
    
    void setStartEnd(int startNum, int targetNoteNumber)
    {
        m_glideStartNum  = startNum;
        m_glideEndNum    = targetNoteNumber;
        
        if (m_timeInSeconds == 0.0 || targetNoteNumber == startNum)
        {
            m_centDetune = 0;
            m_progress01 = 1.0f;
            m_distance   = 0;
        }
        else if (m_centDetune <= 0)
        {
            m_centDetune = (m_glideStartNum - m_glideEndNum) * 100;
            m_progress01 = 0.f;
            m_distance   = m_centDetune;
        }
        else
        {
            //shift the direction as is in mid glide
            m_progress01 = 0.f;
            m_centDetune = (m_glideStartNum - m_glideEndNum) * 100 + m_centDetune;
            m_distance   = m_centDetune;
        }
    }
    
    int m_glideStartNum = 60.f;
    int m_glideEndNum   = 60.f;
    
    float m_centDetune = 0.f;
    float m_distance   = 0.f;
    
    float m_progress01 = 0.f;
    float m_increment  = 0.f;
    
    float m_samplerate     = 44100.f;
    float m_timeInSeconds  = 0.3f;
};

template<typename Modules, typename ModSources>
class Voice : public juce::ValueTree::Listener
{
    public:
    
    Voice(SharedModuleDataRegistry& moduleSharedData)
    : m_moduleSharedData(moduleSharedData)
    , moduleList(moduleSharedData)
    , modulationSourceList(moduleSharedData)
    {
        static_assert(is_module_list<Modules>::value,    "Modules must be an instance of ModuleList<...>");
        static_assert(is_module_list<ModSources>::value, "ModSources must be an instance of ModuleList<...>");
        
        assignInstanceIds(moduleList.toArray());
        assignInstanceIds(modulationSourceList.toArray());
        
        //pass pointer to modulation sources around the modules
        moduleList.forEach([&] (auto& mod, auto)
        {
            mod.setModulationSources(modulationSourceList.toArray());
        });
    }
    
    //==============================================================================
    void prepare (float samplerate, int buffersize)
    {
        moduleList.forEach([&] (auto& mod, auto)
        {
            mod.prepareToPlay(samplerate, buffersize);
        });
        
        modulationSourceList.forEach([&] (auto& mod, auto)
        {
            mod.prepareToPlay(samplerate, buffersize);
        });
        
        voiceEnvelope.prepareToPlay(samplerate, buffersize);
        portaController.prepare(samplerate);
    }
    
    //==============================================================================
    void noteOn(const NoteOnEvent& event)
    {
        moduleList.forEach([&] (auto& mod, auto)
        {
            mod.noteOn(event);
        });
        
        modulationSourceList.forEach([&] (auto& mod, auto)
        {
            mod.noteOn(event);
        });
        
        voiceEnvelope.noteOn(event);
        
        portaController.noteOn(event);
        
        noteOnMessage = event.midiMessage;
        m_isPlaying = true;
    }
    
    void noteOff(bool isHardNoteOff)
    {
        //DBG("Voice Note Off");
        moduleList.forEach([&] (auto& mod, auto)
        {
            mod.noteOff(isHardNoteOff);
        });
        
        modulationSourceList.forEach([&] (auto& mod, auto)
        {
            mod.noteOff(isHardNoteOff);
        });
        
        voiceEnvelope.noteOff(isHardNoteOff);
        
        m_isReleasing = true;
    }
    
    void applyMidi(juce::MidiMessage message)
    {
        moduleList.forEach([&] (auto& mod, auto)
        {
            mod.applyMidi(message);
        });
        
        modulationSourceList.forEach([&] (auto& mod, auto)
        {
            mod.applyMidi(message);
        });
        
        voiceEnvelope.applyMidi(message);
    }
    
    void process(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        if (tmpBuffer.getNumSamples() != buffer.getNumSamples()
            || tmpBuffer.getNumChannels() != buffer.getNumChannels())
        {
            tmpBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
        }
        
        float freqHz = portaController.getNextPitch(numSamples);
        
        //calculate any mapped parameters
        modulationSourceList.forEach([&] (auto& mod, auto)
                                     {
            //TODO: this is a bit of a waste of processing - the whole
            //TODO: modulation buffer is processed when only the last
            //TODO: sample will be used
            for (int i = startSample; i < startSample + numSamples; i++)
            {
                float dummyFloatL = 1.f;
                float dummyFloatR = 1.f;
                mod.processSample(&dummyFloatL, &dummyFloatR);
            }
            
            //modulation source parameters may themselves be modulated
            mod.pitchUpdated(freqHz);
            mod.runModulations();
        });
        
        //pre calc the voice env multiplicative buffer
        juce::AudioBuffer<float> adsrBuffer(1, buffer.getNumSamples());
        for (int i = startSample; i < startSample + numSamples; i++)
        {
            adsrBuffer.getWritePointer(0)[i] = 1.f;
            getVoiceADSR()->processSample(adsrBuffer.getWritePointer(0) + i, tmpBuffer.getWritePointer(1) + i);
        }
        
        //TODO: stereo processing
        moduleList.forEach([&] (auto& mod, auto)
        {
            if (!mod.isModuleEnabled()) return;
            
            mod.pitchUpdated(freqHz);
            mod.runModulations();
            tmpBuffer.clear();
            
            for (int i = startSample; i < startSample + numSamples; i++)
            {
                mod.processSample(tmpBuffer.getWritePointer(0) + i, tmpBuffer.getWritePointer(1) + i);
            }
            
            //if uses the voice env then apply the voice env
            if (mod.getVoiceMonitorType() == Module::VoiceMonitorType::adsr)
            {
                for (int i = startSample; i < startSample + numSamples; i++)
                {
                    float env = adsrBuffer.getWritePointer(0)[i];
                    tmpBuffer.getWritePointer(0)[i] *= env;
                    tmpBuffer.getWritePointer(1)[i] *= env;
                }
            }
            
            for (int i = startSample; i < startSample + numSamples; i++)
            {
                buffer.getWritePointer(0)[i] += tmpBuffer.getWritePointer(0)[i];
                buffer.getWritePointer(1)[i] += tmpBuffer.getWritePointer(1)[i];
            }
        });
        
        //if both the adsr and silence detector return true then we can clear the note
        if (checkVoiceEnvelope() && runSilenceDetector(buffer))
            m_isPlaying = false;
    }
    
    void reset()
    {
        moduleList.forEach([&] (auto& mod, auto)
        {
            mod.reset();
        });
        
        modulationSourceList.forEach([&] (auto& mod, auto)
        {
            mod.reset();
        });
        
        voiceEnvelope.reset();
        
        m_isPlaying = false;
        m_isReleasing = false;
        noteOnMessage = juce::MidiMessage();
    }
    
    EnvelopeModule* getVoiceADSR()
    {
        return &voiceEnvelope;
    }
    
    void setData(juce::ValueTree data)
    {
        //set envelope data in the voice adsr
        if (data.getChildWithName(Module::ParamIdents::MODULES)
                .getChildWithProperty(Module::ParamIdents::NAME, getVoiceADSR()->getNameInternal()).isValid())
        {
            getVoiceADSR()->setModuleState(data.getChildWithName(Module::ParamIdents::MODULES)
                                               .getChildWithProperty(Module::ParamIdents::NAME, getVoiceADSR()->getNameInternal()));
        }
        
        //set data in each module
        moduleList.forEach([&] (auto& mod, auto)
        {
            mod.setModuleState(data.getChildWithName(Module::ParamIdents::MODULES).getChildWithProperty(Module::ParamIdents::NAME, mod.getNameInternal()));
        });
        
        //set data in each mod source
        modulationSourceList.forEach([&] (auto& mod, auto)
        {
            jassert(data.getChildWithName(Module::ParamIdents::MODULATION_SOURCES).getChildWithProperty(Module::ParamIdents::NAME, mod.getNameInternal()).isValid());
            
            mod.setModuleState(data.getChildWithName(Module::ParamIdents::MODULATION_SOURCES).getChildWithProperty(Module::ParamIdents::NAME, mod.getNameInternal()));
        });
        
        //listen to the data for glide time
        m_glideTimeParamData = data.getChildWithName(Module::ParamIdents::MODULES)
                                   .getChildWithProperty(Module::ParamIdents::NAME, "Voice Control")
                                   .getChildWithName(Module::ParamIdents::PARAMETERS)
                                   .getChildWithProperty(Module::ParamIdents::NAME, "Porta Time");
        jassert(m_glideTimeParamData.isValid());
        m_glideTimeParamData.addListener(this);
        valueTreePropertyChanged(m_glideTimeParamData, Module::ParamIdents::VALUE);
    }
    
    bool isPlaying()
    {
        return m_isPlaying;
    }
    
    bool isReleasing()
    {
        return m_isReleasing;
    }
    
    int getNoteNumber()
    {
        return noteOnMessage.getNoteNumber();
    }
    
    juce::MidiMessage getCurrNoteOnMessage()
    {
        return noteOnMessage;
    }
    
    juce::Array<Module*> getModulesArray()
    {
        auto arr = moduleList.toArray();
        arr.addArray(modulationSourceList.toArray());
        return arr;
    }
    
    void forEachModule(std::function<void(Module& mod)> fnc)
    {
        moduleList.forEach([&] (auto& mod, auto)
        {
            fnc(mod);
        });
    }
    
    private:
    
    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property)
    {
        if (property == Module::ParamIdents::VALUE)
        {
            portaController.setPortamentoTime(float(tree[Module::ParamIdents::VALUE]));
        }
    }
    
    //returns true if the voice envelope is finished
    bool checkVoiceEnvelope()
    {
        return !voiceEnvelope.isActive();
    }
    
    //returns true if the silence detector detects note finished
    bool runSilenceDetector(juce::AudioBuffer<float>& outputBuffer)
    {
        // silence detector
        bool active = false;
        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        {
            if (outputBuffer.getRMSLevel(ch, 0, outputBuffer.getNumSamples()) > 0.001f)
            {
                active = true;
                break;
            }
        }
        
        return !active;
    }
    
    //==============================================================================
    juce::HeapBlock<char> heapBlock;
    juce::dsp::AudioBlock<float> tempBlock;
    juce::AudioBuffer<float> tmpBuffer;
    
    SharedModuleDataRegistry& m_moduleSharedData;
    Modules moduleList;
    ModSources modulationSourceList;
    EnvelopeModule voiceEnvelope;
    bool m_isPlaying=false;
    bool m_isReleasing = false;
    juce::MidiMessage noteOnMessage;
    
    juce::ValueTree m_glideTimeParamData;
    PortamentoController portaController;
};

template<typename Modules, typename ModSources>
class VoiceController : public juce::ValueTree::Listener
{
    using VoiceType = Voice<Modules, ModSources>;
    const int numVoices = 32;
    
    juce::Array<std::shared_ptr<VoiceType>> voices;
    std::shared_ptr<VoiceType> latestVoice;
    
    enum class ArticulationType
    {
        poly=0, mono, legato
    };
    
    ArticulationType articulationType = ArticulationType::legato;
    std::list<juce::MidiMessage> monoNoteHistory;
    juce::ValueTree m_voiceModeData;
    
public:
    
    VoiceController(SharedModuleDataRegistry& moduleSharedData)
    {
        for (int i = 0; i < numVoices; i++)
        {
            voices.add(std::make_shared<VoiceType>(moduleSharedData));
        }
    }
    
    virtual ~VoiceController() {}
    
    virtual void prepare(float sampleRate, int bufferSize)
    {
        for (auto voice : voices )
        {
            voice->prepare(sampleRate, bufferSize);
        }
    }
    
    virtual void reset()
    {
        for (auto voice : voices )
        {
            voice->reset();
        }
    }
    
    virtual void process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages, int startSample, int numSamples)
    {
        auto prevSample = startSample;
        const auto endSample = startSample + numSamples;

        for (auto it = midiMessages.findNextSamplePosition (startSample); it != midiMessages.cend(); ++it)
        {
            const auto metadata = *it;

            if (metadata.samplePosition >= endSample)
                break;

            const auto smallBlockAllowed = (prevSample == startSample);
            const auto thisBlockSize = smallBlockAllowed ? 1 : 32;

            if (metadata.samplePosition >= prevSample + thisBlockSize)
            {
                for (auto voice : voices)
                {
                    if (voice->isPlaying())
                        voice->process(buffer, prevSample, metadata.samplePosition - prevSample);
                }

                prevSample = metadata.samplePosition;
            }

            handleMidiEvent(metadata.getMessage());
        }

        if (prevSample < endSample)
        {
            for (auto voice : voices)
            {
                if (voice->isPlaying())
                    voice->process(buffer, prevSample, endSample - prevSample);
            }
        }
    }
    
    void setArticulationType(ArticulationType type)
    {
        reset();
        articulationType = type;
    }
    
    VoiceType* getVoice(int index)
    {
        if (index < 0 || index > numVoices)
            return nullptr;
        
        return voices[index].get();
    }
    
    VoiceType* getLatestVoice()
    {
        return latestVoice.get();
    }
    
    const int getNumVoices()
    {
        return numVoices;
    }
    
    //sets data void the voice controller to use
    //in order to track certain parameters
    void setData(juce::ValueTree valueTree)
    {
        m_voiceModeData = valueTree.getChildWithName(Module::ParamIdents::MODULES)
                                   .getChildWithProperty(Module::ParamIdents::NAME, "Voice Control")
                                   .getChildWithName(Module::ParamIdents::PARAMETERS)
                                   .getChildWithProperty(Module::ParamIdents::PARAMETER_NAME, "Voice Mode");
        m_voiceModeData.addListener(this);
        jassert(m_voiceModeData.isValid());
        valueTreePropertyChanged(m_voiceModeData, Module::ParamIdents::VALUE);
    }
    
private:
    
    void valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property)
    {
        if (property == Module::ParamIdents::VALUE)
        {
            auto value = tree[Module::ParamIdents::VALUE].toString();
            
            if (value == "Poly") setArticulationType(ArticulationType::poly);
            else if (value == "Mono") setArticulationType(ArticulationType::mono);
            else if (value == "Legato") setArticulationType(ArticulationType::legato);
        }
    }

    void handleMidiEvent(juce::MidiMessage message)
    {
        if (message.isNoteOn())
            doNoteOn(message);
        else if (message.isNoteOff())
            doNoteOff(message);
        else
            for (auto v : voices)
                v->applyMidi(message);
    }
    
    void doNoteOn(juce::MidiMessage message)
    {
        auto glideFromNote = monoNoteHistory.size() > 0 ? monoNoteHistory.back() : juce::MidiMessage();
        monoNoteHistory.push_back(message);
        
        switch (articulationType)
        {
            case ArticulationType::poly:
            {
                auto nextVoice = getNextVoice();
                
                if (nextVoice->isPlaying())
                {
                    NoteOnEvent noteOn = {message, true, glideFromNote};
                    nextVoice->noteOn(noteOn);
                }
                else
                {
                    NoteOnEvent noteOn = {message, false, glideFromNote};
                    nextVoice->reset();
                    nextVoice->noteOn(noteOn);
                }
                
                break;
            }
            case ArticulationType::mono:
            {
                NoteOnEvent noteOn = { message, false, glideFromNote};
                
                if (latestVoice && latestVoice->isPlaying())
                {
                    latestVoice->noteOff(true);
                }
                
                //replace the mono voice with next voice and give note on
                latestVoice = getNextVoice();
                latestVoice->reset();
                latestVoice->noteOn(noteOn);
                
                break;
            }
            case ArticulationType::legato:
            {
                //if is playing then initiate glide
                if (latestVoice && latestVoice->isPlaying() && !latestVoice->isReleasing())
                {
                    latestVoice->noteOn({ message, true, glideFromNote});
                }
                else
                {
                    //else spin up a new note
                    latestVoice = getNextVoice();
                    latestVoice->reset();
                    latestVoice->noteOn({ message, false, glideFromNote});
                }
                break;
            }
            default:
                break;
        }
    }
    
    void doNoteOff(juce::MidiMessage message)
    {
        monoNoteHistory.remove_if([message] (juce::MidiMessage& note) -> bool
        {
            return message.getNoteNumber() == note.getNoteNumber();
        });
        
        switch (articulationType)
        {
            case ArticulationType::poly:
            {
                //get matching note
                if (auto v = getVoiceByNoteNumber(message.getNoteNumber()))
                    v->noteOff(false);
            }
            case ArticulationType::mono:
            {
                if (latestVoice && message.getNoteNumber() == latestVoice->getNoteNumber())
                {
                    latestVoice->noteOff(false);
                    
                    //if there is another key down then this is given a note on
                    if (monoNoteHistory.size() > 0)
                    {
                        NoteOnEvent nod = { monoNoteHistory.back(), false, latestVoice->getCurrNoteOnMessage()};
                        latestVoice = getNextVoice();
                        latestVoice->noteOn( nod);
                    }
                }
                break;
            }
            case ArticulationType::legato:
            {
                if (latestVoice && message.getNoteNumber() == latestVoice->getNoteNumber())
                {
                    //if there is another key down then this is given a note on
                    if (monoNoteHistory.size() > 0)
                    {
                        NoteOnEvent nod = { monoNoteHistory.back(), true, latestVoice->getCurrNoteOnMessage()};
                        latestVoice->noteOn( nod);
                    }
                    else
                    {
                        latestVoice->noteOff(false);
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    
    std::shared_ptr<VoiceType> getNextVoice()
    {
        for (auto v : voices)
        {
            if (!v->isPlaying())
            {
                return v;
            }
        }
        
        //TODO: steal a voice if no other is found
        return voices[0];
    }
    
    std::shared_ptr<VoiceType> getVoiceByNoteNumber(int noteNumber)
    {
        for (auto v : voices)
        {
            if (v->getNoteNumber() == noteNumber && v->isPlaying() && !v->isReleasing())
            {
                return v;
            }
        }
        return nullptr;
    }
};

} // end namespace sketchbook
