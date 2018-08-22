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
 * @brief get graphics queue information needed to create a logical device based on discovered queue properties
 * @param count a pointer to an integer to populate the count of create info returned
 * @returns a pointer to an array of VkDeviceQueueCreateInfo populated
 */
const VkDeviceQueueCreateInfo *gf3d_vqueues_get_queue_create_info(Uint32 *count);

/**
 * @brief after device creation call this to setup internal queue handles
 * @param device the logical device handle
 */
void gf3d_vqueues_setup_device_queues(VkDevice device);


#endif
