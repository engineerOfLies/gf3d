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

#include "gf3d_graphics.h"
#include "gf3d_types.h"
#include "gf3d_vector.h"
#include "gf3d_matrix.h"
#include "gf3d_text.h"

typedef struct
{
    Uint32 ref_count;
    TextLine filepath;
    
    GLuint vertex_buffer;
    GLuint normal_buffer;
    GLuint texel_buffer;
    GLuint textures[2];
    Uint32 vertex_count;
    Uint32 normal_count;
}Model;

/**
 * @brief initialize the 3d model manager
 * @param max the limit on the number of models that can be held in memory at once
 */
void gf3d_model_manager_init(Uint32 max);

/**
 * @brief unloads all models from memory, but keeps model system initialized
 */
void gf3d_model_clear_all();

/**
 * @brief load 3d model data from a json file
 * @param filename the path to the file to load
 * @returns NULL on error or a pointer to a setup model
 */
Model *gf3d_model_load_from_json_file(char *filename);

/**
 * @brief make a model object that contains only a triange
 * @return NULL if error, or a handle to a triangle model otherwise
 */
Model *gf3d_model_new_triangle();

/**
 * @brief render a model this frame
 * @param model the model to render
 * @param mat the model matrix, containing the translation, rotation, and scaling of the model, if NULL identity matrix is used
 * @param program the shader program to use
 */
void gf3d_model_render(Model *model,Matrix4 mat,GLuint program);

/**
 * @brief free a previously loaded model
 * @param model pointer to the model data to free
 */
void gf3d_model_free(Model *model);

#endif
