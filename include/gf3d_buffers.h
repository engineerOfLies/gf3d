#ifndef __GF3D_BUFFERS_H__
#define __GF3D_BUFFERS_H__

#include <vulkan/vulkan.h>

/**
 * @brief copy from one buffer to another
 * @param scrBuffer the buffer to copy from
 * @param dstBuffer the buffer to copy to
 * @param size how much to copy
 */
void gf3d_buffer_copy(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

/**
 * @brief create and allocate the memory for a buffer
 * @param size how much memory to create
 * @param usage usage flags
 * @param properties memory properties
 * @param buffer (output) will be set with the handle to the buffer
 * @param bufferMemory (output) will be set with the handle to the bufferMemory
 * @return 1 on success, 0 on failure
 */
int gf3d_buffer_create(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer * buffer,
    VkDeviceMemory * bufferMemory);

#endif
