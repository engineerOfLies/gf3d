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
#include "gf3d_text.h"

typedef struct
{
    Uint32 ref_count;
    TextLine filepath;
    
    GLuint vertex_buffer, element_buffer;
    GLuint textures[2];
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

#endif
