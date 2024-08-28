#ifndef __GF3D_TEXTURE_H__
#define __GF3D_TEXTURE_H__

#include <vulkan/vulkan.h>
#include "gfc_types.h"
#include "gfc_text.h"

typedef struct
{
    Uint8               _inuse;
    Uint32              _refcount;
    Uint32              width,height;
    GFC_TextLine        filename;
    VkImage             textureImage;
    VkDeviceMemory      textureImageMemory;
    VkImageView         textureImageView;
    VkSampler           textureSampler;
    SDL_Surface        *surface;    /**<the image data in CPU space*/
}Texture;

/**
 * @brief initialize the texture subsystem
 * @param max_textures the maximum number of concurrent textures to be supported.
 * This is inclusive of all model textures and sprites
 */
void gf3d_texture_init(Uint32 max_textures);

/**
 * @brief load a texture from file.
 * @param filename the path to the file to load
 * @return NULL on error or the texture loaded
 */
Texture *gf3d_texture_load(const char *filename);

/**
 * @brief create a texture based on the provided surface.
 * @note the filename is not populated by this
 * @param surface the SDL_Surface image data to convert
 * @return NULL on error or a new Texture otherwise
 */
Texture *gf3d_texture_convert_surface(SDL_Surface * surface);

/**
* @brief free a previously loaded texture
 */
void gf3d_texture_free(Texture *tex);

#endif
