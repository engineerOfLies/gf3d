#ifndef __GF3D_GRAPHICS_C__
#define __GF3D_GRAPHICS_C__

#include "gf3d_types.h"
#include "gf3d_vector.h"
#include "gf3d_matrix.h"

/**
 * @brief initializes SDL and setups up basic window and rendering contexts
 * @param windowName the name that you would want displayed on the game window
 * @param viewWidth how wide you want your game window to be
 * @param viewHeight how high you want your game window to be
 * @param renderWidth How much draw width you want to work with logically
 * @param renderHeight How much draw height you want to work with logically
 * @param bgcolor what you want the default background color to be
 * @param fullscreen if you want the window to render full screen or not
 * @param major what major version of opengl you want to use
 * @param minor what minor version of opengl you want to use
 */
void gf3d_graphics_initialize(
    char *windowName,
    int viewWidth,
    int viewHeight,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    int major,
    int minor
);

/**
 * @brief get the loaded shader program
 * @returns 0 if no program is loaded or error, a valid program id otherwise
 */
GLuint gf3d_graphics_get_shader_program_id();

/**
 * @brief sets the amount of delay to aim for between frames.
 * @param frameDelay the amount of time, in milliseconds, that each frame should take
 */
void gf3d_graphics_set_frame_delay(Uint32 frameDelay);

/**
 * @brief gets the functional number of frames rendered per second
 * @return the current frame rate
 */
float gf3d_graphics_get_frames_per_second();

/**
 * @brief render the current frame to screen
 */
void gf3d_graphics_next_frame();

/**
 * @brief clears drawing buffer.  Should be called each frame before drawing
 */
void gf3d_graphics_clear_screen();

/**
 * @brief get the current project matrix
 * default is set based on a fov of 45, aspect ratio based on sceen resolution, near plane at 0.1 and far plane at 100.0
 * @param projection output the matrix pointed to by projection is set to the projection matrix
 */
void gf3d_graphics_get_projection(Matrix4 projection);

/**
 * @brief set a custom projection matrix
 * @param projection the new matrix to use for projections
 */
void gf3d_graphics_set_projection(Matrix4 projection);

#endif
