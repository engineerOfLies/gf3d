#ifndef __GF3D_VGRAPHICS_H__
#define __GF3D_VGRAPHICS_H__

#include <vulkan/vulkan.h>
#include "gfc_vector.h"
#include "gfc_matrix.h"

#include "gf3d_pipeline.h"
#include "gf3d_commands.h"

#define GF3D_VGRAPHICS_DISCRETE 1
//Choosing whether to use discrete [1] or integrated graphics [0]

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
 * @brief get the handle to the active command buffer for the current 3d model rendering context
 * @note: THIS SHOULD ONLY BE CALLED BETWEEN CALLS TO gf3d_vgraphics_render_start() and gf3d_vgraphics_render_end()
 * @return the handle to the command buffer.
 */
VkCommandBuffer gf3d_vgraphics_get_current_command_model_buffer();

/**
 * @brief get the handle to the active command buffer for the current 2d overlay rendering context
 * @note: THIS SHOULD ONLY BE CALLED BETWEEN CALLS TO gf3d_vgraphics_render_start() and gf3d_vgraphics_render_end()
 * @return the handle to the command buffer.
 */
VkCommandBuffer gf3d_vgraphics_get_current_command_overlay_buffer();


/**
 * @brief After initialization 
 */
VkDevice gf3d_vgraphics_get_default_logical_device();

VkPhysicalDevice gf3d_vgraphics_get_default_physical_device();

VkExtent2D gf3d_vgraphics_get_view_extent();

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
Matrix4 *gf3d_vgraphics_get_view_matrix();


VkBuffer gf3d_vgraphics_get_uniform_buffer_by_index(Uint32 index);
UniformBufferObject gf3d_vgraphics_get_uniform_buffer_object();

/**
 * @brief get the pipeline that is used to render 2d images to the overlay
 * @return NULL on error or the pipeline in question
 */
Pipeline *gf3d_vgraphics_get_graphics_overlay_pipeline();

Command *gf3d_vgraphics_get_graphics_command_pool();

VkImageView gf3d_vgraphics_create_image_view(VkImage image, VkFormat format);


#endif
