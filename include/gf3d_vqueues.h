#ifndef __GF3D_VQUEUES_H__
#define __GF3D_VQUEUES_H__

#include <vulkan/vulkan.h>

#include "gf3d_types.h"

/**
 * @brief initialize the vulkan queues
 * @param device the device to use for setup
 * @param surface the vulkan surface to check for compatibility
 */
void gf3d_vqueues_init(VkPhysicalDevice device,VkSurfaceKHR surface);


#endif
