#ifndef __GF3D_TEXTURE_H__
#define __GF3D_TEXTURE_H__

#include <vulkan/vulkan.h>
#include "gf3d_types.h"
#include "gf3d_text.h"

typedef struct
{
    Uint8               _inuse;
    Uint32              _refcount;
    TextLine            filename;
    VkImage             textureImage;
    VkDeviceMemory      textureImageMemory;
    VkImageView         textureImageView;
}Texture;


void gf3d_texture_init(Uint32 max_textures);
Texture *gf3d_texture_load(char *filename);

#endif
