#ifndef __GF3D_SPRITE_H__
#define __GF3D_SPRITE_H__

/**
 * gf3d_sprite
 * @license The MIT License (MIT)
   @copyright Copyright (c) 2019 EngineerOfLies
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "gfc_types.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_text.h"

#include "gf3d_texture.h"

typedef struct
{
    Uint8                       _inuse;
    TextLine                    filename;               /**<the name of the file used to create the sprite*/
    Uint32                      frameCount;             /**<how many frames are loaded for this model*/
    Texture                    *texture;                /**<texture memory pointer*/
    Uint8                       framesPerLine;          /**<how many frames are per line in the sprite sheet*/
    Uint32                      frameWidth,frameHeight; /*<the size, in pixels, of the individual sprite frames*/
    VkBuffer                    buffer;                 /**<vertex buffer for the sprite (always 4 vertices)*/
    VkDeviceMemory              bufferMemory;           /**<memory handle for the vertex buffer*/
    VkDescriptorSet            *descriptorSet;          /**<descriptor sets used for this sprite to render*/
    VkBuffer                   *uniformBuffers;         /**<handles for the UBO*/
    VkDeviceMemory             *uniformBuffersMemory;   /**<memory handle for the UBO memory*/
    Uint32                      uniformBufferCount;     /**<how many UBOs for the sprite*/
}Sprite;

/**
 * @brief initialize the internal management system for sprites, auto-cleaned up on program exit
 * @param max_sprites how many concurrent sprites to support
 * @param chain_length how many images are available in the swap chain
 * @param device the logical vulkan device to be rendering to
 */
void gf3d_sprite_manager_init(Uint32 max_sprites,Uint32 chain_length,VkDevice device);

/**
 * @brief loads a sprite sheet into memory
 * @param frame_width how wide an individual frame is on the sprite sheet
 * @param frame_height how high an individual frame is on the sprite sheet
 * @param frames_per_line how many frames across are on the sprite sheet
 * @return NULL on error (check logs) or a pointer to a sprite that can be draw to the 2d overlay
 */
Sprite * gf3d_sprite_load(char * filename,int frame_width,int frame_height, Uint32 frames_per_line);

/**
 * @brief free a previously loaded sprite
 * @param sprite a pointer to the sprite to be freed
 */
void gf3d_sprite_free(Sprite *sprite);

/**
 * @brief draw a sprite frame to the current buffer frame
 * @param sprite the sprite to draw
 * @param position where on the screen to draw the sprite
 * @param scale amount to scale the sprite by.  (1,1) is no scale
 * @param frame the frame of the sprite to draw
 */
void gf3d_sprite_draw(Sprite *sprite,Vector2D position,Vector2D scale,Uint32 frame);

/**
 * @brief get the default pipeline for overlay rendering
 * @return NULL on error or not yet initlialized, the pipeline otherwise
 */
Pipeline *gf3d_sprite_get_pipeline();

/**
 * @brief get the binding description for a sprite
 */
VkVertexInputBindingDescription * gf3d_sprite_get_bind_description();

VkVertexInputAttributeDescription * gf3d_sprite_get_attribute_descriptions(Uint32 *count);


#endif
