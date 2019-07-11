#ifndef __GF3D_SHADERS_H__
#define __GF3D_SHADERS_H__

#include <vulkan/vulkan.h>

#include "gfc_types.h"

/**
 * @brief load a SPIR-V shader program from file
 * @param filename the name of the file to load
 * @param rsize [output] saves the size, in bytes, of the buffer loaded
 * @returns NULL on error (check logs) or a pointer to the buffer containing the shader.
 * @note: The buffer returned must be freed, it is not managed internally
 */
char *gf3d_shaders_load_data(char * filename,size_t *rsize);

/**
 * @brief create shader module from a loaded SPIR-V buffer
 * @param shader the buffer containing shader byte code
 * @param size the mount of bites contained in the shader
 * @param device the logical device to create this for
 */
VkShaderModule gf3d_shaders_create_module(char *shader,size_t size,VkDevice device);


#endif
