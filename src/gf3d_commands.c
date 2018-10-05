#include "gf3d_commands.h"
#include "gf3d_vgraphics.h"
#include "gf3d_vqueues.h"
#include "gf3d_swapchain.h"
#include "gf3d_mesh.h"
#include "simple_logger.h"

#include <string.h>


 // TODO: Make a command buffer resource manager

typedef struct
{
    Command     *   command_list;
    Uint32          max_commands;
    VkDevice        device;
}CommandManager;

extern Mesh *testMesh;

static CommandManager gf3d_commands = {0};

void gf3d_command_pool_close();
void gf3d_command_free(Command *com);
void gf3d_command_buffer_begin(Command *com,Pipeline *pipe);

void gf3d_command_system_close()
{
    int i;
    if (gf3d_commands.command_list != NULL)
    {
        for (i = 0; i < gf3d_commands.max_commands; i++)
        {
            gf3d_command_free(&gf3d_commands.command_list[i]);
        }
        free(gf3d_commands.command_list);
    }
    slog("command pool system closed");
}

void gf3d_command_system_init(Uint32 max_commands,VkDevice defaultDevice)
{
    slog("command pool system init");
    if (!max_commands)
    {
        slog("cannot initliaze 0 command pools");
        return;
    }
    gf3d_commands.device = defaultDevice;
    gf3d_commands.max_commands = max_commands;
    gf3d_commands.command_list = gf3d_allocate_array(sizeof(CommandManager),max_commands);
    
    atexit(gf3d_command_system_close);
}

Command *gf3d_command_new()
{
    int i;
    for (i = 0; i < gf3d_commands.max_commands;i++)
    {
        if (!gf3d_commands.command_list[i]._inuse)
        {
            gf3d_commands.command_list[i]._inuse = 1;
            return &gf3d_commands.command_list[i];
        }
    }
    slog("failed to get a new command pool, list full");
    return NULL;
}

void gf3d_command_free(Command *com)
{
    if ((!com)||(!com->_inuse))return;
    if (com->commandBuffers)
    {
        free(com->commandBuffers);
    }
    vkDestroyCommandPool(gf3d_commands.device, com->commandPool, NULL);
    memset(com,0,sizeof(Command));
}

Command * gf3d_command_transfer_pool_setup(Uint32 count,Pipeline *pipe)
{
    Command *com;
    VkCommandPoolCreateInfo poolInfo = {0};
    VkCommandBufferAllocateInfo allocInfo = {0};
    
    com = gf3d_command_new();
    
    if (!com)
    {
        return NULL;
    }
    
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = gf3d_vqueues_get_transfer_queue_family();
    poolInfo.flags = 0; // Optional    
    
    if (vkCreateCommandPool(gf3d_commands.device, &poolInfo, NULL, &com->commandPool) != VK_SUCCESS)
    {
        slog("failed to create command pool!");
        return NULL;
    }
    
    com->commandBuffers = (VkCommandBuffer*)gf3d_allocate_array(sizeof(VkCommandBuffer),count);
    if (!com->commandBuffers)
    {
        slog("failed to allocate command buffer array");
        gf3d_command_free(com);
        return NULL;
    }
    
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = com->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;
    com->commandBufferCount = count;

    if (vkAllocateCommandBuffers(gf3d_commands.device, &allocInfo, com->commandBuffers) != VK_SUCCESS)
    {
        slog("failed to allocate command buffers!");
        gf3d_command_free(com);
        return NULL;
    }
    
    gf3d_command_buffer_begin(com,pipe);
    
    slog("created command buffer");
    return com;
}

Command * gf3d_command_graphics_pool_setup(Uint32 count,Pipeline *pipe)
{
    Command *com;
    VkCommandPoolCreateInfo poolInfo = {0};
    VkCommandBufferAllocateInfo allocInfo = {0};
    
    com = gf3d_command_new();
    
    if (!com)
    {
        return NULL;
    }
    
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = gf3d_vqueues_get_graphics_queue_family();
    poolInfo.flags = 0; // Optional    
    
    if (vkCreateCommandPool(gf3d_commands.device, &poolInfo, NULL, &com->commandPool) != VK_SUCCESS)
    {
        slog("failed to create command pool!");
        return NULL;
    }
    
    com->commandBuffers = (VkCommandBuffer*)gf3d_allocate_array(sizeof(VkCommandBuffer),count);
    if (!com->commandBuffers)
    {
        slog("failed to allocate command buffer array");
        gf3d_command_free(com);
        return NULL;
    }
    
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = com->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;
    com->commandBufferCount = count;

    if (vkAllocateCommandBuffers(gf3d_commands.device, &allocInfo, com->commandBuffers) != VK_SUCCESS)
    {
        slog("failed to allocate command buffers!");
        gf3d_command_free(com);
        return NULL;
    }
    
    gf3d_command_buffer_begin(com,pipe);
    
    slog("created command buffer");
    return com;
}


void gf3d_command_execute_render_pass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,VkFramebuffer framebuffer,VkPipeline graphicsPipeline,VkPipelineLayout pipelineLayout, VkDescriptorSet *descriptorSet)
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

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSet, 0, NULL);
    gf3d_mesh_render(testMesh,commandBuffer);
    slog("doing render pass");
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

void gf3d_command_buffer_begin(Command *com,Pipeline *pipe)
{
    size_t i = 0;
    if (!com)return;
    for (i = 0; i < com->commandBufferCount; i++)
    {
        gf3d_command_execute_render_pass(
            com->commandBuffers[i], 
            pipe->renderPass,
            gf3d_swapchain_get_frame_buffer_by_index(i),
            pipe->graphicsPipeline,
            pipe->pipelineLayout,
            gf3d_vgraphics_get_descriptor_set_by_index(i));
    }
}

VkCommandBuffer * gf3d_command_buffer_get_by_index(Command *com,Uint32 index)
{
    if (!com)return NULL;
    if (index >= com->commandBufferCount)
    {
        slog("FATAL: request for command buffer %i exceeds count",index);
        return NULL;
    }
    return &com->commandBuffers[index];
}
/*eol@eof*/
