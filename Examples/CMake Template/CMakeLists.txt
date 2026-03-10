#command to use to generate:
#mkdir build
#cd build
#cmake .. -GXcode

#cmake version def
cmake_minimum_required(VERSION 3.22)

#declare project name
project(Sketchbook_Template VERSION 0.0.1)

#compile options 
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

#juce directory
add_subdirectory(../../Submodules/JUCE JUCE)

#set up universal binary for mac
option(UniversalBinary "Build universal binary for mac" ON)
if (UniversalBinary)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")
endif()

juce_add_module(../../DSP_Sketchbook)

juce_add_plugin(Sketchbook_Template
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    IS_SYNTH TRUE                               # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE                       # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE            # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE                # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE RBpd               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE RBSB                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS Standalone AU VST3                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Sketchbook Template")         # The name of the final executable, which can differ from the target name

# add <juce_header.h>
juce_generate_juce_header(Sketchbook_Template)

target_compile_definitions(Sketchbook_Template
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)


#just glob all the files in the source dir
file(GLOB_RECURSE SourceFiles "Source/*.h" "Source/*.hpp" "Source/*.cpp")
target_sources(Sketchbook_Template
    PRIVATE
        ${SourceFiles}
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

#juce modules, plus the DSP Sketchbook module
target_link_libraries(Sketchbook_Template
    PRIVATE
        juce::juce_audio_utils
        DSP_Sketchbook
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
