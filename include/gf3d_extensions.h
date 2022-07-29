#ifndef __GF3D_EXTENSIONS_H__
#define __GF3D_EXTENSIONS_H__

#include <vulkan/vulkan.h>
#include "gfc_types.h"

typedef enum
{
    ET_Instance,
    ET_Device
}ExtensionType;

/**
 * @brief initialize gf3d vulkan extension system
 * @param config a json file containing a list instance_extensions to enable
 */
void gf3d_extensions_instance_init(const char *config);

/**
 * @brief during setup phase, this will queue an extension to be enabled
 * @param extType the type of extension to enable
 * @param extensionName the name of the extension to enable, spelling counts!
 */
Bool gf3d_extensions_enable(ExtensionType extType, const char *extensionName);

/**
 * @brief get the names of instance extensions to support and the count
 * @param count the number of extensions marked to be enabled
 * @returns an array of pointers to the names of the extensions to be enabled.
 * @note: Do not free this array, its managed internally to gf3d_extensions.
 */
const char* const* gf3d_extensions_get_instance_enabled_names(Uint32 *count);

/**
 * @brief get the names of device extensions to support and the count
 * @param count the number of extensions marked to be enabled
 * @returns an array of pointers to the names of the extensions to be enabled.
 * @note: Do not free this array, its managed internally to gf3d_extensions.
 */
const char* const* gf3d_extensions_get_device_enabled_names(Uint32 *count);

/**
 * @brief after device creation, setup vulkan extension support
 * @param device the physical device to setup extensions for
 * @param config a json file containing a list device_extensions to enable
 */
void gf3d_extensions_device_init(VkPhysicalDevice device,const char *config);


const char* const* gf3d_extensions_get_instance_available_names(Uint32 *count);

#endif
