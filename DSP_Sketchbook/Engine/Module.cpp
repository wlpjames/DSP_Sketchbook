/*
  ==============================================================================

    Module.cpp
    Created: 29 Apr 2025 2:19:02pm
    Author:  William James

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Module.h"

namespace sketchbook
{
using namespace juce;

const int UIBufferSize = 126;
const int UiAnimFPS = 65;

const Identifier Module::ParamIdents::MODULE                = Identifier("MODULE");
const Identifier Module::ParamIdents::NAME                  = Identifier("NAME");
const Identifier Module::ParamIdents::INSTANCE_ID           = Identifier("INSTANCE_ID");
const Identifier Module::ParamIdents::PARAMETERS            = Identifier("PARAMETERS");
const Identifier Module::ParamIdents::PARAMETER_FLOAT       = Identifier("PARAMETER_FLOAT");
const Identifier Module::ParamIdents::PARAMETER_INTEGER     = Identifier("PARAMETER_INTEGER");
const Identifier Module::ParamIdents::PARAMETER_BOOL        = Identifier("PARAMETER_BOOL");
const Identifier Module::ParamIdents::PARAMETER_CHOICE      = Identifier("PARAMETER_CHOICE");
const Identifier Module::ParamIdents::PARAMETER_FILE        = Identifier("PARAMETER_FILE");
const Identifier Module::ParamIdents::PARAMETER_NAME        = Identifier("NAME");
const Identifier Module::ParamIdents::VALUE                 = Identifier("VALUE");
const Identifier Module::ParamIdents::IS_MODABLE            = Identifier("IS_MODABLE");
const Identifier Module::ParamIdents::MIN                   = Identifier("MIN");
const Identifier Module::ParamIdents::MAX                   = Identifier("MAX");
const Identifier Module::ParamIdents::ENABLED               = Identifier("ENABLED");
const Identifier Module::ParamIdents::PARAMETER_OPTIONS     = Identifier("OPTIONS");

//MODULATIONS
const Identifier Module::ParamIdents::MODULATION            = Identifier("MODULEATION");
const Identifier Module::ParamIdents::MODULATION_SOURCE     = Identifier("MODULATION_SOURCE");
const Identifier Module::ParamIdents::MOD_AMOUNT            = Identifier("MOD_AMOUNT");
const Identifier Module::ParamIdents::MOD_CENTRED           = Identifier("MOD_CENTRED");
const Identifier Module::ParamIdents::MOD_REVERSED          = Identifier("MOD_REVERSED");

//MODULE LISTS
const Identifier Module::ParamIdents::MODULES               = Identifier("MODULES");
const Identifier Module::ParamIdents::MODULATION_SOURCES    = Identifier("MODULATION_SOURCES");
const Identifier Module::ParamIdents::EFFECT_FILTERS        = Identifier("EFFECT_FILTERS");

void RingBuffer::setSize(int bufferLen)
{
    data.setSize(1, bufferLen);
    len = bufferLen;
    
    if (writePoint > bufferLen)
        writePoint = 0;
}

void RingBuffer::appendSingleSample(float sample)
{
    writePoint += 1;
    if (writePoint >= len)
        writePoint = 0;
    
    data.setSample(0, writePoint, sample);
}

float RingBuffer::getLastSample()
{
    return data.getSample(0, writePoint);
}

void RingBuffer::copyFromBuffer(AudioBuffer<float> &buffer)
{
    jassert(buffer.getNumSamples() == data.getNumSamples());
    data.copyFrom(0, 0, buffer, 0, 0, len);
}

AudioBuffer<float>& RingBuffer::getData()
{
    return data;
}

AudioBuffer<float> RingBuffer::getBuffer()
{
    AudioBuffer<float> b;
    
    if (writePoint != 0)
    {
        b.setSize(1, len);
        b.clear();
        b.copyFrom(0, 0, data, 0, writePoint, len - writePoint);
        b.copyFrom(0, len-(writePoint+1), data, 0, 0, writePoint);
    }
    else
    {
        b.makeCopyOf(data);
    }
    
    return b;
}

//float param
Module::ParameterInternal::ParameterInternal(juce::String name, std::function<void(float)> callback, float _initialValue, float _min, float _max)
: parameterValue(_initialValue)
, paramName(name)
, m_min(_min)
, m_max(_max)
, paramChangedCallback(callback)
{
    data = ValueTree(ParamIdents::PARAMETER_FLOAT)
        .setProperty(ParamIdents::PARAMETER_NAME, name, nullptr)
        .setProperty(ParamIdents::VALUE, parameterValue, nullptr)
        .setProperty(ParamIdents::MIN, m_min, nullptr)
        .setProperty(ParamIdents::MAX, m_max, nullptr);
    
    if (data.isValid())
        data.addListener(this);
    
    //send an initial value to the callback
    setValue(data[Module::ParamIdents::VALUE]);
    paramChangedCallback(getValue());
}

//integer param
Module::ParameterInternal::ParameterInternal(String name, std::function<void(int)> callback, int initialValue, int _min, int _max)
: parameterValue(initialValue)
, paramName(name)
, m_min(_min)
, m_max(_max)
, paramChangedCallback(callback)
{
    data = ValueTree(ParamIdents::PARAMETER_INTEGER)
        .setProperty(ParamIdents::PARAMETER_NAME, name, nullptr)
        .setProperty(ParamIdents::VALUE, parameterValue, nullptr)
        .setProperty(ParamIdents::MIN, m_min, nullptr)
        .setProperty(ParamIdents::MAX, m_max, nullptr);
    
    if (data.isValid())
        data.addListener(this);
    
    //send an initial value to the callback
    setValue(data[Module::ParamIdents::VALUE]);
    paramChangedCallback(getValue());
}

//bool parameter
Module::ParameterInternal::ParameterInternal(String name, std::function<void(bool)> callback, bool initialValue)
: parameterValue(initialValue)
, paramName(name)
, paramChangedCallback(callback)
{
    data = ValueTree(ParamIdents::PARAMETER_BOOL)
        .setProperty(ParamIdents::PARAMETER_NAME, name, nullptr)
        .setProperty(ParamIdents::VALUE, parameterValue, nullptr);
    
    if (data.isValid())
        data.addListener(this);
    
    //send an initial value to the callback
    setValue(data[Module::ParamIdents::VALUE]);
    paramChangedCallback(getValue());
}

//choice parameter
Module::ParameterInternal::ParameterInternal(String name, std::function<void(juce::String)> callback, StringArray options, String initialValue)
: parameterValue(initialValue)
, paramName(name)
, paramChangedCallback(callback)
{
    data = ValueTree(ParamIdents::PARAMETER_CHOICE)
        .setProperty(ParamIdents::PARAMETER_NAME, name, nullptr)
        .setProperty(ParamIdents::PARAMETER_OPTIONS, options.joinIntoString(";"), nullptr)
        .setProperty(ParamIdents::VALUE, parameterValue, nullptr);
    
    if (data.isValid())
        data.addListener(this);
    
    //send an initial value to the callback
    setValue(data[Module::ParamIdents::VALUE]);
    paramChangedCallback(getValue());
}

//file Parameter
Module::ParameterInternal::ParameterInternal(juce::String name, std::function<void(juce::String)> callback, String initialValue)
: parameterValue(initialValue)
, paramName(name)
, paramChangedCallback(callback)
{
    data = ValueTree(ParamIdents::PARAMETER_FILE)
        .setProperty(ParamIdents::PARAMETER_NAME, name, nullptr)
        .setProperty(ParamIdents::VALUE, initialValue, nullptr);
    
    if (data.isValid())
        data.addListener(this);
    
    setValue(data[Module::ParamIdents::VALUE]);
    paramChangedCallback(getValue());
}

Module::ParameterInternal::~ParameterInternal()
{
    data.removeListener(this);
}

void Module::ParameterInternal::setValue(var value)
{
    parameterValue = value;
}

var Module::ParameterInternal::getValue()
{
    return parameterValue;
}

Identifier Module::ParameterInternal::getName()
{
    return paramName;
}

void Module::ParameterInternal::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    if (property == ParamIdents::VALUE)
    {
        parameterValue = treeWhosePropertyHasChanged[ParamIdents::VALUE];
        
        if (paramChangedCallback)
            paramChangedCallback(parameterValue);
    }
}

ValueTree Module::ParameterInternal::getValueTree()
{
    return data;
}

void Module::ParameterInternal::setValueTree(ValueTree newData)
{
    if (data.isValid())
        data.removeListener(this);
    
    data = newData;
    data.addListener(this);
}

float Module::ParameterInternal::getMinValue()
{
    return m_min;
}

float Module::ParameterInternal::getMaxValue()
{
    return m_max;
}

float Module::ParameterInternal::getRange()
{
    return float(m_max) - float(m_min);
}

Module::ModifiedParameter::Mapping::Mapping(ValueTree _data,
                                            Module* _sourceModule)
: data(_data)
, sourceModule(_sourceModule)
{
    jassert(data.isValid());
    data.addListener(this);
}

Module::ModifiedParameter::Mapping::Mapping(const Module::ModifiedParameter::Mapping& other)
: data(other.data)
, sourceModule(other.sourceModule)
, amount(other.amount)
, centred(other.centred)
, reversed(other.reversed)
{
    data.addListener(this);
}

Module::ModifiedParameter::Mapping::~Mapping()
{
    if (data.isValid())
        data.removeListener(this);
}

void Module::ModifiedParameter::Mapping::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    if (property == ParamIdents::MOD_AMOUNT)
    {
        amount = float(treeWhosePropertyHasChanged[ParamIdents::MOD_AMOUNT]);
    }
    else if (property == ParamIdents::MOD_CENTRED)
    {
        centred = bool(treeWhosePropertyHasChanged[ParamIdents::MOD_CENTRED]);
    }
    else if (property == ParamIdents::MOD_REVERSED)
    {
        reversed = bool(treeWhosePropertyHasChanged[ParamIdents::MOD_REVERSED]);
    }
}

Module::ModifiedParameter::ModifiedParameter(std::shared_ptr<ParameterInternal> _parameter)
: parameter(_parameter)
{
    parameterName = parameter->getName();
}

void Module::ModifiedParameter::addMapping(ValueTree data, Module* source)
{
    currMappings.add(Mapping(data, source));
    
    if (currMappings.size() == 1 && onModulationBeginOrEnd)
        onModulationBeginOrEnd(true);
}

void Module::ModifiedParameter::removeMapping(ValueTree data)
{
    currMappings.removeIf([data] (Mapping& map) {
        return map.data == data;
    });
    
    if (currMappings.size() == 0 && onModulationBeginOrEnd)
        onModulationBeginOrEnd(false);
}

void Module::ModifiedParameter::calculateAndSendModulation()
{
    float normModVal = 0.f;
    
    for (auto& mapping : currMappings)
    {
        float modVal = mapping.sourceModule->internalBuffer.getLastSample();
        
        if (mapping.reversed)
            modVal = 1.f - modVal;
        
        //re-map from 0to1 to -1to1
        if (mapping.centred)
            modVal = modVal * 2.f - 1.f;
        
        normModVal += modVal * mapping.amount;
    }
    
    float modifiedValue = float(parameter->getValue());
    
    modifiedValue = modifiedValue + normModVal * parameter->getRange();
    modifiedValue = std::clamp(modifiedValue,
                               float(parameter->getMinValue()),
                               float(parameter->getMaxValue()));
    
    modulatedValue = modifiedValue;
    parameter->paramChangedCallback(modifiedValue);
}

int Module::ModifiedParameter::getNumMappings()
{
    return currMappings.size();
}

float Module::ModifiedParameter::getModulatedValue()
{
    return modulatedValue;
}

void Module::ModifiedParameter::reset()
{
    modulatedValue = float(parameter->getValue());
}

inline Identifier Module::ModifiedParameter::getParamName()
{
    return parameterName;
}

Module::Module()
{
    internalBuffer.setSize(UIBufferSize);
    internalBuffer.getData().clear();
    
    //set up a default module state -- must remain nameless until
    //the subclass is fully initiated
    moduleState = getDefaultState("");
    moduleState.addListener(this);
}

Module::~Module() {}

void Module::prepareToPlay(float samplerate, int buffersize) {}

void Module::noteOn(const NoteOnEvent& event) {}

void Module::noteOff(bool) {}

void Module::reset()
{
    internalBuffer.data.clear();
    
    for (auto p : modifiedParameters)
        p->reset();
}

void Module::applyMidi(const MidiMessage& message) {}

void Module::process(juce::AudioBuffer<float>& buffer)
{
    //incase this function is not overloaded, make
    //individual calls to the other process function
    for(int i = 0; i < buffer.getNumSamples(); i++)
        processSample(buffer.getWritePointer(0) + i, buffer.getWritePointer(1) + i);
}

bool Module::isModuleEnabled()
{
    return moduleState[ParamIdents::ENABLED];
}

void Module::setIsDefaultEnabled(bool defaultEnabled)
{
    isDefaultEnabled = defaultEnabled;
}

void Module::setVoiceMonitorType(Module::VoiceMonitorType type)
{
    voiceMonitorType = type;
}

Module::VoiceMonitorType Module::getVoiceMonitorType()
{
    return voiceMonitorType;
}

AudioBuffer<float> Module::getLastBuffer()
{
    return internalBuffer.getBuffer();
}

std::shared_ptr<Module::ModifiedParameter> Module::getModifiedParam(Identifier paramName)
{
    for (auto mp : modifiedParameters)
    {
        if (mp->getParamName() == paramName)
            return mp;
    }
    
    return nullptr;
}

juce::String Module::getNameInternal()
{
    return getName() + (instanceId > -1 ? juce::String("_") + juce::String(instanceId+1) : juce::String());
}

void Module::setInstanceId(int _id)
{
    instanceId = _id;
    
    //set the name acording to the instance Id
    moduleState.setProperty(ParamIdents::NAME, getNameInternal(), nullptr);
    
    if (instanceId >= 0)
        moduleState.setProperty(ParamIdents::INSTANCE_ID, _id, nullptr);
    
    else if (moduleState.hasProperty(ParamIdents::INSTANCE_ID))
        moduleState.removeProperty(ParamIdents::INSTANCE_ID, nullptr);
}

int Module::getInstanceId()
{
    return instanceId;
}

void Module::valueTreeChildAdded(ValueTree &parentTree, ValueTree &childWhichHasBeenAdded)
{
    if (childWhichHasBeenAdded.getType() != ParamIdents::MODULATION)
        return;
    
    //search for the source
    Module* sourceModule = nullptr;
    String sourceName = childWhichHasBeenAdded[ParamIdents::MODULATION_SOURCE].toString();
    for (auto source : modulationSources)
    {
        if (source->getNameInternal() == sourceName)
        {
            sourceModule = source;
            break;
        }
    }
    
    if (!sourceModule)
    {
        DBG(String("received mapping to invalid source module: ") + sourceName);
        return;
    }
    
    //search for the parameter this modulation is for
    for (auto p : modifiedParameters)
    {
        if (p->getParamName().toString() == childWhichHasBeenAdded.getParent()[ParamIdents::PARAMETER_NAME].toString())
        {
            p->addMapping(childWhichHasBeenAdded, sourceModule);
            break;
        }
    }
    
    
    return;
}

void Module::valueTreeChildRemoved (ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
{
    if (childWhichHasBeenRemoved.getType() == ParamIdents::MODULATION)
    {
        for (auto p : modifiedParameters)
        {
            if (p->getParamName().toString() == parentTree[ParamIdents::PARAMETER_NAME].toString())
            {
                p->removeMapping(childWhichHasBeenRemoved);
            }
        }
    }
    
    return;
}

void Module::setModuleParameters(Array< std::shared_ptr< Module::ParameterInternal>> parameters)
{
    moduleState.setProperty(ParamIdents::NAME, getNameInternal(), nullptr);
    moduleState.setProperty(ParamIdents::ENABLED, isDefaultEnabled, nullptr);
    moduleParameters = parameters;
    
    //need to set the value tree to include these parameters
    for (auto parameter : parameters)
    {
        moduleState.getChildWithName(ParamIdents::PARAMETERS).addChild(parameter->getValueTree(), -1, nullptr);
    }
    
    modifiedParameters.clear();
    for (auto p : parameters)
    {
        modifiedParameters.add(std::make_shared<ModifiedParameter>(p));
    }
}

void Module::applyAllParameters()
{
    for (auto p : moduleParameters)
    {
        p->paramChangedCallback(p->getValue());
    }
}

void Module::runModulations()
{
    for (auto param : modifiedParameters)
    {
        if (param->getNumMappings() > 0)
        {
            param->calculateAndSendModulation();
        }
    }
}

void Module::setModulationSources(Array<Module*> modSources)
{
    modulationSources = modSources;
}

ValueTree Module::getModuleState()
{
    return moduleState;
}

void Module::setModuleState(ValueTree newModuleState)
{
    if (moduleState.isValid())
        moduleState.removeListener(this);
    
    moduleState = newModuleState;
    moduleState.addListener(this);
    
    if (moduleState.hasProperty(ParamIdents::INSTANCE_ID))
        setInstanceId(moduleState[ParamIdents::INSTANCE_ID]);
    
    //itterate the parameter and swap out the states they are following
    for (auto& parameter : moduleParameters)
    {
        jassert(moduleState.getChildWithName(ParamIdents::PARAMETERS)
                .getChildWithProperty(ParamIdents::PARAMETER_NAME, parameter->getName().toString()).isValid());
        
        parameter->setValueTree(moduleState.getChildWithName(ParamIdents::PARAMETERS)
                                .getChildWithProperty(ParamIdents::PARAMETER_NAME, parameter->getName().toString()));
    }
}

} //end namespace sketchbook

