#ifndef __GF3D_MODEL_H__
#define __GF3D_MODEL_H__

/**
 * gf3d_model
 * @license The MIT License (MIT)
   @copyright Copyright (c) 2015 EngineerOfLies
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
#include "gf3d_mesh.h"

/**
 * @purpose the model is a single instance of 3d mesh data.  Each can be drawn individually in the rendering pipeline.
 * Multiple models can reference the same mesh and texture data, but will have separate UBO data and descriptorSets
 */
typedef struct
{
    Uint8                       _inuse;
    TextLine                    filename;
    Mesh                    *   mesh;
    Texture                 *   texture;
    VkDescriptorSet         *   descriptorSet;
    VkBuffer                   *uniformBuffers;
    VkDeviceMemory             *uniformBuffersMemory;
    Uint32                      uniformBufferCount;
}Model;


void gf3d_model_manager_init(Uint32 max_models,Uint32 chain_length,VkDevice device);

Model * gf3d_model_load(char * filename);
Model * gf3d_model_new();
/**
 * @brief queue up a model for rendering
 * @param model the model to render
 * @param bufferFrame the swap chain frame to render for
 * @param commandBuffer the command used to send this render request
 * @param modelMat the model matrix (MVP)
 */
void gf3d_model_draw(Model *model,Uint32 bufferFrame,VkCommandBuffer commandBuffer,Matrix4 modelMat);
void gf3d_model_free(Model *model);

/**
 * @brief update the descriptorSet with the model data needed to submit the draw command for the model provided
 * @param model the model data to populate the descriptor set with
 * @param descriptSet the descriptorSet to populate
 * @param chainIndex the swap chain frame to do this for
 * @param modelMat the matrix to transform the model by
 */
void gf3d_model_update_basic_model_descriptor_set(Model *model,VkDescriptorSet descriptorSet,Uint32 chainIndex,Matrix4 modelMat);


#endif
