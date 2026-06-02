cmake_minimum_required(VERSION 3.24)

project(yase
  VERSION 0.1.0
  DESCRIPTION "Yet Another Synthesizer Engine"
  LANGUAGES C CXX
)

include(FetchContent)
include(GNUInstallDirs)

option(YASE_BUILD_EXAMPLES "Build YASE example programs" ON)
option(YASE_BUILD_TESTS "Build YASE tests" ON)
option(YASE_BUILD_DOCS "Build YASE documentation with Doxygen" OFF)
option(YASE_FETCH_DEPS "Download dependencies that are not installed locally" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(PkgConfig QUIET)

function(yase_fetch_dependency name)
  if(NOT YASE_FETCH_DEPS)
    message(FATAL_ERROR
      "${name} was not found. Install it with your Linux package manager, "
      "or configure again with -DYASE_FETCH_DEPS=ON."
    )
  endif()
endfunction()

function(yase_add_header_only_dependency target include_dir)
  add_library(${target} INTERFACE)
  target_include_directories(${target} INTERFACE "${include_dir}")
endfunction()

# nlohmann-json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  yase_fetch_dependency("nlohmann-json")
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()
find_path(YASE_NLOHMANN_COMPAT_INCLUDE_DIR json.hpp PATH_SUFFIXES nlohmann)
if(NOT YASE_NLOHMANN_COMPAT_INCLUDE_DIR AND nlohmann_json_SOURCE_DIR)
  set(YASE_NLOHMANN_COMPAT_INCLUDE_DIR "${nlohmann_json_SOURCE_DIR}/include/nlohmann")
endif()

# AudioFile is header-only and does not ship a package config in many installs.
find_path(AUDIOFILE_INCLUDE_DIR AudioFile.h)
if(AUDIOFILE_INCLUDE_DIR)
  yase_add_header_only_dependency(AudioFile::AudioFile "${AUDIOFILE_INCLUDE_DIR}")
else()
  yase_fetch_dependency("AudioFile")
  FetchContent_Declare(
    audiofile
    GIT_REPOSITORY https://github.com/adamstark/AudioFile.git
    GIT_TAG 1.1.1
  )
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(audiofile)
  if(TARGET AudioFile)
    add_library(AudioFile::AudioFile ALIAS AudioFile)
  else()
    yase_add_header_only_dependency(
      AudioFile::AudioFile
      "${audiofile_SOURCE_DIR}"
    )
  endif()
endif()

# PortAudio
find_package(PortAudio QUIET)
if(PortAudio_FOUND)
  if(TARGET PortAudio::PortAudio)
    set(YASE_PORTAUDIO_TARGET PortAudio::PortAudio)
  elseif(TARGET portaudio)
    set(YASE_PORTAUDIO_TARGET portaudio)
  endif()
endif()
if(NOT YASE_PORTAUDIO_TARGET AND PkgConfig_FOUND)
  pkg_check_modules(PORTAUDIO QUIET IMPORTED_TARGET portaudio-2.0)
  if(TARGET PkgConfig::PORTAUDIO)
    set(YASE_PORTAUDIO_TARGET PkgConfig::PORTAUDIO)
  endif()
endif()
if(NOT YASE_PORTAUDIO_TARGET)
  find_path(PORTAUDIO_INCLUDE_DIR portaudio.h)
  find_library(PORTAUDIO_LIBRARY NAMES portaudio)
  if(PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARY)
    add_library(yase_portaudio UNKNOWN IMPORTED)
    set_target_properties(yase_portaudio PROPERTIES
      IMPORTED_LOCATION "${PORTAUDIO_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${PORTAUDIO_INCLUDE_DIR}"
    )
    set(YASE_PORTAUDIO_TARGET yase_portaudio)
  endif()
endif()
if(NOT YASE_PORTAUDIO_TARGET)
  yase_fetch_dependency("PortAudio")
  set(PA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(PA_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
  FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG v19.7.0
  )
  FetchContent_MakeAvailable(portaudio)
  if(TARGET PortAudio)
    set(YASE_PORTAUDIO_TARGET PortAudio)
  elseif(TARGET portaudio)
    set(YASE_PORTAUDIO_TARGET portaudio)
  elseif(TARGET portaudio_static)
    set(YASE_PORTAUDIO_TARGET portaudio_static)
  else()
    message(FATAL_ERROR "PortAudio was fetched, but no CMake target was found.")
  endif()
endif()

# RtMidi
find_package(RtMidi QUIET)
if(RtMidi_FOUND)
  if(TARGET RtMidi::rtmidi)
    set(YASE_RTMIDI_TARGET RtMidi::rtmidi)
  elseif(TARGET rtmidi)
    set(YASE_RTMIDI_TARGET rtmidi)
  endif()
endif()
if(NOT YASE_RTMIDI_TARGET AND PkgConfig_FOUND)
  pkg_check_modules(RTMIDI QUIET IMPORTED_TARGET rtmidi)
  if(TARGET PkgConfig::RTMIDI)
    set(YASE_RTMIDI_TARGET PkgConfig::RTMIDI)
  endif()
endif()
if(NOT YASE_RTMIDI_TARGET)
  find_path(RTMIDI_INCLUDE_DIR RtMidi.h PATH_SUFFIXES rtmidi)
  find_library(RTMIDI_LIBRARY NAMES rtmidi)
  if(RTMIDI_INCLUDE_DIR AND RTMIDI_LIBRARY)
    add_library(yase_rtmidi UNKNOWN IMPORTED)
    set_target_properties(yase_rtmidi PROPERTIES
      IMPORTED_LOCATION "${RTMIDI_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${RTMIDI_INCLUDE_DIR}"
    )
    set(YASE_RTMIDI_TARGET yase_rtmidi)
  endif()
endif()
if(NOT YASE_RTMIDI_TARGET)
  yase_fetch_dependency("RtMidi")
  set(RTMIDI_BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(RTMIDI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(RTMIDI_TARGETNAME_UNINSTALL rtmidi_uninstall CACHE STRING "" FORCE)
  FetchContent_Declare(
    rtmidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG 6.0.0
  )
  FetchContent_MakeAvailable(rtmidi)
  if(TARGET RtMidi::rtmidi)
    set(YASE_RTMIDI_TARGET RtMidi::rtmidi)
  elseif(TARGET rtmidi)
    set(YASE_RTMIDI_TARGET rtmidi)
  else()
    message(FATAL_ERROR "RtMidi was fetched, but no CMake target was found.")
  endif()
endif()

# FFTW3
if(PkgConfig_FOUND)
  pkg_check_modules(FFTW3 QUIET IMPORTED_TARGET fftw3)
  if(TARGET PkgConfig::FFTW3)
    set(YASE_FFTW3_TARGET PkgConfig::FFTW3)
  endif()
endif()
if(NOT YASE_FFTW3_TARGET)
  find_path(FFTW3_INCLUDE_DIR fftw3.h)
  find_library(FFTW3_LIBRARY NAMES fftw3)
  if(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARY)
    add_library(yase_fftw3 UNKNOWN IMPORTED)
    set_target_properties(yase_fftw3 PROPERTIES
      IMPORTED_LOCATION "${FFTW3_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FFTW3_INCLUDE_DIR}"
    )
    set(YASE_FFTW3_TARGET yase_fftw3)
  endif()
endif()
if(NOT YASE_FFTW3_TARGET)
  yase_fetch_dependency("FFTW3")
  set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    fftw3
    URL https://www.fftw.org/fftw-3.3.10.tar.gz
  )
  FetchContent_MakeAvailable(fftw3)
  if(TARGET fftw3)
    set(YASE_FFTW3_INCLUDE_DIR "${fftw3_SOURCE_DIR}/api")
    set(YASE_FFTW3_TARGET fftw3)
  elseif(TARGET FFTW3::fftw3)
    set(YASE_FFTW3_TARGET FFTW3::fftw3)
  else()
    message(FATAL_ERROR "FFTW3 was fetched, but no CMake target was found.")
  endif()
endif()

file(GLOB YASE_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cc")

add_library(yase STATIC ${YASE_SOURCES})
add_library(yase::yase ALIAS yase)
target_include_directories(yase
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${YASE_NLOHMANN_COMPAT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${YASE_FFTW3_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(yase
  PUBLIC
    Threads::Threads
    nlohmann_json::nlohmann_json
    AudioFile::AudioFile
    ${YASE_PORTAUDIO_TARGET}
    ${YASE_RTMIDI_TARGET}
    ${YASE_FFTW3_TARGET}
)
target_compile_features(yase PUBLIC cxx_std_20)

function(yase_add_program target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PRIVATE yase::yase)
  set_target_properties(${target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
  )
endfunction()

if(YASE_BUILD_EXAMPLES)
  find_package(Curses QUIET)

  yase_add_program(auto_load examples/AutoLoad/auto.cc)
  yase_add_program(big_sound examples/BigSound/bigsound.cc)
  yase_add_program(buffer examples/Buffer/buffer.cc)
  yase_add_program(buttons examples/Buttons/buttons.cc)
  yase_add_program(chorus examples/Chorus/chorus.cc)
  yase_add_program(clock examples/Clock/clock.cc)
  yase_add_program(drums examples/Drums/drums.cc)
  yase_add_program(fft examples/FFT/fft.cc)
  yase_add_program(faders examples/Faders/faders.cc)
  yase_add_program(karplus_strong examples/KarplusStrong/karplus_strong.cc)
  yase_add_program(keys examples/Keys/keys.cc)
  yase_add_program(leds examples/LEDs/leds.cc)
  yase_add_program(launchpad examples/Launchpad/launchpad.cc)
  yase_add_program(lorenz examples/Lorenz/lorenz.cc)
  yase_add_program(midi2analog examples/Midi2Analog/midi2analog.cc)
  yase_add_program(midi_report examples/Midi/midi_report.cc)
  yase_add_program(modular examples/Modular/modular.cc)
  yase_add_program(mono examples/Mono/monophonic.cc examples/Mono/mono.cc)
  yase_add_program(multi_seq examples/Multi/multi-seq.cc examples/Multi/mono.cc)
  yase_add_program(multi_clock examples/MultiClock/multi_clock.cc)
  yase_add_program(play_sample examples/PlaySample/play_sample.cc)
  yase_add_program(polyphonic examples/Poly/polyphonic.cc examples/Poly/voice.cc)
  yase_add_program(record examples/Record/record.cc)
  yase_add_program(sample_and_hold examples/SampleAndHold/sample_and_hold.cc)
  yase_add_program(sampled_drum examples/SampledDrum/sampled_drum.cc examples/SampledDrum/voice.cc)
  yase_add_program(saw examples/Saw/saw.cc)
  yase_add_program(sequence examples/SimpleSequencer/sequence.cc)
  yase_add_program(sine_wave examples/Sine/sine_wave.cc)
  yase_add_program(sine_threads examples/Threads/sine_threads.cc)
  yase_add_program(wave_tester examples/Waves/wave_tester.cc)
  yase_add_program(wavetable examples/Wavetable/wavetable.cc)

  if(CURSES_FOUND)
    yase_add_program(controls examples/Controls/controls.cc)
    target_link_libraries(controls PRIVATE ${CURSES_LIBRARIES})
    target_include_directories(controls PRIVATE ${CURSES_INCLUDE_DIRS})

    yase_add_program(custom_event examples/CustomEvent/custom_event.cc)
    target_link_libraries(custom_event PRIVATE ${CURSES_LIBRARIES})
    target_include_directories(custom_event PRIVATE ${CURSES_INCLUDE_DIRS})
  else()
    message(STATUS "ncurses was not found; skipping controls and custom_event examples.")
  endif()
endif()

if(YASE_BUILD_TESTS)
  enable_testing()
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    yase_fetch_dependency("GoogleTest")
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(yase_tests
    test/main.cc
    test/event.cc
    test/gtest.cc
    test/passthrough.cc
  )
  target_link_libraries(yase_tests PRIVATE yase::yase GTest::gtest)
  set_target_properties(yase_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
  )
  add_test(NAME yase_tests COMMAND yase_tests)
endif()

if(YASE_BUILD_DOCS)
  find_package(Doxygen REQUIRED)
  add_custom_target(docs
    COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/docs.config"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    VERBATIM
  )
endif()

install(TARGETS yase ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
