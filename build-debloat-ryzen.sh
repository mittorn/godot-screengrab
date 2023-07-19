#!/bin/sh
FLAGS="-g -Os -Wl,-Os -Wl,--gc-sections -ffunction-sections -fno-stack-protector -U_FORTIFY_SOURCE -flto -fipa-pta -fdevirtualize-at-ltrans -march=znver1 -mtune=znver1 -mfma -mavx2 -mavx -msse4a -msse4 -mssse3 -msse3 -msse2 -msse -mfpmath=both -pipe -fno-plt -fvisibility=hidden"
cd godot-cpp
scons platform=linux target=release COMMON_FLAGS="$FLAGS" $*
cd ..
scons platform=linux target=release COMMON_FLAGS="$FLAGS" $*
objcopy --only-keep-debug project/gdnative/linux/libscreengrab.so project/gdnative/linux/libscreengrab.so.debug
strip -s project/gdnative/linux/libscreengrab.so
cd project/gdnative/linux/
objcopy --add-gnu-debuglink=libscreengrab.so.debug libscreengrab.so
cd ../../..
gcc drmsend.c -o drmsend `pkg-config --cflags --libs libdrm` -g
objcopy --only-keep-debug drmsend drmsend.debug
strip -s drmsend
objcopy --add-gnu-debuglink=drmsend.debug drmsend