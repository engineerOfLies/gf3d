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

#include "simple_json.h"

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
}Model;

/**
 * @brief setup the model manager
 * @param max_models the maximum number of models that can be held in memory
 * @param chain_length how many swap chains are supported
 * @param device the logical device to use
 */
void gf3d_model_manager_init(Uint32 max_models,Uint32 chain_length,VkDevice device);

/**
 * @brief get a blank model address
 * @return NULL on full, or a pointer to a blank model
 */
Model * gf3d_model_new();

/**
 * @brief load a model and texture from file where both the model is in models/<filename>.obj and the
 * texture is in images><filename>,png
 * @param filename the common filename to load by
 * @return NULL on error, or the loaded model data otherwise
 */
Model * gf3d_model_load(const char * filename);

/**
 * @brief load a model by its model file path and texture file path
 * @param modelFile where to find the model obj file
 * @param textureFile where to find the image for the texture
 * @return NULL on error or the model file otherwise.  
 */
Model * gf3d_model_load_full(const char * modelFile,const char *textureFile);

/**
 * @brief load a model from config file
 * @param json the json config to parse
 * @return NULL on error, or the json 
 */
Model * gf3d_model_load_from_config(SJson *json);

/**
 * @brief queue up a model for rendering
 * @param model the model to render
 * @param modelMat the model matrix (MVP)
 * @param colorMod color modulation (values from 0 to 1);
 * @param ambient how much ambient light there is
 */
void gf3d_model_draw(Model *model,Matrix4 modelMat,Vector4D colorMod,Vector4D ambient);

/**
 * @brief queue up a model for rendering as highlight wireframe
 * @param model the model to render
 * @param modelMat the model matrix (MVP)
 * @param highlightColor the color of the outline
 */
void gf3d_model_draw_highlight(Model *model,Matrix4 modelMat,Vector4D highlight);

/**
 * @brief free a model
 */
void gf3d_model_free(Model *model);

#endif
