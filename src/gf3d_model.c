#include <assert.h>

#include "simple_logger.h"

#include "gfc_config.h"
#include "gfc_pak.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_commands.h"
#include "gf3d_vgraphics.h"
#include "gf3d_obj_load.h"
#include "gf3d_uniform_buffers.h"
#include "gf3d_lights.h"
#include "gf3d_camera.h"

#include "gf3d_gltf_parse.h"
#include "gf3d_model.h"

extern int __DEBUG;

typedef struct
{
    Uint8                   initiliazed;
    Model               *   model_list;
    Uint32                  max_models;
    Uint32                  chain_length;   /**<length of swap chain*/
    VkDevice                device;
    Pipeline            *   pipe;           /**<the pipeline associated with model rendering*/
    Texture             *   defaultTexture; /**<if a model has no texture, use this one*/
    Texture             *   defaultNormal;  /**<if a model has no normal map, use this one*/
}ModelManager;

static ModelManager gf3d_model = {0};

void gf3d_model_delete(Model *model);
Model * gf3d_model_get_by_filename(const char *filename);

void gf3d_model_create_descriptor_pool(Model *model);
void gf3d_model_create_descriptor_sets(Model *model);
void gf3d_model_create_descriptor_set_layout();
void gf3d_model_update_uniform_buffer(
    Model *model,
    UniformBuffer *ubo,
    Matrix4 modelMat,
    Vector4D colorMod,
    Vector4D ambientLight
);

HighlightUBO gf3d_model_get_highlight_ubo(
    Matrix4 modelMat,
    Vector4D highlightColor);


VkDescriptorSetLayout * gf3d_model_get_descriptor_set_layout();

void gf3d_model_manager_close()
{
    int i;
    for (i = 0; i < gf3d_model.max_models;i++)
    {
        gf3d_model_delete(&gf3d_model.model_list[i]);
    }
    if (gf3d_model.model_list)
    {
        free(gf3d_model.model_list);
    }
    memset(&gf3d_model,0,sizeof(ModelManager));
    if(__DEBUG)slog("model manager closed");
}

void gf3d_model_manager_init(Uint32 max_models)
{
    if (max_models == 0)
    {
        slog("cannot intilizat model manager for 0 models");
        return;
    }
    gf3d_model.chain_length = gf3d_swapchain_get_chain_length();
    gf3d_model.model_list = (Model *)gfc_allocate_array(sizeof(Model),max_models);
    gf3d_model.max_models = max_models;
    gf3d_model.device = gf3d_vgraphics_get_default_logical_device();
    gf3d_model.pipe = gf3d_mesh_get_pipeline();
    gf3d_model.defaultTexture = gf3d_texture_load("images/default.png");
    gf3d_model.defaultNormal = gf3d_texture_load("images/defaultNormalMap.png");
    gf3d_model.initiliazed = 1;
    if(__DEBUG)slog("model manager initiliazed");
    atexit(gf3d_model_manager_close);
}

Model * gf3d_model_get_by_filename(const char *filename)
{
    int i;
    if (!gf3d_model.initiliazed)return NULL;
    for (i = 0; i < gf3d_model.max_models;i++)
    {
        if (!gf3d_model.model_list[i].refCount)
        {
            continue;
        }
        if (strcmp(filename,gf3d_model.model_list[i].filename)==0)return &gf3d_model.model_list[i];
    }
    return NULL;
}

Model * gf3d_model_new()
{
    int i;
    if (!gf3d_model.initiliazed)return NULL;
    for (i = 0; i < gf3d_model.max_models;i++)
    {
        if (!gf3d_model.model_list[i].refCount)
        {
            gf3d_model_delete(&gf3d_model.model_list[i]);
            gf3d_model.model_list[i].refCount = 1;
            gf3d_model.model_list[i].mesh_list = gfc_list_new();
            return &gf3d_model.model_list[i];
        }
    }
    slog("unable to make a new model, out of space");
    return NULL;
}

Model *gf3d_model_load(const char * filename)
{    
    SJson *json,*config;
    Model *model;
    if (!gf3d_model.initiliazed)return NULL;
    if (!filename)return NULL;
    
    model = gf3d_model_get_by_filename(filename);
    if (model)
    {
        model->refCount++;
        return model;
    }
    json = gfc_pak_load_json(filename);
    if (!json)return NULL;
    
    config = sj_object_get_value(json,"model");
    if (!config)
    {
        slog("file %s contains no model object",filename);
        sj_free(json);
        return NULL;
    }
    model = gf3d_model_load_from_config(config);
    if (model)
    {
        gfc_line_cpy(model->filename,filename);
    }
    sj_free(json);
    return model;
}

Model *gf3d_model_load_from_config(SJson *json)
{
    int i,c;
    Mesh *mesh;
    SJson *array,*item;
    Model *model;
    const char *modelFile;
    const char *textureFile;
    if (!gf3d_model.initiliazed)return NULL;
    if (!json)return NULL;

    modelFile = sj_get_string_value(sj_object_get_value(json,"gltf"));
    if (modelFile)
    {
        model = gf3d_gltf_parse_model(modelFile);
        textureFile = sj_get_string_value(sj_object_get_value(json,"texture"));
        if (textureFile)
        {
            model->texture = gf3d_texture_load(textureFile);

        }
        return model;
    }    
    
    model = gf3d_model_new();
    if (!model)return NULL;
    
    textureFile = sj_get_string_value(sj_object_get_value(json,"normalMap"));
    if (textureFile)
    {
        model->normalMap = gf3d_texture_load(textureFile);
    }

    textureFile = sj_get_string_value(sj_object_get_value(json,"texture"));
    if (textureFile)
    {
        model->texture = gf3d_texture_load(textureFile);
    }

    modelFile = sj_get_string_value(sj_object_get_value(json,"obj"));
    if (modelFile)
    {
        mesh = gf3d_mesh_load(modelFile);
        if (!mesh)
        {
            slog("failed to parse mesh data from obj file");
            gf3d_model_free(model);
            return NULL;
        }
        model->mesh_list = gfc_list_append(model->mesh_list,mesh);
        return model;
    }
    array = sj_object_get_value(json,"obj_list");
    if (array)
    {
        c = sj_array_get_count(array);
        for (i = 0; i < c; i++)
        {
            item = sj_array_get_nth(array,i);
            if (!item)continue;
            modelFile = sj_get_string_value(item);
            if (modelFile)
            {
                mesh = gf3d_mesh_load(modelFile);
                if (mesh)model->mesh_list = gfc_list_append(model->mesh_list,mesh);
            }
        }
        return model;
    }
    slog("no known way to parse model file");
    gf3d_model_free(model);
    return NULL;
}


void gf3d_model_free(Model *model)
{
    if (!model)return;
    if (!gf3d_model.initiliazed)return;
    model->refCount--;
    if (model->refCount<=0)
    {
        gf3d_model_delete(model);
    }
}

void gf3d_model_delete(Model *model)
{
    int i,c;
    Mesh *mesh;
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    c = gfc_list_get_count(model->mesh_list);
    for (i = 0; i < c; i++)
    {
        mesh = gfc_list_get_nth(model->mesh_list,i);
        if (!mesh)continue;
        gf3d_mesh_free(mesh);
    }
    gfc_list_delete(model->mesh_list);
    gf3d_texture_free(model->texture);
    memset(model,0,sizeof(Model));
}

Model * gf3d_model_load_full(const char * modelFile,const char *textureFile)
{
    Mesh *mesh;
    Model *model;
    if (!gf3d_model.initiliazed)return NULL;
    
    model = gf3d_model_get_by_filename(modelFile);
    if (model)
    {
        model->refCount++;
        return model;
    }
    model = gf3d_model_new();
    if (!model)return NULL;
    
    gfc_line_cpy(model->filename,modelFile);

    mesh = gf3d_mesh_load(modelFile);
    if (!mesh)
    {
        gf3d_model_free(model);
        return NULL;
    }
    model->mesh_list = gfc_list_append(model->mesh_list,mesh);
    
    model->texture = gf3d_texture_load(textureFile);

    if (!model->texture)
    {
        model->texture = gf3d_texture_load("images/default.png");
    }
    
    return model;
}

void gf3d_model_mat_set_scale(ModelMat *mat,Vector3D scale)
{
    if (!mat)return;
    vector3d_copy(mat->scale,scale);
}

void gf3d_model_mat_set_position(ModelMat *mat,Vector3D position)
{
    if (!mat)return;
    vector3d_copy(mat->position,position);
}

void gf3d_model_mat_set_rotation(ModelMat *mat,Vector3D rotation)
{
    if (!mat)return;
    vector3d_copy(mat->rotation,rotation);
}

void gf3d_model_mat_scale(ModelMat *mat,Vector3D scale)
{
    if (!mat)return;
    vector3d_multiply(mat->scale,scale);
}

void gf3d_model_mat_move(ModelMat *mat,Vector3D translation)
{
    if (!mat)return;
    vector3d_add(mat->position,mat->position,translation);
}

void gf3d_model_mat_rotate(ModelMat *mat,Vector3D rotation)
{
    if (!mat)return;
    vector3d_add(mat->rotation,mat->rotation,rotation);
}

void gf3d_model_mat_reset(ModelMat *mat)
{
    if (!mat)return;
    memset(mat,0,sizeof(ModelMat));
    gfc_matrix_identity(mat->mat);
    mat->scale = vector3d(1,1,1);
    mat->scaleDelta = vector3d(1,1,1);
}

void gf3d_model_mat_extract_vectors(ModelMat *mat)
{
    if (!mat)return;
    gfc_matrix4_to_vectors(
        mat->mat,
        &mat->position,
        &mat->rotation,
        &mat->scale);
}

void gf3d_model_mat_set_matrix(ModelMat *mat)
{
    if (!mat)return;
    gfc_matrix4_from_vectors(
        mat->mat,
        mat->position,
        mat->rotation,
        mat->scale);
}

void gf3d_model_mat_free(ModelMat *mat)
{
    if (!mat)return;
    if (mat->model)gf3d_model_free(mat->model);
    free(mat);
}

SJson *gf3d_model_mat_save(ModelMat *mat,Bool updateFirst)
{
    SJson *json;
    if (!mat)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
    if (updateFirst)
    {
        gf3d_model_mat_extract_vectors(mat);
    }
    if (mat->model)sj_object_insert(json,"model",sj_new_str(mat->model->filename));
    sj_object_insert(json,"position",sj_vector3d_new(mat->position));
    sj_object_insert(json,"rotation",sj_vector3d_new(mat->rotation));
    sj_object_insert(json,"scale",sj_vector3d_new(mat->scale));
    sj_object_insert(json,"positionDelta",sj_vector3d_new(mat->positionDelta));
    sj_object_insert(json,"rotationDelta",sj_vector3d_new(mat->rotationDelta));
    sj_object_insert(json,"scaleDelta",sj_vector3d_new(mat->scaleDelta));
    return json;
}

void mat_from_parent(
    Matrix4 out,
    Matrix4 parent,
    Vector3D position,
    Vector3D rotation,
    Vector3D scale)
{
    Matrix4 temp;
    gfc_matrix4_from_vectors(temp,position,rotation,scale);
    gfc_matrix_multiply(out,temp,parent);
}


void gf3d_model_mat_parse(ModelMat *mat,SJson *config)
{
    const char *str;
    if (!mat)return;
    if (!config)return;
    gfc_matrix_identity(mat->mat);
    str = sj_object_get_value_as_string(config,"model");
    if (str)mat->model= gf3d_model_load(str);
    sj_value_as_vector3d(sj_object_get_value(config,"position"),&mat->position);
    sj_value_as_vector3d(sj_object_get_value(config,"rotation"),&mat->rotation);
    sj_value_as_vector3d(sj_object_get_value(config,"positionDelta"),&mat->positionDelta);
    sj_value_as_vector3d(sj_object_get_value(config,"rotationDelta"),&mat->rotationDelta);
    sj_value_as_vector3d(sj_object_get_value(config,"scaleDelta"),&mat->scaleDelta);
    vector3d_scale(mat->rotation,mat->rotation,GFC_DEGTORAD);//config file is in degrees
    vector3d_scale(mat->rotationDelta,mat->rotationDelta,GFC_DEGTORAD);//config file is in degrees
    sj_value_as_vector3d(sj_object_get_value(config,"scale"),&mat->scale);
}

ModelMat *gf3d_model_mat_new()
{
    ModelMat *modelMat;
    modelMat = gfc_allocate_array(sizeof(ModelMat),1);
    if (!modelMat)return NULL;
    gfc_matrix_identity(modelMat->mat);
    vector3d_set(modelMat->scale,1,1,1);
    vector3d_set(modelMat->scaleDelta,1,1,1);
    return modelMat;
}

MeshUBO gf3d_model_get_mesh_ubo(
    Matrix4 modelMat,
    Vector4D colorMod,
    Vector4D ambient,
    Vector4D detail)

{
    Vector3D cameraPosition;
    UniformBufferObject graphics_ubo;
    MeshUBO modelUBO = {0};
    
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    gfc_matrix_copy(modelUBO.model,modelMat);
    gfc_matrix_copy(modelUBO.view,graphics_ubo.view);
    gfc_matrix_copy(modelUBO.proj,graphics_ubo.proj);
    vector4d_copy(modelUBO.color,colorMod);
    vector4d_copy(modelUBO.detailColor,detail);
    cameraPosition = gf3d_camera_get_position();
    vector3d_copy(modelUBO.cameraPosition,cameraPosition);
    modelUBO.cameraPosition.w = 1;
    
    gf3d_lights_get_global_light(&modelUBO.ambientColor, &modelUBO.ambientDir);
    vector4d_scale_by(modelUBO.ambientColor,modelUBO.ambientColor,ambient);

    return modelUBO;
}

SkyUBO gf3d_model_get_sky_ubo(
    Matrix4 modelMat,
    Vector4D colorMod)
{
    UniformBufferObject graphics_ubo;
    SkyUBO modelUBO;
    
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    
    gfc_matrix_copy(modelUBO.model,modelMat);
    gfc_matrix_copy(modelUBO.view,graphics_ubo.view);
     modelUBO.view[0][3] = 0;
     modelUBO.view[1][3] = 0;
     modelUBO.view[2][3] = 0;
     modelUBO.view[3][0] = 0;
     modelUBO.view[3][1] = 0;
     modelUBO.view[3][2] = 0;
    gfc_matrix_copy(modelUBO.proj,graphics_ubo.proj);
    vector4d_copy(modelUBO.color,colorMod);
    return modelUBO;
}

HighlightUBO gf3d_model_get_highlight_ubo(
    Matrix4 modelMat,
    Vector4D highlightColor)
{
    UniformBufferObject graphics_ubo;
    HighlightUBO modelUBO = {0};
    
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    
    gfc_matrix_copy(modelUBO.model,modelMat);
    gfc_matrix_copy(modelUBO.view,graphics_ubo.view);
    gfc_matrix_copy(modelUBO.proj,graphics_ubo.proj);
    
    vector4d_copy(modelUBO.color,highlightColor);
    return modelUBO;
}

void gf3d_model_draw(Model *model,Uint32 index,Matrix4 modelMat,Vector4D colorMod,Vector4D detailColor, Vector4D ambientLight)
{
    Mesh *mesh;
    MeshUBO uboData = {0};
    Texture *texture;
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    uboData = gf3d_model_get_mesh_ubo(
        modelMat,
        colorMod,
        ambientLight,
        detailColor);
    
    if (!model->texture)
    {
        texture = gf3d_model.defaultTexture;
    }
    else texture = model->texture;
        // queue up a render for batch rendering
    mesh = gfc_list_get_nth(model->mesh_list,index);
    gf3d_mesh_queue_render(mesh,gf3d_model.pipe,&uboData,texture);
    gf3d_mesh_queue_render(mesh,gf3d_mesh_get_alpha_pipeline(),&uboData,texture);
}

void gf3d_model_draw_highlight(Model *model,Uint32 index,Matrix4 modelMat,Vector4D highlight)
{
    Mesh *mesh;
    Texture *texture;
    HighlightUBO uboData = {0};
    
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    uboData = gf3d_model_get_highlight_ubo(modelMat,highlight);
    if (!model->texture)
    {
        texture = gf3d_model.defaultTexture;
    }
    else texture = model->texture;
    // queue up a render for batch rendering
    mesh = gfc_list_get_nth(model->mesh_list,index);
    gf3d_mesh_queue_render(mesh,gf3d_mesh_get_highlight_pipeline(),&uboData,texture);
}

void gf3d_model_draw_sky(Model *model,Matrix4 modelMat,Color color)
{
    Mesh *mesh;
    Texture *texture;
    SkyUBO uboData = {0};
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    uboData = gf3d_model_get_sky_ubo(modelMat,gfc_color_to_vector4f(color));
    if (!model->texture)
    {
        texture = gf3d_model.defaultTexture;
    }
    else texture = model->texture;
    // queue up a render for batch rendering
    mesh = gfc_list_get_nth(model->mesh_list,0);
    
    gf3d_mesh_queue_render(mesh,gf3d_mesh_get_sky_pipeline(),&uboData,texture);
}

/*eol@eof*/
