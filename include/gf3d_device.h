#ifndef __GF3D_DEVICE_H__
#define __GF3D_DEVICE_H__

#include <vulkan/vulkan.h>

/**
 * @brief initialize the internal manager for vulkan devices
 * @param config path to a json formatted config file containing device information for prioritizing which device to use
 * @param instance the vulkan instance to work with
 */
void gf3d_device_manager_init(const char *config, VkInstance instance);

/**
 * @brief get the physical device that best matches the configured priorities for devices
 * @return VK_NULL_HANDLE if uninitialized or no device set or the physical device handle
 */
VkPhysicalDevice gf3d_devices_get_best_device();

/**
 * @brief get the physical device handle by name
 * @param name the name of the device as it appears to vulkan
 * @return VK_NULL_HANDLE if no matches or the physical device handle in question
 */
VkPhysicalDevice gf3d_devices_get_best_device_by_name(const char *name);


#endif
