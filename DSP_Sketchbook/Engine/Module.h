/*
  ==============================================================================

    Module.h
    Created: 29 Apr 2025 10:00:44am
    Author:  William James

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
namespace sketchbook
{

struct NoteOnEvent
{
    juce::MidiMessage midiMessage;
    bool isLegatoNoteOn = false;
    juce::MidiMessage glideNote;
};

class RingBuffer
{
    public:
    
    RingBuffer() {}
    void setSize(int bufferLen);
    void appendSingleSample(float sample);
    float getLastSample();
    void copyFromBuffer(juce::AudioBuffer<float> &buffer);
    
    /*
     returns the raw data
     */
    juce::AudioBuffer<float>& getData();
    
    /*
     returns a linier buffer as if were not a ring buffer
     */
    juce::AudioBuffer<float> getBuffer();
    
    juce::AudioBuffer<float> data;
    
    int writePoint = 0;
    int len = 0;
};

class Module : public juce::ValueTree::Listener
{
    public:
    
    class SharedData {};
    class SharedDataHolderBase
    {
        public:
        virtual ~SharedDataHolderBase() {}
        virtual std::shared_ptr<SharedData> createSharedData() const = 0;
    };
    
    template <typename SharedDataType>
    class SharedDataHolder : public SharedDataHolderBase
    {
        public:
        virtual ~SharedDataHolder() {}
        std::shared_ptr<SharedData> createSharedData() const override
        {
            return std::make_shared<SharedDataType>();
        }
    };
    
    enum VoiceMonitorType
    {
        adsr, silenceDetection
    };
    
    private:
    class ParameterInternal : public juce::ValueTree::Listener
    {
        juce::var parameterValue;
        juce::String paramName;
        juce::ValueTree data;
        
        juce::var min;
        juce::var max;
        
        public:
        
        //Float paramter
        ParameterInternal(juce::String name, std::function<void(float)> callback, float initialValue, float min, float max);
        
        //int parameter
        ParameterInternal(juce::String name, std::function<void(int)> callback, int initialValue, int min, int max);
        
        //bool parameter
        ParameterInternal(juce::String name, std::function<void(bool)> callback, bool initialValue);
        
        //choice parameter
        ParameterInternal(juce::String name, std::function<void(juce::String)> callback, juce::StringArray options, juce::String initialValue);
        
        //file Parameter
        ParameterInternal(juce::String name, std::function<void(juce::String)> callback, juce::String initialValue);
        
        ~ParameterInternal() override;
        
        void setValue(juce::var value);
        
        juce::var getValue();
        
        juce::Identifier getName();
        
        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged, const juce::Identifier &property) override;
        
        std::function<void(juce::var)> paramChangedCallback;
        
        juce::ValueTree getValueTree();
        
        void setValueTree(juce::ValueTree newData);
        
        float getMinValue();
        
        float getMaxValue();
        
        float getRange();
        
    };
    
    public:
    
    enum class parameterType
    {
        floatParam = 0, intParam, booleanParam, choiceParam, fileParam, numParameterTypes
    };
    
    struct Parameter
    {
        static std::shared_ptr<ParameterInternal> Float(juce::String name, std::function<void(float)> callback, float initialValue, float min, float max)
        {
            return std::make_shared<ParameterInternal>(name, callback, initialValue, min, max);
        }
        
        static std::shared_ptr<ParameterInternal> Integer(juce::String name, std::function<void(int)> callback, int initialValue, int min, int max)
        {
            return std::make_shared<ParameterInternal>(name, callback, initialValue, min, max);
        }
        
        static std::shared_ptr<ParameterInternal> Boolean(juce::String name, std::function<void(bool)> callback, bool initialValue=true)
        {
            return std::make_shared<ParameterInternal>(name, callback, initialValue);
        }
        
        static std::shared_ptr<ParameterInternal> Choice(juce::String name, std::function<void(juce::String)> callback, juce::StringArray options, juce::String initialValue)
        {
            return std::make_shared<ParameterInternal>(name, callback, options, initialValue);
        }
        
        static std::shared_ptr<ParameterInternal> File(juce::String name, std::function<void(juce::String)> callback, juce::String initialValue)
        {
            return std::make_shared<ParameterInternal>(name, callback, initialValue);
        }
    };
    
    public:
    
    class ModifiedParameter
    {
        
        private:
        class Mapping : juce::ValueTree::Listener
        {
            
            public:
            
            juce::ValueTree data;
            Module* sourceModule;
            float amount = 1.0;
            
            bool centred = false;
            bool reversed = false;
            
            Mapping(juce::ValueTree _data, Module* _sourceModule);
            Mapping(const Mapping& other);
            virtual ~Mapping();
            void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                          const juce::Identifier &property) override;
        };
        
        juce::Array<Mapping> currMappings;
        std::shared_ptr<ParameterInternal> parameter;
        float modulatedValue;
        juce::Identifier parameterName;
        
        public:
        std::function<void(bool)> onModulationBeginOrEnd;
        
        ModifiedParameter(std::shared_ptr<ParameterInternal> _parameter);
        
        void addMapping(juce::ValueTree data, Module* source);
        
        void removeMapping(juce::ValueTree data);
        
        void calculateAndSendModulation();
        
        float getModulatedValue();
        
        void reset();
        
        int getNumMappings();
        
        inline juce::Identifier getParamName();
        
        static juce::ValueTree defaultMappingTo(juce::String sourceModName)
        {
            return juce::ValueTree(ParamIdents::MODULATION).setProperty(ParamIdents::MODULATION_SOURCE, sourceModName, nullptr)
                .setProperty(ParamIdents::MOD_AMOUNT,      1.f,   nullptr)
                .setProperty(ParamIdents::MOD_CENTRED,     false, nullptr)
                .setProperty(ParamIdents::MOD_REVERSED,    false, nullptr);
        }
    };
    
    public:
    
    Module();
    
    virtual ~Module() override;
    
    virtual void prepareToPlay(float samplerate, int buffersize);
    
    virtual void noteOn(const NoteOnEvent& event);
    
    virtual void noteOff(bool isHardNoteOff);
    
    virtual void reset();
    
    virtual void applyMidi(const juce::MidiMessage& message);
    
    virtual void processSample(float* sample) {};
    
    virtual void process(juce::AudioBuffer<float>& buffer) {}
    
    virtual void pitchUpdated(float newPitch) {}
    
    virtual juce::String getName() = 0;
    
    bool isModuleEnabled();
    
    /**
     By default this will be true
     */
    void setIsDefaultEnabled(bool defaultEnabled);
    
    void setVoiceMonitorType(VoiceMonitorType type);
    
    VoiceMonitorType getVoiceMonitorType();
    
    juce::AudioBuffer<float> getLastBuffer();
    
    std::shared_ptr<ModifiedParameter> getModifiedParam(juce::Identifier paramName);
    
    juce::String getNameInternal();
    
    void setInstanceId(int _id);
    
    int getInstanceId();
    
    //a non recursive function that will run through any modified
    //parameters and calculate their next values
    void runModulations();
    
    static juce::ValueTree getDefaultState(juce::String name)
    {
        juce::ValueTree output(ParamIdents::MODULE);
        output.setProperty(ParamIdents::NAME, name, nullptr);
        output.setProperty(ParamIdents::ENABLED, true, nullptr);
        
        output.addChild(juce::ValueTree(ParamIdents::PARAMETERS), 0, nullptr);
        
        return output;
    }
    
    static juce::ValueTree parameter(juce::Identifier name, juce::var initValue)
    {
        return juce::ValueTree(name).setProperty(ParamIdents::VALUE, initValue, nullptr);
    }
    
    static float shiftFreqByCent(float freq, float cent)
    {
        float value = freq * powf(2.0f, cent/1200.0f);
        return value;
    }
    
    struct ParamIdents
    {
        static const juce::Identifier MODULE;
        static const juce::Identifier NAME;
        static const juce::Identifier INSTANCE_ID;
        static const juce::Identifier PARAMETERS;
        static const juce::Identifier PARAMETER_NAME;
        static const juce::Identifier VALUE;
        static const juce::Identifier IS_MODABLE;
        static const juce::Identifier MIN;
        static const juce::Identifier MAX;
        static const juce::Identifier ENABLED;
        static const juce::Identifier PARAMETER_OPTIONS;
        
        //PARAMETER TYPES
        static const juce::Identifier PARAMETER_FLOAT;
        static const juce::Identifier PARAMETER_INTEGER;
        static const juce::Identifier PARAMETER_BOOL;
        static const juce::Identifier PARAMETER_CHOICE;
        static const juce::Identifier PARAMETER_FILE;
        
        //MODULATIONS
        static const juce::Identifier MODULATION;
        static const juce::Identifier MODULATION_SOURCE;
        static const juce::Identifier MOD_AMOUNT;
        static const juce::Identifier MOD_CENTRED;
        static const juce::Identifier MOD_REVERSED;
        
        //MODULE LISTS
        static const juce::Identifier MODULES;
        static const juce::Identifier MODULATION_SOURCES;
        static const juce::Identifier EFFECT_FILTERS;
    };
    
    void valueTreeChildAdded(juce::ValueTree &parentTree,
                             juce::ValueTree &childWhichHasBeenAdded) override;
    
    void valueTreeChildRemoved(juce::ValueTree &parentTree,
                               juce::ValueTree &childWhichHasBeenRemoved,
                               int indexFromWhichChildWasRemoved) override;
    
    void setModuleParameters(juce::Array<std::shared_ptr<Module::ParameterInternal>> parameters);
    
    /*
     This can be used to set shared data across instaces in the same voice
    */
    void setSharedData(std::shared_ptr<SharedData> sharedDataObject);
    
    /*
     returns an instance of the shared data that is used across voices for this module
    */
    template<typename SharedDataType>
    std::shared_ptr<SharedDataType> getSharedData()
    {
        return std::static_pointer_cast<SharedDataType>(m_sharedData);
    }
    
    void applyAllParameters();
    
    void setModulationSources(juce::Array<Module*> modSources);
    
    juce::ValueTree getModuleState();
    
    void setModuleState(juce::ValueTree newModuleState);
    
    RingBuffer internalBuffer;
    juce::ValueTree moduleState;
    float lastProcessedSample=0;
    
    private:
    juce::Array< std::shared_ptr<Module::ModifiedParameter>> modifiedParameters;
    juce::Array< std::shared_ptr<Module::ParameterInternal>> moduleParameters;
    bool isProcessingBuffer=false;
    juce::Array<Module*> modulationSources;
    VoiceMonitorType voiceMonitorType = adsr;
    int instanceId = -1; ///If there are more that one instances of a module, this number will be appened to the name - else will be -1
    bool isDefaultEnabled = true;
    std::shared_ptr<SharedData> m_sharedData;
};

//==============================================================================
template <typename... Modules>
class ModuleList
{
    std::tuple<Modules...> modules;
    
    public:
    
    template <typename Fn>
    constexpr void forEach(Fn&& fn)
    {
        forEachInTuple(fn, modules);
    }
    
    juce::Array<Module*> toArray()
    {
        juce::Array<Module*> result;
        
        forEach([&] (auto& mod, auto)
                {
            result.add(&mod);
        });
        
        return result;
    }
    
    private:
    
    template <typename Fn, typename Tuple, size_t... Ix>
    static constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple, std::index_sequence<Ix...>)
    {
        (fn (std::get<Ix> (tuple), std::integral_constant<size_t, Ix>()), ...);
    }
    
    template <typename T>
    using TupleIndexSequence = std::make_index_sequence<std::tuple_size_v<std::remove_cv_t<std::remove_reference_t<T>>>>;
    
    template <typename Fn, typename Tuple>
    static constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple)
    {
        forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple>{});
    }
};

//in order to static assert on wrong types
template<typename T>
struct is_module_list : std::false_type {};

template<typename... Modules>
struct is_module_list<ModuleList<Modules...>> : std::true_type {};

} //end namespace sketchbook
