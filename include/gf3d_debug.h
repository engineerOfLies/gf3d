#ifndef __GF3D_DEBUG_H__
#define __GF3D_DEBUG_H__

#include <vulkan/vulkan.h>

/**
 * @brief call to setup the debug watcher thread
 * @param instance the vulkan instance to watch
 */
void gf3d_debug_setup(VkInstance instance);

/**
 * @brief call to close the thread at program exit
 */
void gf3d_debug_close();

#endif
