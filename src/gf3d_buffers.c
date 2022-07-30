#include "simple_logger.h"

#include "gf3d_vgraphics.h"
#include "gf3d_buffers.h"

void gf3d_buffer_copy(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkBufferCopy copyRegion = {0};

    VkCommandBuffer commandBuffer = gf3d_command_begin_single_time(gf3d_vgraphics_get_graphics_command_pool());
    
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    gf3d_command_end_single_time(gf3d_vgraphics_get_graphics_command_pool(), commandBuffer);
    
}

int gf3d_buffer_create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {0};
    VkMemoryRequirements memRequirements;
    VkMemoryAllocateInfo allocInfo = {0};

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(gf3d_vgraphics_get_default_logical_device(), &bufferInfo, NULL, buffer) != VK_SUCCESS)
    {
        slog("failed to create buffer!");
        return 0;
    }

    vkGetBufferMemoryRequirements(gf3d_vgraphics_get_default_logical_device(), *buffer, &memRequirements);

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = gf3d_vgraphics_find_memory_type(memRequirements.memoryTypeBits, properties);

    
    if (vkAllocateMemory(gf3d_vgraphics_get_default_logical_device(), &allocInfo, NULL, bufferMemory) != VK_SUCCESS)
    {
        slog("failed to allocate buffer memory!");
        return 0;
    }

    vkBindBufferMemory(gf3d_vgraphics_get_default_logical_device(), *buffer, *bufferMemory, 0);
    return 1;
}

/*eol@eof*/
