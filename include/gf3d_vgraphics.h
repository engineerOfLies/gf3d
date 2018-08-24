#ifndef __GF3D_VGRAPHICS_H__
#define __GF3D_VGRAPHICS_H__

#include <vulkan/vulkan.h>

#include "gf3d_vector.h"

void gf3d_vgraphics_init(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    Bool enableValidation
);

VkDevice gf3d_vgraphics_get_default_logical_device();
VkExtent2D gf3d_vgraphics_get_view_extent();

void gf3d_vgraphics_clear();

void gf3d_vgraphics_render();


#endif
