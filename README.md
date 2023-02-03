# gf3d
a collection of utlitity functions designed to facilitate creating 3D games with Vulkan and SDL2
This project is specifically intended to function as an educational tool for my students taking 3D Game Programming.

This branch, MASTER, is designed to have all of the most advanced features that have been developed.  It is compiled as a library in the /libs folder and can be linked statically or dynamically.  This branch is more complex and not intended for beginners just learning.


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

