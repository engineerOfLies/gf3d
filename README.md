# gf3d
a collection of utlitity functions designed to facilitate creating 3D games with Vulkan and SDL2
This project is specifically intended to function as an educational tool for anyone looking to learn 3D Game Programming with Vulkan and to challenge myself to learn the underlying systems necessary for a 3D Game Engine.

# Linux Build Process

Before you can build the example code we are providing for you, you will need to obtain the libraries required
by the source code. Some of these libraries are easy to obtain and others are provided by the vendors of hardware
that is installed in your computer. After the libraries are obtained the following sequence of steps should be
performable from the following steps from the root of the cloned git repository within a terminal. 

external dependencies:
1. vulkan (get the Lunar SDK for windows, or the libvulkan-dev library for linux)
2. SDL2 (get from libsdl.org or from your linux repo)
3. SDL2_image
4. SDL2_ttf
5. SDL2_mixer

build steps:
1. Obtain the code: `git clone <repo name>`
2. Checkout seed branch: `git checkout <branch name>`
3. Make sure ou fetch submodules: `git submodule update --init --recursive`
4. Build libraries: `pushd gfc/src; make; popd`
5. Build this library: `pushd src; make; make static; popd`

You should now have a `libgf3d.a` static library and `libgf3d.so.1` dynamic library in the libs/ folder 

# directories
## actors/
sample files for making actors (files that describe how a sprite should be handled)
## config/
sample config files for the various subsystems of the gf3d library
## gfc/
a submodule, like gf3d, meant to be core files common to both 2d and 3d games
## menus/
sample menu definition files used by the gf2d_window system
## shaders/
sample shaders in glsl and spir-v

## test/
a test game.c and makefile showing how you can use the library / link against it (it least in linux)

# Window Build Process - Visual Studio
You will need to download the development libraries for Vulkan, SDL2, SDL2_image, SDL2_mixer, and SDL2_ttf.  I recommend extracting them to a libs folder in a folder alongside your project (so they can be re-used with other projects).  

Make a new empty c/c++ project
Set the runtime path and output directory to be the root of your project.  Remember that all file access is relative to the root of the project.

Add additional include directories to be your project's include, gf3d/include, gf3d/gfc/include, gf3d/gfc/simple_json/incude, gf3d/gfc/simple_logger/include as well as all the include directories within the SDL2 libraries and vulkan

Now add addition library directories to be the lib folders within sdl2 and vulkan.  be sure to match the x86/32 for 32 bit projects or x64 for 64 bit projects.

Add the .lib files for each of the SDL2 libraries to the additional libraries section under linker section.  sdl2main.lib, sdl2.lib, sdl2_image.lib,sdl2_mixer.lib, sdl2_ttf.lib and the vulkan-1.lib 

Add existing items: all the .c and .h files from each of the submodules gf3d,gfc,simple_logger, simple_json 

If this is your first project you can try using the sample in the test folder.

