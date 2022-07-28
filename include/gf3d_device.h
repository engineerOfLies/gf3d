#ifndef __GF3D_DEVICE_H__
#define __GF3D_DEVICE_H__

#include <vulkan/vulkan.h>


typedef struct
{
    VkPhysicalDevice device;                        /**vulkan device handle*/
    VkPhysicalDeviceProperties  deviceProperties;   /**<properties of the device*/
    VkPhysicalDeviceFeatures    deviceFeatures;     /**<features of the device*/
    int score;                                      /**<how many device features match ideal*/
}GF3D_Device;

/**
 * @brief initialize the internal manager for vulkan devices
 * @param config path to a json formatted config file containing device information for prioritizing which device to use
 * @param instance the vulkan instance to work with
 * @param renderSurface the surface needed to render to with the device
 */
void gf3d_device_manager_init(const char *config, VkInstance instance, VkSurfaceKHR renderSurface);

/**
 * @brief get the logical device in use
 * @return VK_NULL_HANDLE if the device is not setup or other error.
 */
VkDevice gf3d_device_get();

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
VkPhysicalDevice gf3d_devices_get_device_by_name(const char *name);

/**
 * @brief get the GF3D_Device info for the chosen gpu (physical device) if set
 * @return NULL if it has not been chosen yet, or an issue arose while choosing, or the device info otherwise
 */
GF3D_Device *gf3d_device_get_chosen_gpu_info();

/**
 * @brief get the creation info needed to create a logical device based on what has been loaded and configured so far
 * @param enableValidationLayers if true, this will turn on validation layers. 
 * @return an empty (all zeros) logical device create info, or a configured one otherwise
 */
VkDeviceCreateInfo gf3d_device_get_logical_device_info(Bool enableValidationLayers);


#endif
