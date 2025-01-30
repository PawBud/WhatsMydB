cmake_minimum_required(VERSION 3.15)
project(Whats_my_dB)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        Whats_my_dB
        main.cpp
        AudioTapper.cpp
        AudioTapper.h
)

target_link_libraries(
        Whats_my_dB
        PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
)