#include "gf3d_commands.h"
#include "gf3d_vqueues.h"
#include "gf3d_swapchain.h"
#include "simple_logger.h"

#include <string.h>

typedef struct
{
    VkCommandPool       commandPool;
    VkCommandBuffer    *commandBuffers;
    Uint32              commandBufferCount;
    VkDevice            device;
}Commands;

static Commands gf3d_commands = {0};

void gf3d_command_pool_close();
void gf3d_command_buffer_begin(Pipeline *pipe);

void gf3d_command_pool_setup(VkDevice device,Uint32 count,Pipeline *pipe)
{
    VkCommandPoolCreateInfo poolInfo = {0};
    VkCommandBufferAllocateInfo allocInfo = {0};
    
    gf3d_commands.device = device;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = gf3d_vqueues_get_graphics_queue_family();
    poolInfo.flags = 0; // Optional    
    
    if (vkCreateCommandPool(device, &poolInfo, NULL, &gf3d_commands.commandPool) != VK_SUCCESS)
    {
        slog("failed to create command pool!");
        return;
    }
    
    gf3d_commands.commandBuffers = (VkCommandBuffer*)gf3d_allocate_array(sizeof(VkCommandBuffer),count);
    if (!gf3d_commands.commandBuffers)
    {
        slog("failed to allocate command buffer array");
        gf3d_command_pool_close();
        return;
    }
    
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = gf3d_commands.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;
    gf3d_commands.commandBufferCount = count;

    if (vkAllocateCommandBuffers(device, &allocInfo, gf3d_commands.commandBuffers) != VK_SUCCESS)
    {
        slog("failed to allocate command buffers!");
        gf3d_command_pool_close();
        return;
    }
    
    gf3d_command_buffer_begin(pipe);
    
    slog("created command buffers");
    atexit(gf3d_command_pool_close);
}

void gf3d_command_pool_close()
{
    if (gf3d_commands.commandBuffers)
    {
        free(gf3d_commands.commandBuffers);
    }
    vkDestroyCommandPool(gf3d_commands.device, gf3d_commands.commandPool, NULL);
    memset(&gf3d_commands,0,sizeof(Commands));
}

void gf3d_command_execute_render_pass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,VkFramebuffer framebuffer,VkPipeline graphicsPipeline)
{
    VkClearValue clearColor = {0};
    VkRenderPassBeginInfo renderPassInfo = {0};
    VkCommandBufferBeginInfo beginInfo = {0};
    
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = NULL; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        slog("failed to begin recording command buffer!");
    }
    
    clearColor.color.float32[3] = 1.0;
    
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = gf3d_swapchain_get_extent();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    //vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
    //instanceCount: Used for instanced rendering, use 1 if you're not doing that.
    //firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
    //firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        slog("failed to record command buffer!");
    }
    else
    {
        slog("created renderpass command");
    }
}

void gf3d_command_buffer_begin(Pipeline *pipe)
{
    size_t i = 0;
    for (i = 0; i < gf3d_commands.commandBufferCount; i++)
    {
        gf3d_command_execute_render_pass(
            gf3d_commands.commandBuffers[i], 
            pipe->renderPass,
            gf3d_swapchain_get_frame_buffer_by_index(i),
            pipe->graphicsPipeline);
    }
}

VkCommandBuffer * gf3d_command_buffer_get_by_index(Uint32 index)
{
    if (index >= gf3d_commands.commandBufferCount)
    {
        slog("FATAL: request for command buffer %i exceeds count",index);
        return NULL;
    }
    return &gf3d_commands.commandBuffers[index];
}
/*eol@eof*/
