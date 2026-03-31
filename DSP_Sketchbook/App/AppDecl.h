
#define SKETCHBOOK_DECLARE_APP(Name, VoiceModules, EffectsModules, ModulationSourceModules)                 \
                                                                                                            \
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()                                                    \
{                                                                                                           \
    return new DSPSketchbookAudioProcessor<VoiceModules, EffectsModules, ModulationSourceModules>(Name);    \
}                                                                                                           \
