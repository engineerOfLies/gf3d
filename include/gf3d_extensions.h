#ifndef __GF3D_EXTENSIONS_H__
#define __GF3D_EXTENSIONS_H__

#include "gf3d_types.h"

/**
 * @brief initialize gf3d vulkan extension system
 */
void gf3d_extensions_init();

/**
 * @brief during setup phase, this will queue an extension to be enabled
 */
Bool gf3d_extensions_enable(const char *extensionName);

/**
 * @brief get the names of extensions to support and the count
 * @param count the number of extensions marked to be enabled
 * @returns an array of pointers to the names of the extensions to be enabled.
 * @note: Do not free this array, its managed internally to gf3d_extensions.
 */
const char* const* gf3d_extensions_get_enabled_names(Uint32 *count);


#endif
