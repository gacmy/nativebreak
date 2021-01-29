cmake_minimum_required(VERSION 3.4.1)
project(catch)
set(LIB_SRC
  ./catchbreak.h
  ./catchbreak.cpp
  ./native-bridge.h
  ./native-bridge.cpp
  ./log.h
  )
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
add_library(catch SHARED ${LIB_SRC})
target_link_libraries(catch stdc++ dl ${log-lib})

