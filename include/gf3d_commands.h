#ifndef __GF3D_COMMANDS_H__
#define __GF3D_COMMANDS_H__

#include <vulkan/vulkan.h>

#include "gf3d_types.h"
#include "gf3d_pipeline.h"

/**
 * @brief setup up the command pools
 * @param device the logical device to use for this
 * @param count the swap chain count
 * @param pipe the pointer to the graphics pipeline to use
 */
void gf3d_command_pool_setup(VkDevice device,Uint32 count,Pipeline *pipe);

/**
 * @brief execute a render pass
 */
void gf3d_command_execute_render_pass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,VkFramebuffer framebuffer,VkPipeline graphicsPipeline);

/**
 * @brief get a command buffer by index
 * @returns NULL on error, or a pointer to the command buffer requested
 */
VkCommandBuffer *gf3d_command_buffer_get_by_index(Uint32 index);

#endif
