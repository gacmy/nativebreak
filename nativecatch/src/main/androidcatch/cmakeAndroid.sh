#!/bin/bash
abi=armeabi-v7a

v7a=armeabi-v7a
x86=x86
v64=armeabi64-v8a
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DCMAKE_TOOLCHAIN_FILE=~/programs/ndk21/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=x86 \
    -DANDROID_PLATFORM=android-15 \
    -DCMAKE_BUILD=Debug \
    -DANDROID_NDK=~/programs/ndk21 \
    -DANDROID_TOOLCHAIN=clang  ..


#-DANDROID_NATIVE_API_LEVEL=NDK \
