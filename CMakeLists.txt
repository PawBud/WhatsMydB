cmake_minimum_required(VERSION 3.15)
project(Whats_my_dB)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        Whats_my_dB
        src/GetAudioDevice.cpp
        src/AudioTapper.cpp
        src/AudioTapper.h
        src/GetAudioDevice.h
        src/AudioInfo.cpp
        src/AudioInfo.h
)

target_link_libraries(
        Whats_my_dB
        PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
)