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

/**
 * @brief get queue information needed to create a logical device based on discovered queue properties
 * @returns the VkDeviceQueueCreateInfo populated
 */
VkDeviceQueueCreateInfo gf3d_vqueues_get_graphics_queue_info();

/**
 * @brief after setting up the logical device, store the queue handle to use for graphics commands with this
 * @param graphicsQueue the queue handle to use for graphics calls
 */
void gf3d_vqueues_set_graphics_queue(VkQueue graphicsQueue);


#endif
