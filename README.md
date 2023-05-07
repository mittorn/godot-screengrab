# Godot screengrap addon

This is based on w23's OBS plugin for zero-copy screen capture, should work on drm-based single-display configurations
Use update_texture method on gogot's ExternalTexture

Language: [GDNative C++](https://docs.godotengine.org/en/latest/tutorials/scripting/gdnative/index.html)

Renderer: GLES 2

Dependencies:
 * You need [godot-cpp](https://github.com/godotengine/godot-cpp),
   this is now a Git submodule of this repo.
 * `clang`, `gcc`, or any decent C++ compiler that's C++14 compatible.

## Compiling

You can use SCons to compile the library:

```
scons platform=PLATFORM
```

Where PLATFORM is: `windows`, `linux`, or `osx`.

This creates the file `libscreengrab` in the respective
subfolders in the `project/gdnative` directory.
