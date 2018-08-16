#ifndef __GF3D_VALIDATION_H__
#define __GF3D_VALIDATION_H__

#include <vulkan/vulkan.h>
#include <SDL.h>
/**
 * @brief Setup Vulkan Validation Layers
 */
void gf3d_validation_init();

/**
 * @brief get the array address for the validation layers
 * @note this data is managed by vulkan and should not be modified
 * @returns NULL if validation layers have not been initialized, or the address of the first validation layer property
 */
VkLayerProperties *gf3d_validation_get_validation_layer_data();

/**
 * @brief get the number of validation layers that are available
 * @return 0 if there are none, or validation has not been initialized.  The polled count otherwises
 */
Uint32 gf3d_validation_get_validation_layer_count();

/**
 * @brief get an array of pointers to the validation layer names
 * @return NULL on error,no layers, or if init has not been called yet
 */
const char* const* gf3d_validation_get_validation_layer_names();

#endif
