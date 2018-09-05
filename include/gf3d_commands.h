#ifndef __GF3D_COMMANDS_H__
#define __GF3D_COMMANDS_H__

#include <vulkan/vulkan.h>

#include "gf3d_types.h"

/**
 * @brief setup up the command pools
 */
void gf3d_command_pool_setup(VkDevice device,Uint32 count);


#endif
