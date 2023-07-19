#!/bin/sh
FLAGS="-Os -Wl,-Os -Wl,--gc-sections -ffunction-sections -fno-stack-protector -U_FORTIFY_SOURCE -flto -fipa-pta -fdevirtualize-at-ltrans -march=znver1 -mtune=znver1 -mfma -mavx2 -mavx -msse4a -msse4 -mssse3 -msse3 -msse2 -msse -mfpmath=both -pipe -fno-plt -fvisibility=hidden"
cd godot-cpp
scons platform=linux target=release COMMON_FLAGS="$FLAGS" $*
cd ..
scons platform=linux target=release COMMON_FLAGS="$FLAGS" $*