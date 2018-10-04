#ifndef __GF3D_COMMANDS_H__
#define __GF3D_COMMANDS_H__

#include <vulkan/vulkan.h>

#include "gf3d_types.h"
#include "gf3d_pipeline.h"

typedef struct
{
    Uint8               _inuse;
    VkCommandPool       commandPool;
    VkCommandBuffer    *commandBuffers;
    Uint32              commandBufferCount;
}Command;

/**
 * @brief initialize the command pool subsystem
 * @param max_commands how many different command pools you want to support
 * @param device the default logical device to create the command pool for
 */
void gf3d_command_system_init(Uint32 max_commands,VkDevice defaultDevice);

/**
 * @brief setup up the command pool for graphics commands
 * @param count the number of command buffers to create
 * @param pipe the pointer to the graphics pipeline to use
 * @return NULL on error or a pointer to a setup command pool
 */
Command * gf3d_command_graphics_pool_setup(Uint32 count,Pipeline *pipe);

/**
 * @brief setup up the command pool for memory transfer commands
 * @param count the number of command buffers to create
 * @param pipe the pointer to the command pipeline to use
 * @return NULL on error or a pointer to a setup command pool
 */
Command * gf3d_command_transfer_pool_setup(Uint32 count,Pipeline *pipe);


/**
 * @brief execute a render pass
 */
void gf3d_command_execute_render_pass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,VkFramebuffer framebuffer,VkPipeline graphicsPipeline);

/**
 * @brief get a command buffer by index
 * @param com the command pool to get the buffer from
 * @param index the index to retrieve
 * @returns NULL on error, or a pointer to the command buffer requested
 */
VkCommandBuffer * gf3d_command_buffer_get_by_index(Command *com,Uint32 index);

#endif

