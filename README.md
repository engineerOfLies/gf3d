# gf3d
a collection of utlitity functions designed to facilitate creating 3D games with ~~OpenGL~~ Vulkan and SDL2
This project is specifically intended to function as an educational tool for my students taking 3D Game Programming.

Currently the project is in a WIP state.
This represents a few revisions of the vulkan code now.  It is still not exactly where I want it as a seed project for 3D Game Programming, but it is far more stable than it has been.

# Build Process

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

build process:
1. Obtain the code: `git clone <repo name>`
2. Checkout seed branch: `git checkout <branch name>`
3. Make sure ou fetch submodules: `git submodule update --init --recursive`
4. Build libraries: `pushd gfc/src; make; popd`
5. Build game: `pushd src; make; popd`

You should now have a `gf3d` binary within the root of your git repository. Executing this will start your game.
