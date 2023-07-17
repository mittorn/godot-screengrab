#!/bin/sh
FLAGS="-Os -Wl,-Os -Wl,--gc-sections -ffunction-sections -fno-stack-protector -U_FORTIFY_SOURCE -flto -fipa-pta -fdevirtualize-at-ltrans -msse4 -mssse3 -msse3 -msse2 -msse -mfpmath=both -pipe -fno-plt -static-libgcc -fvisibility=hidden"
cd godot-cpp
scons platform=linux target=release COMMON_FLAGS="$FLAGS"
cd ..
scons platform=linux target=release COMMON_FLAGS="$FLAGS"