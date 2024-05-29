#ifndef __GF3D_VGRAPHICS_H__
#define __GF3D_VGRAPHICS_H__

#include <vulkan/vulkan.h>
#include "gfc_vector.h"
#include "gfc_matrix.h"

#include "gf3d_pipeline.h"
#include "gf3d_commands.h"

#define GF3D_VGRAPHICS_DISCRETE 1
//Choosing whether to use discrete [1] or integrated graphics [0]
//NOTE: make this configurable

typedef struct
{
    GFC_Matrix4 model;
    GFC_Matrix4 view;
    GFC_Matrix4 proj;
}ModelViewProjection;

/**
 * @brief init Vulkan / SDL, setup device and initialize infrastructure for 3d graphics
 * @param config json file containing setup information
 */
void gf3d_vgraphics_init(const char *config);

/**
 * @brief kick off a rendering call for the next buffer frame.
 */
void gf3d_vgraphics_render_start();

/**
 * @brief finish a render command and send all commands to the GPU
 */
void gf3d_vgraphics_render_end();

/**
 * @brief get the buffer frame for the current rendering context
 * @note: THIS SHOULD ONLY BE CALLED BETWEEN CALLS TO gf3d_vgraphics_render_start() and gf3d_vgraphics_render_end()
 * @param the active buffer frame (swap chain link number)
 */
Uint32  gf3d_vgraphics_get_current_buffer_frame();

/**
 * @brief After initialization 
 */
VkDevice gf3d_vgraphics_get_default_logical_device();

/**
 * @brief get the physical device handle for the graphics system
 */
VkPhysicalDevice gf3d_vgraphics_get_default_physical_device();

/**
 * @brief get the screen resolution as a Vulkan Extent
 */
VkExtent2D gf3d_vgraphics_get_view_extent();

/**
 * @brief get the screen extent as a 2d gfc_vector
 */
GFC_Vector2D gf3d_vgraphics_get_view_extent_as_vector2d();

/**
 * @brief get the screen extent as a 2d gfc_vector
 */
GFC_Vector2D gf3d_vgraphics_get_resolution();

/**
 * @brief translate a 3D position to the corresponding screen position
 * @note this does the same operations that are better handled in shaders, so it sparingly
 * @param position the position in 3D space input
 * @return the same position as it maps to the screen.
 */
GFC_Vector2D vgraphics_3d_position_to_screen(GFC_Vector3D position);

/**
 * @brief translate a 3D position to the corresponding screen position
 * @note this does the same operations that are better handled in shaders, so it sparingly
 * @param position the position in 3D space input
 * @return the same position as it maps to the screen with the z component being the screen depth
 */
GFC_Vector3D vgraphics_3d_position_to_screen_depth(GFC_Vector3D position);

/**
 * @brief copy into view the current view matrix
 * @param view [output]
 */
void gf3d_vgraphics_get_view(GFC_Matrix4 *view);


void gf3d_vgraphics_clear();

/**
 * @brief check if the device supports memory of the given types
 * @param typefilter a search critera
 * @param properties more search critera
 * @return 0 if there is no matching memory type supported, the memory type index otherwise
 */
uint32_t gf3d_vgraphics_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void gf3d_vgraphics_rotate_camera(float degrees);

/**
 * @brief get the matrix used for rendering the view
 * @return the view matrix sent to every rendering call
 */
GFC_Matrix4 *gf3d_vgraphics_get_view_matrix();

/**
 * @brief get the projection matrix
 * @param proj where to put the projection matrix
 */
void gf3d_vgraphics_get_projection_matrix(GFC_Matrix4 *proj);


/**
 * @brief get the vulkan handle for a uniform buffer by its index
 */
VkBuffer gf3d_vgraphics_get_uniform_buffer_by_index(Uint32 index);

/**
 * @brief get the current MVP matrix for the render calls.
 */
ModelViewProjection gf3d_vgraphics_get_mvp();

/**
 * @brief get the pipeline that is used to render 2d images to the overlay
 * @return NULL on error or the pipeline in question
 */
Pipeline *gf3d_vgraphics_get_graphics_overlay_pipeline();

/**
 * @brief get a command from the graphics command pool
 * @return NULL if non are left, or an empty command
 */
Command *gf3d_vgraphics_get_graphics_command_pool();

/**
 * @brief create an image view in the given vulkan format
 * @param image the image to create the view for
 * @param format the format to create the view for
 * @return VkNullHandle on error or an imageview handle otherwise
 */
VkImageView gf3d_vgraphics_create_image_view(VkImage image, VkFormat format);

/**
 * @brief create an empty SDL_Surface in the format supported by the screen
 * @param w the width to create, should be non-zero
 * @param h the hight to create, should be non-zero
 * @return NULL on error, or an empty SDL_Surface in the proper format
 */
SDL_Surface *gf3d_vgraphics_create_surface(Uint32 w,Uint32 h);

/**
 * @brief convert a SDL_Surface to the format supported by the system
 * @param surface a pointer to the SDL_Surface pointer that contains the image data to convert
 * @return NULL on failure, or a new SDL surface of the same image, but in the supported format.
 * @note this will clear the data of the original surface if it is successful automatically.
 */
SDL_Surface *gf3d_vgraphics_screen_convert(SDL_Surface **surface);

#endif
