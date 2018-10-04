#ifndef __GF3D_VGRAPHICS_H__
#define __GF3D_VGRAPHICS_H__

#include <vulkan/vulkan.h>

#include "gf3d_vector.h"

#define GF3D_VGRAPHICS_DISCRETE 1   //Choosing whether to use discrete [1] or integrated graphics [0]

void gf3d_vgraphics_init(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    Bool enableValidation
);

VkDevice gf3d_vgraphics_get_default_logical_device();
VkPhysicalDevice gf3d_vgraphics_get_default_physical_device();

VkExtent2D gf3d_vgraphics_get_view_extent();

void gf3d_vgraphics_clear();

void gf3d_vgraphics_render();

VkDescriptorSetLayout * gf3d_vgraphics_get_descriptor_set_layout();

int gf3d_vgraphics_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * bufferMemory);

uint32_t gf3d_vgraphics_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);


#endif
