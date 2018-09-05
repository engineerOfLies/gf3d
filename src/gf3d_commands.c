#include "gf3d_commands.h"
#include "gf3d_vqueues.h"
#include "simple_logger.h"

typedef struct
{
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
    VkDevice device;
}Commands;

static Commands gf3d_commands = {0};

void gf3d_command_pool_close();

void gf3d_command_pool_setup(VkDevice device,Uint32 count)
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

    if (vkAllocateCommandBuffers(device, &allocInfo, gf3d_commands.commandBuffers) != VK_SUCCESS)
    {
        slog("failed to allocate command buffers!");
        gf3d_command_pool_close();
        return;
    }
    
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

/*eol@eof*/
