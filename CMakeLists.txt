cmake_minimum_required(VERSION 3.22)

project(Mz7 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# JUCE lives beside the repository rather than inside it, shared with every other JUCE
# project on the machine. Override with -DJUCE_PATH=... if it is somewhere else.
#
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Where JUCE is checked out")

if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR
        "No JUCE at ${JUCE_PATH}.\n"
        "  cd ${CMAKE_CURRENT_SOURCE_DIR}/..\n"
        "  git clone --depth 1 https://github.com/juce-framework/JUCE\n"
        "or pass -DJUCE_PATH=<wherever it is>.")
endif()

add_subdirectory("${JUCE_PATH}" JUCE)

#
# The per-user VST3 folder, which needs no elevation - JUCE's default of
# C:\Program Files\Common Files\VST3 cannot be written without running as administrator.
#
if(WIN32)
    set(VST3_DEFAULT "$ENV{LOCALAPPDATA}/Programs/Common/VST3")
elseif(APPLE)
    set(VST3_DEFAULT "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
else()
    set(VST3_DEFAULT "$ENV{HOME}/.vst3")
endif()

set(VST3_DESTINATION "${VST3_DEFAULT}" CACHE PATH "Where to install the built VST3")

set(MZ7_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND MZ7_FORMATS AU)
elseif(UNIX)
    list(APPEND MZ7_FORMATS LV2)   # what Linux hosts - Ardour, Carla, Qtractor - load best
endif()

#
# PRODUCT_NAME is what a player sees. "Mz7" rather than "DX7": Yamaha and DX7 are Yamaha's
# trademarks, and this is an independent emulation.
#
# The two codes are the plugin's IDENTITY and must never change once anyone has saved a
# project with it - a VST3's class ID is made from them, and a host finds the plugin in a
# saved set by that ID.
#
juce_add_plugin(Mz7
    COMPANY_NAME                "simozzer"
    COMPANY_COPYRIGHT           "Created by Simon Moscrop"
    PRODUCT_NAME                "Mz7"
    PLUGIN_MANUFACTURER_CODE    Smoz          # never change - see above
    PLUGIN_CODE                 Mz07          # never change - see above
    LV2URI                      "https://github.com/simozzer/Mz7"   # never change - an LV2's identity

    # JUCE does not inherit this from project().
    VERSION                     "${PROJECT_VERSION}"

    VST3_COPY_DIR               "${VST3_DESTINATION}"
    VST3_CATEGORIES             Instrument Synth

    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    # Installed by install_vst3.cmake instead, so a host holding the old binary open is a
    # message rather than a failed build.
    COPY_PLUGIN_AFTER_BUILD     FALSE

    FORMATS                     ${MZ7_FORMATS})

target_sources(Mz7 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Params.cpp
    Source/DX7/Banks.cpp
    Source/DX7/Engine.cpp
    Source/DX7/Patch.cpp)

target_include_directories(Mz7 PRIVATE Source)

add_custom_command(TARGET Mz7_VST3 POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
            "-DSRC=${CMAKE_CURRENT_BINARY_DIR}/Mz7_artefacts/$<CONFIG>/VST3/Mz7.vst3"
            "-DDST=${VST3_DESTINATION}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/install_vst3.cmake"
    VERBATIM)

target_compile_definitions(Mz7 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Mz7
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

#
# The engine's own checks. They link no JUCE: the DX7 model is plain C++, so it can be held to
# pitch, level and the sysex round trip, and every factory voice played, before a plugin is
# wrapped round it.
#
add_executable(EngineCheck
    Tests/EngineCheck.cpp
    Source/DX7/Engine.cpp
    Source/DX7/Patch.cpp)

add_executable(BankCheck
    Tests/BankCheck.cpp
    Source/DX7/Banks.cpp
    Source/DX7/Engine.cpp
    Source/DX7/Patch.cpp)

target_include_directories(EngineCheck PRIVATE Source/DX7)
target_include_directories(BankCheck PRIVATE Source/DX7)

enable_testing()
add_test(NAME engine COMMAND EngineCheck)
add_test(NAME banks COMMAND BankCheck)
