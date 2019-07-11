#ifndef __GF3D_COMMANDS_H__
#define __GF3D_COMMANDS_H__

#include <vulkan/vulkan.h>
#include "gfc_types.h"

#include "gf3d_pipeline.h"

typedef struct
{
    Uint8               _inuse;
    VkCommandPool       commandPool;
    VkCommandBuffer    *commandBuffers;
    Uint32              commandBufferCount;
    Uint32              commandBufferNext;
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

VkCommandBuffer gf3d_command_begin_single_time(Command *com);

void gf3d_command_end_single_time(Command *com, VkCommandBuffer commandBuffer);

Uint32 gf3d_command_pool_get_used_buffer_count(Command *com);

VkCommandBuffer * gf3d_command_pool_get_used_buffers(Command *com);

/**
 * @brief begin recording a command that will take rendering pass information.  Submit all draw commands between this and gf3d_command_rendering_end
 * @param index the rendering frame to use
 * @return the command buffer used for this drawing pass.
 */
VkCommandBuffer gf3d_command_rendering_begin(Uint32 index);

void gf3d_command_rendering_end(VkCommandBuffer commandBuffer);

void gf3d_command_configure_render_pass_end(VkCommandBuffer commandBuffer);


#endif

