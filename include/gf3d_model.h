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
#include "gfc_list.h"

#include "gf3d_texture.h"
#include "gf3d_mesh.h"
#include "gf3d_armature.h"
#include "gf3d_lights.h"
#include "gf3d_materials.h"


//all inclusive of features
typedef struct
{
    MeshUBO         mesh;
    ArmatureUBO     armature;
    MaterialUBO     material;   //this may become an array
    LightUBO        lights;
    GFC_Vector4D    flags;      //.x is for bones, .y is for transparency/opaque pass 
}ModelUBO;

/**
 * @purpose the model is a single instance of 3d mesh data.  Each can be drawn individually in the rendering pipeline.
 * Multiple models can reference the same mesh and texture data, but will have separate UBO data and descriptorSets
 */
typedef struct
{
    Uint32              refCount;
    GFC_TextLine        filename;
    
    GFC_List           *mesh_list;      //sub meshes of the model
    Uint8               mesh_as_frame;  //if true, this model is animated as a sequence of meshes
    GF3D_Material      *material;       //if set, use this material when sending draw calls
    Texture            *texture;
    Texture            *normalMap;
    Armature3D         *armature;
    GFC_Box             bounds;         //copied from the mesh
    GFC_Matrix4         matrix;         //a delta applied right before rendering.  an adjustment loaded from file
    GFC_ActionList     *action_list;    //list of animation actions
}Model;

/**
 * @brief this struct groups the model with common use case for it.
 * for many game objects and entities, they will dynamically change the model's matrix to suit
 */
typedef struct
{
    Model *model;
    GFC_Matrix4 mat;
    GFC_Vector3D position;
    GFC_Vector3D rotation;
    GFC_Vector3D scale;
    GFC_Vector3D positionDelta;
    GFC_Vector3D rotationDelta;
    GFC_Vector3D scaleDelta;
}ModelMat;

/**
 * @brief setup the model manager
 * @param max_models the maximum number of models that can be held in memory
 * @param chain_length how many swap chains are supported
 * @param device the logical device to use
 */
void gf3d_model_manager_init(Uint32 max_models);

/**
 * @brief get a blank model address
 * @return NULL on full, or a pointer to a blank model
 */
Model * gf3d_model_new();

/**
 * @brief load a model and texture from a config file that describe where the mesh data and texture data can be found
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
 * @param filename the name of the file this was loaded from
 * @return NULL on error, or the json 
 */
Model * gf3d_model_load_from_config(SJson *json,const char *filename);

/**
 * @brief make a separate copy of the input model
 * @param in the model to copy
 * @return NULL on error or a new copy of the input model
 */
Model *gf3d_model_copy(Model *in);

/**
 * @brief append the mesh data from modelB to modelA
 * @note modelB is not affected in this proccess
 * @param modelA the model to gain mesh data
 * @param modelB the model to provide mesh data
 * @param offset move the vertices of modelB by this much as they are added
 * @param rotation apply this rotation to the vertices and normals
 */
void gf3d_model_append(Model *modelA,Model *modelB, GFC_Vector3D offsetB,GFC_Vector3D rotation);

/**
 * @brief move all of the vertices of the model mesh data by the offset
 * @param in the model to move
 * @param offset how much to move it
 * @param rotation apply this rotation to the vertices and normals
 * @note this is memory heavy.  do not do often
 */
void gf3d_model_move(Model *in, GFC_Vector3D offset,GFC_Vector3D rotation);

/**
 * @brief queue up a model for rendering
 * @param model the model to render
 * @param modelMat the model matrix (MVP)
 * @param colorMod color modulation (values from 0 to 1);
 * @param lighting the lighting that should be applied
 * @param frame the animation frame to use for armature based animations
 */
void gf3d_model_draw(
    Model *model,
    GFC_Matrix4 modelMat,
    GFC_Color   colorMod,//TODO pass a material instead
    LightUBO *lighting,
    Uint32 frame);

/**
 * @brief queue up a model for rendering, specifying one mesh in the model (this can be for animation, or sub-meshes)
 * @param model the model to render
 * @param index the mesh to render from the mesh_list, could be animation frames of a sequence of objs, or sub-meshes
 * @param modelMat the model matrix (MVP)
 * @param colorMod color modulation (values from 0 to 1);
 * @param lighting the lighting that should be applied
 * @param frame the animation frame to use for armature based animations
 */
void gf3d_model_draw_index(
    Model *model,
    Uint32 index,
    GFC_Matrix4 modelMat,
    GFC_Color   colorMod,
    LightUBO *lighting,
    Uint32 frame);

/**
 * @brief draw all of the meshes of a model.  This is meant for multi-mesh models
 * @param model the model to render
 * @param modelMat the model matrix (MVP)
 * @param colorMod color modulation
 * @param lighting the lighting that should be applied
 * @param frame used to access a frame of armature based animation
 * @note this is called by gf3d_model_draw when not using a sequence of meshes
 */
void gf3d_model_draw_all_meshes(
    Model *model,
    GFC_Matrix4 modelMat,
    GFC_Color   colorMod,
    LightUBO *lighting,
    Uint32 frame);

/**
 * @brief queue up a model for rendering as highlight wireframe
 * @param model the model to render
 * @param modelMat the model matrix (MVP)
 * @param highlightGFC_Color the color of the outline
 */
void gf3d_model_draw_highlight(Model *model,Uint32 index,GFC_Matrix4 modelMat,GFC_Color highlight);

/**
 * @brief queue up a model for rendering as a sky
 * @param model the model to render
 * @param modelMat the model matrix (MVP)
 * @param color the color adjustement (gfc_color(1,1,1,1) for no color change
 */
void gf3d_model_draw_sky(Model *model,GFC_Matrix4 modelMat,GFC_Color color);

/**
 * @brief free a model
 */
void gf3d_model_free(Model *model);

/**
 * @brief populate a modelMat based on config info
 * @note pulls model,position,rotation, and scale out of the config
 * @param mat the matrix to populate
 * @param config the json to find the information
 */
void gf3d_model_mat_parse(ModelMat *mat,SJson *config);

/**
 * @brief used to make a matrix when something is a child of another matrix
 * @param out the output parameter
 * @param parent the matrix used to multiply the new matrix
 * @param position used to make this matrix
 * @param rotation used to make this matrix
 * @param scale used to make this matrix
 */
void mat_from_parent(
    GFC_Matrix4 out,
    GFC_Matrix4 parent,
    GFC_Vector3D position,
    GFC_Vector3D rotation,
    GFC_Vector3D scale);


/**
 * @brief save a modelMat to config
 * @param mat the matrix to save
 * @param updateFirst if true, run the extraction from the matrix4 to the component gfc_vectors before making the config
 * @return NULL on error, or the populated SJson otherwise
 */
SJson *gf3d_model_mat_save(ModelMat *mat,Bool updateFirst);

/**
 * @brief allocate an initialize a blank ModelMat
 * @return NULL on memory error, or an initialized modelMat otherwise
 */
ModelMat *gf3d_model_mat_new();

/**
 * @brief free an allocated modelMat and its model
 * @param mat the modelmat to free
 */
void gf3d_model_mat_free(ModelMat *mat);

/**
 * @brief reset a ModelMat to identity (all values zero except for the scale values to 1)
 * @param mat the ModelMat to reset.
 */
void gf3d_model_mat_reset(ModelMat *mat);

/**
 * @brief set the modelMat matrix based on its position,rotation, and scale
 * @param mat the modelMat that will have its matrix set
 */
void gf3d_model_mat_set_matrix(ModelMat *mat);

/**
 * @brief extract the translation, rotation, and scale gfc_vectors from the modelMat matrix
 * @param mat the matrix to extract the gfc_vectors from
 */
void gf3d_model_mat_extract_vectors(ModelMat *mat);

/**
 * @brief set a model mat scale
 * @param mat the modelMat to set
 * @param scale the scale value (1,1,1) is no scale at all
 */
void gf3d_model_mat_set_scale(ModelMat *mat,GFC_Vector3D scale);

/**
 * @brief set a model mat scale
 * @param mat the modelMat to set
 * @param position the position to set
 */
void gf3d_model_mat_set_position(ModelMat *mat,GFC_Vector3D position);

/**
 * @brief set a model mat rotation
 * @param mat the modelMat to set
 * @param rotation the rotation to set
 */
void gf3d_model_mat_set_rotation(ModelMat *mat,GFC_Vector3D rotation);

/**
 * @brief scale a modelMat from its current scale
 * @param mat the modelMat to change
 * @param scale the amount to change the scale by
 */
void gf3d_model_mat_scale(ModelMat *mat,GFC_Vector3D scale);

/**
 * @brief move a modelMat from its current position
 * @param mat the modelMat to change
 * @param translation the amount to change the position by
 */
void gf3d_model_mat_move(ModelMat *mat,GFC_Vector3D translation);

/**
 * @brief rotate a modelMat from its current rotation
 * @param mat the modelMat to change
 * @param rotation the amount to change the rotation by
 */
void gf3d_model_mat_rotate(ModelMat *mat,GFC_Vector3D rotation);

/**
 * @brief parse a mesh / model out of a gltf file.  Including bone associations and weights
 * @param filename the gltf file containing a number of meshes
 * @return NULL on error, or the model file loaded.
 */
Model *gf3d_gltf_parse_model(const char *filename);

#endif
