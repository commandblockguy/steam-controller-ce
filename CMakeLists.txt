cmake_minimum_required(VERSION 3.15)
project(midi C)

set(CMAKE_C_STANDARD 90)

include_directories($ENV{CEDEV}/include)

add_custom_target(Make ALL $(MAKE) debug WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# A dummy target to make CLion do stuff
add_executable(dummy
        src/midifile/midifile.c
        src/midifile/midifile.h
        src/main.c)
