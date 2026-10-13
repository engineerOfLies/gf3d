#ifndef __MODEL_H__
#define __MODEL_H__

#include "gf3d_mesh.h"

//absolute basics of the mesh information sent to the graphics card
typedef struct
{
    GFC_Matrix4     model;
    GFC_Matrix4     view;
    GFC_Matrix4     proj;
    GFC_Vector4D    color;
}ModelUBO;

typedef struct
{
	int _refCount;
    GFC_TextLine    filename;
    Texture         *texture;           /**<texture memory pointer*/
    Mesh*           mesh;               /**<GPU handels for mesh data*/
}Model;

/**
 * @brief initialize the internal management system for models, auto-cleaned up on program exit
 * @param max_models how many concurrent sprites to support
 */
void model_manager_init(Uint32 max_models);

/**
 * @brief get a pointer to a free model
 * @note this provides a pointer to a zero initialized model, no other work is done
 * @return NULL on error or an empty model otherwise
 */
Model* model_new();

/**
 * @brief loads a model into memory from an .model filename
 * @param filename the name of the file containing the model data
 * @return NULL on error (check logs) or a pointer to a model that can be rendered
 */
Model* model_load(const char* filename);

/**
 * @brief create a model from an Mesh
 * @param mesh pointer to loaded mesh data
 * @return NULL on error (check logs) or a pointer to a model that can be rendered
 * @note not implemented yet, may want to in the future
 */
Model* model_from_mesh(Mesh mesh);

/**
 * @brief free a previously loaded model
 * @param model a pointer to the model to be freed
 */
void model_free(Model* model);

/**
 * @brief draw a model to the screen 
 * @param the model to draw
 * @param mat where to draw the model?
 * @param colorMod what color to draw the model?
 */
void model_queue_render(Model* model, GFC_Matrix4 mat, GFC_Color colorMod);

/**
 * @brief get the pipeline that is used to render basic 3d models
 * @return NULL on error or the pipeline in question
 */
Pipeline* model_get_pipeline();

/**
 * @brief given a model matrix and basic color, build the modelUBO needed to render a model
 * @param modelMat the model Matrix
 * @param colorMod the color for the UBO
 */
ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod);

#endif