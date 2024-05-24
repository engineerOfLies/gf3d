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
    Texture             *   defaultSpecular;/**<if a model has no normal map, use this one*/
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
    GFC_Matrix4 modelMat,
    GFC_Vector4D colorMod,
    GFC_Vector4D ambientLight
);


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
    const char *armatureFile;
    if (!gf3d_model.initiliazed)return NULL;
    if (!json)return NULL;

    modelFile = sj_get_string_value(sj_object_get_value(json,"gltf"));
    if (modelFile)
    {
        model = gf3d_gltf_parse_model(modelFile);
        if (!model)return NULL;

        textureFile = sj_get_string_value(sj_object_get_value(json,"texture"));
        if (textureFile)
        {
            model->texture = gf3d_texture_load(textureFile);
        }
        armatureFile = sj_get_string_value(sj_object_get_value(json,"armature"));
        if (armatureFile)
        {
            model->armature = gf3d_armature_load(armatureFile);
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
        gfc_box_cpy(model->bounds,mesh->bounds);
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
                if (mesh)
                {
                    gfc_box_cpy(model->bounds,mesh->bounds);
                    model->mesh_list = gfc_list_append(model->mesh_list,mesh);
                }
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
    gf3d_armature_free(model->armature);
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
    gfc_box_cpy(model->bounds,mesh->bounds);
    model->mesh_list = gfc_list_append(model->mesh_list,mesh);
    
    model->texture = gf3d_texture_load(textureFile);

    if (!model->texture)
    {
        model->texture = gf3d_texture_load("images/default.png");
    }
    
    return model;
}

void gf3d_model_mat_set_scale(ModelMat *mat,GFC_Vector3D scale)
{
    if (!mat)return;
    gfc_vector3d_copy(mat->scale,scale);
}

void gf3d_model_mat_set_position(ModelMat *mat,GFC_Vector3D position)
{
    if (!mat)return;
    gfc_vector3d_copy(mat->position,position);
}

void gf3d_model_mat_set_rotation(ModelMat *mat,GFC_Vector3D rotation)
{
    if (!mat)return;
    gfc_vector3d_copy(mat->rotation,rotation);
}

void gf3d_model_mat_scale(ModelMat *mat,GFC_Vector3D scale)
{
    if (!mat)return;
    gfc_vector3d_multiply(mat->scale,scale);
}

void gf3d_model_mat_move(ModelMat *mat,GFC_Vector3D translation)
{
    if (!mat)return;
    gfc_vector3d_add(mat->position,mat->position,translation);
}

void gf3d_model_mat_rotate(ModelMat *mat,GFC_Vector3D rotation)
{
    if (!mat)return;
    gfc_vector3d_add(mat->rotation,mat->rotation,rotation);
}

void gf3d_model_mat_reset(ModelMat *mat)
{
    if (!mat)return;
    memset(mat,0,sizeof(ModelMat));
    gfc_matrix4_identity(mat->mat);
    mat->scale = gfc_vector3d(1,1,1);
    mat->scaleDelta = gfc_vector3d(1,1,1);
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
    GFC_Matrix4 out,
    GFC_Matrix4 parent,
    GFC_Vector3D position,
    GFC_Vector3D rotation,
    GFC_Vector3D scale)
{
    GFC_Matrix4 temp;
    gfc_matrix4_from_vectors(temp,position,rotation,scale);
    gfc_matrix4_multiply(out,temp,parent);
}


void gf3d_model_mat_parse(ModelMat *mat,SJson *config)
{
    const char *str;
    if (!mat)return;
    if (!config)return;
    gfc_matrix4_identity(mat->mat);
    str = sj_object_get_value_as_string(config,"model");
    if (str)mat->model= gf3d_model_load(str);
    sj_value_as_vector3d(sj_object_get_value(config,"position"),&mat->position);
    sj_value_as_vector3d(sj_object_get_value(config,"rotation"),&mat->rotation);
    sj_value_as_vector3d(sj_object_get_value(config,"positionDelta"),&mat->positionDelta);
    sj_value_as_vector3d(sj_object_get_value(config,"rotationDelta"),&mat->rotationDelta);
    sj_value_as_vector3d(sj_object_get_value(config,"scaleDelta"),&mat->scaleDelta);
    gfc_vector3d_scale(mat->rotation,mat->rotation,GFC_DEGTORAD);//config file is in degrees
    gfc_vector3d_scale(mat->rotationDelta,mat->rotationDelta,GFC_DEGTORAD);//config file is in degrees
    sj_value_as_vector3d(sj_object_get_value(config,"scale"),&mat->scale);
}

ModelMat *gf3d_model_mat_new()
{
    ModelMat *modelMat;
    modelMat = gfc_allocate_array(sizeof(ModelMat),1);
    if (!modelMat)return NULL;
    gfc_matrix4_identity(modelMat->mat);
    gfc_vector3d_set(modelMat->scale,1,1,1);
    gfc_vector3d_set(modelMat->scaleDelta,1,1,1);
    return modelMat;
}

MaterialUBO gf3d_model_get_material_ubo(
    GFC_Vector4D baseColorMod,
    float shininess,
    GFC_Vector3D specular,
    GFC_Vector4D colorMods[MAX_TEXTURE_LAYERS])
{
    int i;
    GFC_Vector3D cameraPosition;
    MaterialUBO materialUBO = {0};
    
    gfc_vector4d_copy(materialUBO.baseColorMod,baseColorMod);
    materialUBO.shininess = shininess;
    for (i = 0;i < MAX_TEXTURE_LAYERS;i++)
    {
        gfc_vector4d_copy(materialUBO.colorMods[i],colorMods[i]);
    }
    cameraPosition = gf3d_camera_get_position();
    gfc_vector3d_copy(materialUBO.cameraPosition,cameraPosition);
    materialUBO.cameraPosition.w = 1;
    

    return materialUBO;
}

MeshUBO gf3d_model_get_mesh_ubo(GFC_Matrix4 modelMat)
{
    UniformBufferObject graphics_ubo;
    MeshUBO modelUBO = {0};
    
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    gfc_matrix4_copy(modelUBO.model,modelMat);
    gfc_matrix4_copy(modelUBO.view,graphics_ubo.view);
    gfc_matrix4_copy(modelUBO.proj,graphics_ubo.proj);

    return modelUBO;
}

MeshUBO gf3d_model_get_sky_ubo(GFC_Matrix4 modelMat)
{
    UniformBufferObject graphics_ubo;
    MeshUBO modelUBO;
    
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    
    gfc_matrix4_copy(modelUBO.model,modelMat);
    gfc_matrix4_copy(modelUBO.view,graphics_ubo.view);
     modelUBO.view[0][3] = 0;
     modelUBO.view[1][3] = 0;//zero out translations so the sky is always a fixed distance away
     modelUBO.view[2][3] = 0;
     modelUBO.view[3][0] = 0;
     modelUBO.view[3][1] = 0;
     modelUBO.view[3][2] = 0;
    gfc_matrix4_copy(modelUBO.proj,graphics_ubo.proj);
    return modelUBO;
}

MeshUBO gf3d_model_get_highlight_ubo(
    GFC_Matrix4 modelMat)
{
    UniformBufferObject graphics_ubo;
    MeshUBO modelUBO = {0};
    
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    
    gfc_matrix4_copy(modelUBO.model,modelMat);
    gfc_matrix4_copy(modelUBO.view,graphics_ubo.view);
    gfc_matrix4_copy(modelUBO.proj,graphics_ubo.proj);
    
    return modelUBO;
}

void gf3d_model_draw_all_meshes(
    Model *model,
    GFC_Matrix4 modelMat,
    MaterialUBO materialUbo,
    GF3D_Light_UBO lightUbo,
    Uint32 frame)
{
    int i,c;
    if (!model)return;
    c = gfc_list_get_count(model->mesh_list);
    for (i = 0;i < c; i++)
    {
        gf3d_model_draw(
            model,
            i,
            modelMat,
            materialUbo,
            lightUbo,
            frame);
    }
}

void gf3d_model_draw(
    Model *model,
    Uint32 index,
    GFC_Matrix4 modelMat,
    MaterialUBO materialUbo,
    GF3D_Light_UBO lightUbo,
    Uint32 frame)
{
    PipelineTuple *tuple;
    GFC_List *uboList;
    GFC_List *textureList = NULL;

    Mesh *mesh;
    MeshUBO uboData = {0};
    ArmatureUBO armatureUbo= {0};
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    uboData = gf3d_model_get_mesh_ubo(modelMat);    
    
    uboList = gfc_list_new();

    //MeshUBO
    tuple = gf3d_pipeline_tuple_new();
    tuple->data = &uboData;
    tuple->index = 0;
    tuple->uboIndex = 0;
    gfc_list_append(uboList,tuple);
    
    //materials
    tuple = gf3d_pipeline_tuple_new();
    tuple->data = &materialUbo;
    tuple->index = 1;
    tuple->uboIndex = 1;
    gfc_list_append(uboList,tuple);
    
    if (model->armature)
    {
        armatureUbo = gf3d_armature_get_ubo(model->armature,frame);
        uboData.flags.x = 1;
        tuple = gf3d_pipeline_tuple_new();
        tuple->data = &armatureUbo;
        tuple->index = 2;
        tuple->uboIndex = 2;
        gfc_list_append(uboList,tuple);
    }

    //lights
    tuple = gf3d_pipeline_tuple_new();
    tuple->data = &lightUbo;
    tuple->index = 3;
    tuple->uboIndex = 3;
    gfc_list_append(uboList,tuple);
    
    textureList = gfc_list_new();
    tuple = gf3d_pipeline_tuple_new();
    tuple->index = 4;//the start of the textures in the shaders
    if (model->texture)tuple->data = model->texture;
    else tuple->data = gf3d_model.defaultTexture;
    gfc_list_append(textureList,tuple);
    
    //TODO: support the rest of texture layers & normalMap, specular, and emit

    // queue up a render for batch rendering
    mesh = gfc_list_get_nth(model->mesh_list,index);
    if (!mesh)return;
    gf3d_mesh_queue_render(
        mesh,
        gf3d_model.pipe,
        uboList,
        textureList);
    
    //gf3d_mesh_queue_render(mesh,gf3d_model.pipe,&uboData,texture);
    //TODO: re-support the alpha
    //gf3d_mesh_queue_render(mesh,gf3d_mesh_get_alpha_pipeline(),&uboData,texture);
}

void gf3d_model_draw_highlight(Model *model,Uint32 index,GFC_Matrix4 modelMat,GFC_Vector4D highlight)
{
    Mesh *mesh;
    PipelineTuple *tuple;
    GFC_List *uboList;
    MeshUBO uboData = {0};
    
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    uboData = gf3d_model_get_highlight_ubo(modelMat);
    
    uboList = gfc_list_new();

    tuple = gf3d_pipeline_tuple_new();
    tuple->data = &uboData;
    tuple->index = 0;
    tuple->uboIndex = 0;
    gfc_list_append(uboList,tuple);


    mesh = gfc_list_get_nth(model->mesh_list,index);
    
    gf3d_mesh_queue_render(
        mesh,
        gf3d_mesh_get_highlight_pipeline(),
        uboList,
        NULL);

}

void gf3d_model_draw_sky(Model *model,GFC_Matrix4 modelMat,GFC_Color color)
{
    Mesh *mesh;
    PipelineTuple *tuple;
    GFC_List *uboList;
    GFC_List *textureList = NULL;
    MeshUBO uboData = {0};
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    uboData = gf3d_model_get_sky_ubo(modelMat);
    
    uboList = gfc_list_new();

    //basic mesh ubo
    tuple = gf3d_pipeline_tuple_new();
    tuple->data = &uboData;
    tuple->index = 0;
    tuple->uboIndex = 0;
    gfc_list_append(uboList,tuple);
    
    //sky texture
    textureList = gfc_list_new();
    tuple = gf3d_pipeline_tuple_new();
    tuple->index = 1;//the start of the textures in the shaders
    if (model->texture)tuple->data = model->texture;
    else tuple->data = gf3d_model.defaultTexture;
    gfc_list_append(textureList,tuple);
    
    mesh = gfc_list_get_nth(model->mesh_list,0);

    gf3d_mesh_queue_render(
        mesh,
        gf3d_mesh_get_sky_pipeline(),
        uboList,
        textureList);

}

/*eol@eof*/
