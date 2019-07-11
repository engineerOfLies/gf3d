#ifndef __GF3D_VQUEUES_H__
#define __GF3D_VQUEUES_H__

#include <vulkan/vulkan.h>

#include "gfc_types.h"

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

/**
 * @brief get the queue family index for the presentation queue
 * @return the index of the presentation queue family
 */
Sint32 gf3d_vqueues_get_present_queue_family();

/**
 * @brief get the queue family index for the graphics queue
 * @return the index of the graphics queue family
 */
Sint32 gf3d_vqueues_get_graphics_queue_family();

/**
 * @brief get the queue family index for the graphics queue
 * @return the index of the graphics queue family
 */
Sint32 gf3d_vqueues_get_transfer_queue_family();

/**
 * @brief get the queue to be used for graphics calls
 * @returns the queue in question
 */
VkQueue gf3d_vqueues_get_graphics_queue();

/**
 * @brief get the queue to be used for presentation calls
 * @returns the queue in question
 */
VkQueue gf3d_vqueues_get_present_queue();

/**
 * @brief get the queue to be used for transfer calls
 * @returns the queue in question
 */
VkQueue gf3d_vqueues_get_transfer_queue();

#endif
