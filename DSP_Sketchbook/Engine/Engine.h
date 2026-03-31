#pragma once

#define INIT_POS_PLUCK false

#include "Module.h"
#include "Voices.h"
#include "../Modules/LfoModule.h"
#include "../Modules/EnvelopeModule.h"

namespace sketchbook
{

//==============================================================================
static void assignInstanceIds(juce::Array<Module*> modules)
{
    std::unordered_map<juce::String, int> nameCounts;
    
    for (auto& mod : modules)
    {
        int instanceIndex = nameCounts[mod->getName()]++;
        
        mod->setInstanceId(instanceIndex);
    }
    
    //ensure that if there is only one element then the
    //instance id is not displayed
    for (auto& mod : modules)
    {
        if (mod->getInstanceId() == 0 && nameCounts[mod->getName()] < 2)
        {
            mod->setInstanceId(-1);
        }
    }
}

//------------------------------------------------------------------
//------------------------------------------------------------------

/*
 template <typename... Ts>
 struct TypeList {};
 */
template <typename VoiceModules, typename FxModules = ModuleList<>, typename ModulationSources = ModuleList<LfoModule, LfoModule, EnvelopeModule, EnvelopeModule>>
class AudioEngine
: public sketchbook::SharedModuleDataColector<VoiceModules, FxModules, ModulationSources>
, public sketchbook::VoiceController<VoiceModules, ModulationSources>
{
    public:
    using sharedData = SharedModuleDataColector<VoiceModules, FxModules, ModulationSources>;
    
    //==============================================================================
    AudioEngine()
    : sketchbook::VoiceController<VoiceModules, ModulationSources>(sharedData::getSharedDataRegister())
    , fxChain(sharedData::getSharedDataRegister())
    {
        static_assert(is_module_list<FxModules>::value, "FxModules must be an instance of ModuleList<Modules...>");
        static_assert(is_module_list<VoiceModules>::value, "VoiceModules must be an instance of ModuleList<Modules...>");
        static_assert(is_module_list<ModulationSources>::value, "ModulationSources must be an instance of ModuleList<Modules...>");
        
        //setup parameter state (in this case as if there is no saved state loaded)
        pluginData = getDefaultData();
        
        if (isVoiceEnvelopeNeeded())
        {
            auto envState = EnvelopeModule().getModuleState();
            envState.removeProperty(Module::ParamIdents::ENABLED, nullptr);
            pluginData.getChildWithName(Module::ParamIdents::MODULES).addChild(envState, -1, nullptr);
        }
        
        //set the data in the voice controller to track "voice mode"
        sketchbook::VoiceController<VoiceModules, ModulationSources>::setData(pluginData);
        
        //create a temporary voiceModules object in order to grab the module state data in the structure setup
        VoiceModules tmpVoiceModules(sharedData::getSharedDataRegister());
        ModulationSources tmpModSources(sharedData::getSharedDataRegister());
        setInstanceIdsForAll(tmpVoiceModules, tmpModSources, fxChain);
        
        tmpVoiceModules.forEach([&] (Module& mod, auto)
        {
            pluginData.getChildWithName(Module::ParamIdents::MODULES).addChild(mod.getModuleState(), -1, nullptr);
        });
        
        //do the same for modulation sources
        tmpModSources.forEach([&] (Module& mod, auto)
        {
            pluginData.getChildWithName(Module::ParamIdents::MODULATION_SOURCES).addChild(mod.getModuleState(), -1, nullptr);
        });
        
        //setup individual voices
        for (int i = 0; i < sketchbook::VoiceController<VoiceModules, ModulationSources>::getNumVoices(); i++)
        {
            if (auto v = sketchbook::VoiceController<VoiceModules, ModulationSources>::getVoice(i))
            {
                v->setData(pluginData);
            }
        }
        
        //setup fx parameter
        fxChain.forEach([&] (auto& mod, auto) {
            pluginData.getChildWithName(Module::ParamIdents::EFFECT_FILTERS).addChild(mod.getModuleState(), -1, nullptr);
        });
    }
    
    virtual ~AudioEngine() override
    {
        
    }
    
    //==============================================================================
    void prepare (float samplerate, int blockSize) noexcept override
    {
        sketchbook::VoiceController<VoiceModules, ModulationSources>::prepare(samplerate, blockSize);
        
        fxChain.forEach([&] (auto& mod, auto)
                        {
            mod.prepareToPlay(samplerate, blockSize);
        });
    }
    
    juce::ValueTree getPluginData()
    {
        return pluginData;
    }
    
    /**
     Will search for a module by name - in the latest playing voice or in the fxChain
     */
    Module* getLatestPlayingModuleByName(juce::String name)
    {
        if (auto v = sketchbook::VoiceController<VoiceModules, ModulationSources>::getLatestVoice())
        {
            for (auto mod : v->getModulesArray())
                if (mod->getNameInternal() == name)
                    return mod;
        }
        
        for (auto fxMod : fxChain.toArray())
            if (fxMod->getNameInternal() == name)
                return fxMod;
        
        //no voice is playing?
        return nullptr;
    }
    
    void process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages, int startSample, int numSamples) override
    {
        sketchbook::VoiceController<VoiceModules, ModulationSources>::process(buffer, midiMessages, startSample, numSamples);
        
        fxChain.forEach([&] (auto& mod, auto)
        {
            mod.runModulations();
            if (mod.isModuleEnabled())
                mod.process(buffer);
        });
    }
    
    private:
    
    static juce::ValueTree getDefaultData()
    {
        //header values
        auto tree = juce::ValueTree("DSP_SKETCHBOOK");
        
        //default children for modules
        tree.addChild(juce::ValueTree(Module::ParamIdents::MODULES), -1, nullptr);
        tree.addChild(juce::ValueTree(Module::ParamIdents::MODULATION_SOURCES), -1, nullptr);
        tree.addChild(juce::ValueTree(Module::ParamIdents::EFFECT_FILTERS), -1, nullptr);
        
        //voice Control Parameters
        juce::ValueTree vcModule = Module::getDefaultState("Voice Control");
        vcModule.removeProperty(Module::ParamIdents::ENABLED, nullptr);
        auto voiceMode = Module::Parameter::Choice("Voice Mode", [&] (juce::var) {}, {"Poly", "Mono", "Legato"}, "Poly");
        auto portaTime = Module::Parameter::Float("Porta Time",  [&] (juce::var) {}, 0.0f, 0.f, 1.f);
        vcModule.getChildWithName(Module::ParamIdents::PARAMETERS).addChild(voiceMode->getValueTree(), -1, nullptr);
        vcModule.getChildWithName(Module::ParamIdents::PARAMETERS).addChild(portaTime->getValueTree(), -1, nullptr);
        tree.getChildWithName(Module::ParamIdents::MODULES).addChild(vcModule, -1, nullptr);
        
        return tree;
    }
    
    //set instance ids for repeated instances of a class
    static void setInstanceIdsForAll(VoiceModules& voiceMods, ModulationSources& modSources, FxModules& fxModules)
    {
        assignInstanceIds(voiceMods.toArray());
        assignInstanceIds(modSources.toArray());
        assignInstanceIds(fxModules.toArray());
    }
    
    
    /// Runs through modules and checks if any are marked as requiring
    ///  an adsr envelope
    bool isVoiceEnvelopeNeeded()
    {
        VoiceModules tmpVoiceModules(sharedData::getSharedDataRegister());
        bool output = false;
        tmpVoiceModules.forEach([&] (auto& mod, auto)
        {
            if (mod.getVoiceMonitorType() == Module::VoiceMonitorType::adsr)
                output = true;
        });
        return output;
    }
    
    private:
    
    juce::ValueTree pluginData;
    FxModules fxChain;
};

//==============================================================================
class AudioBufferQueue
{
    public:
    //==============================================================================
    static constexpr size_t order = 11;
    static constexpr size_t bufferSize = 1U << order;
    static constexpr size_t numBuffers = 5;
    
    //==============================================================================
    void push (const float* dataToPush, size_t numSamples)
    {
        jassert (numSamples <= bufferSize);
        
        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (1, start1, size1, start2, size2);
        
        jassert (size1 <= 1);
        jassert (size2 == 0);
        
        if (size1 > 0)
            juce::FloatVectorOperations::copy (buffers[(size_t) start1].data(), dataToPush, (int) juce::jmin (bufferSize, numSamples));
        
        abstractFifo.finishedWrite (size1);
    }
    
    //==============================================================================
    void pop (float* outputBuffer)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead (1, start1, size1, start2, size2);
        
        jassert (size1 <= 1);
        jassert (size2 == 0);
        
        if (size1 > 0)
            juce::FloatVectorOperations::copy (outputBuffer, buffers[(size_t) start1].data(), (int) bufferSize);
        
        abstractFifo.finishedRead (size1);
    }
    
    private:
    //==============================================================================
    juce::AbstractFifo abstractFifo { numBuffers };
    std::array<std::array<float, bufferSize>, numBuffers> buffers;
};

//==============================================================================
template <typename SampleType>
class ScopeDataCollector
{
    public:
    //==============================================================================
    ScopeDataCollector (AudioBufferQueue& queueToUse)
    : audioBufferQueue (queueToUse)
    {}
    
    //==============================================================================
    void process (const SampleType* data, size_t numSamples)
    {
        size_t index = 0;
        
        if (state == State::waitingForTrigger)
        {
            while (index++ < numSamples)
            {
                auto currentSample = *data++;
                
                if (currentSample >= triggerLevel && prevSample < triggerLevel)
                {
                    numCollected = 0;
                    state = State::collecting;
                    break;
                }
                
                prevSample = currentSample;
            }
        }
        
        if (state == State::collecting)
        {
            while (index++ < numSamples)
            {
                buffer[numCollected++] = *data++;
                
                if (numCollected == buffer.size())
                {
                    audioBufferQueue.push (buffer.data(), buffer.size());
                    state = State::waitingForTrigger;
                    prevSample = SampleType (100);
                    break;
                }
            }
        }
    }
    
    private:
    //==============================================================================
    AudioBufferQueue& audioBufferQueue;
    std::array<SampleType, AudioBufferQueue::bufferSize> buffer;
    size_t numCollected;
    SampleType prevSample = SampleType (100);
    
    static constexpr auto triggerLevel = SampleType (0.01);
    
    enum class State { waitingForTrigger, collecting } state { State::waitingForTrigger };
};

} //end namespace sketchbook
