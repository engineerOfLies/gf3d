#include <assert.h>

#include "simple_logger.h"

#include "gfc_config.h"

#include "gf2d_actor.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_commands.h"
#include "gf3d_vgraphics.h"
#include "gf3d_obj_load.h"
#include "gf3d_uniform_buffers.h"
#include "gf3d_materials.h"
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
            gfc_matrix4_identity(gf3d_model.model_list[i].matrix);
            gf3d_model.model_list[i].mesh_list = gfc_list_new();
            return &gf3d_model.model_list[i];
        }
    }
    slog("unable to make a new model, out of space");
    return NULL;
}

void gf3d_model_move(Model *in, GFC_Vector3D offset,GFC_Vector3D rotation)
{
    int i,c;
    Mesh *mesh;
    if (!in);
    c = gfc_list_get_count(in->mesh_list);
    for (i = 0; i < c; i++)
    {
        mesh = gfc_list_get_nth(in->mesh_list,i);
        if (!mesh)continue;
        gf3d_mesh_move_vertices(mesh, offset,rotation);
    }
}

void gf3d_model_append(Model *modelA,Model *modelB, GFC_Vector3D offsetB,GFC_Vector3D rotation)
{
    int i,c;
    Mesh *meshA,*meshB;
    if ((!modelA)||(!modelB))return;
    //only merge up to the minimum number of meshes between the two
    c = MIN(gfc_list_get_count(modelA->mesh_list),gfc_list_get_count(modelB->mesh_list));
    for (i = 0; i < c; i++)
    {
        meshA = gfc_list_get_nth(modelA->mesh_list,i);
        meshB = gfc_list_get_nth(modelB->mesh_list,i);
        if ((!meshA)||(!meshB))continue;
        gf3d_mesh_append(meshA, meshB, offsetB,rotation);
    }
}


Model *gf3d_model_copy(Model *in)
{
    int i,c;
    Mesh *mesh;
    Mesh *meshNew;
    Model *out;
    if (!in)
    {
        slog("no input to copy");
        return NULL;
    }
    out = gf3d_model_new();
    if (!out)
    {
        slog("failed to get a copy model");
        return NULL;
    }

    gfc_line_sprintf(out->filename,"%s.dup",in->filename);
    
    if (in->mesh_list)
    {
        out->mesh_list = gfc_list_new();
        c = gfc_list_get_count(in->mesh_list);
        for (i = 0; i < c; i++)
        {
            mesh = gfc_list_get_nth(in->mesh_list,i);
            if (!mesh)continue;
            meshNew = gf3d_mesh_copy(mesh);
            if (!meshNew)continue;
            gfc_list_append(out->mesh_list,meshNew);
        }
    }
    out->mesh_as_frame = in->mesh_as_frame;
    if (in->material)
    {
        out->material = in->material;       //if set, use this material when sending draw calls
        out->material->_refCount++;
    }
    
    if (in->texture)
    {
        out->texture = in->texture;       //if set, use this material when sending draw calls
        out->texture->_refcount++;
    }
    if (in->normalMap)
    {
        out->normalMap = in->normalMap;       //if set, use this material when sending draw calls
        out->normalMap->_refcount++;
    }
    
    memcpy(&out->bounds,&in->bounds,sizeof(GFC_Box));

    return out;
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
    json = sj_load(filename);
    if (!json)return NULL;
    
    config = sj_object_get_value(json,"model");
    if (!config)
    {
        slog("file %s contains no model object",filename);
        sj_free(json);
        return NULL;
    }
    model = gf3d_model_load_from_config(config,filename);
    sj_free(json);
    return model;
}

Model *gf3d_model_load_from_config(SJson *json,const char *filename)
{
    int i,c;
    Mesh *mesh;
    SJson *array,*item;
    Model *model;
    const char *modelFile;
    const char *textureFile;
    const char *materialFile;
    if (!gf3d_model.initiliazed)return NULL;
    if (!json)return NULL;

    modelFile = sj_get_string_value(sj_object_get_value(json,"gltf"));
    if (modelFile)
    {
        model = gf3d_gltf_parse_model(modelFile);
        if (!model)return NULL;
    }    
    else
    {
        model = gf3d_model_new();
        if (!model)return NULL;
        
        if (sj_object_get_value(json,"obj_list"))
        {
            array = sj_object_get_value(json,"obj_list");
            if (!array)
            {
                slog("model file %s obj_list missing objects");
                gf3d_model_free(model);
                return NULL;
            }
            model->mesh_as_frame = 1;
            c = sj_array_get_count(array);
            for (i = 0; i < c; i++)
            {
                item = sj_array_get_nth(array,i);
                if (!item)continue;
                modelFile = sj_get_string_value(item);
                if (modelFile)
                {
                    mesh = gf3d_mesh_load_obj(modelFile);
                    if (mesh)
                    {
                        gfc_box_cpy(model->bounds,mesh->bounds);
                        gfc_list_append(model->mesh_list,mesh);
                    }
                }
            }
        }
        else if (sj_get_string_value(sj_object_get_value(json,"obj")))
        {
            modelFile = sj_get_string_value(sj_object_get_value(json,"obj"));
            if (!modelFile)
            {
                slog("model file %s obj missing filename");
                gf3d_model_free(model);
                return NULL;                
            }
            mesh = gf3d_mesh_load_obj(modelFile);
            if (!mesh)
            {
                slog("failed to parse mesh data from obj file %s",modelFile);
                gf3d_model_free(model);
                return NULL;
            }
            gfc_box_cpy(model->bounds,mesh->bounds);
            gfc_list_append(model->mesh_list,mesh);
        }
        else
        {
            if (__DEBUG)slog("no known way to parse model file");
            gf3d_model_free(model);
            return NULL;
        }
    }
    
    if (filename)gfc_line_cpy(model->filename,filename);
    
    item = sj_object_get_value(json,"matrix");
    if (item)
    {
        sj_value_as_matrix4_vectors(item,model->matrix);
    }
        
    materialFile = sj_object_get_value_as_string(json,"materialFile");
    if (materialFile)
    {
        model->material = gf3d_material_load(materialFile);
    }
    else
    {
        item = sj_object_get_value(json,"material");
        if (item)
        {
            model->material = gf3d_material_parse_js(item,filename);
        }
    }

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
    return model;
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
    gf3d_material_free(model->material);
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

    mesh = gf3d_mesh_load_obj(modelFile);
    if (!mesh)
    {
        gf3d_model_free(model);
        return NULL;
    }
    gfc_box_cpy(model->bounds,mesh->bounds);
    gfc_list_append(model->mesh_list,mesh);
    
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
    gfc_vector3d_scale(mat->rotation,mat->rotation,GFC_DEGTORAD);//config file is in degrees
    sj_value_as_vector3d(sj_object_get_value(config,"scale"),&mat->scale);
}

ModelMat *gf3d_model_mat_new()
{
    ModelMat *modelMat;
    modelMat = gfc_allocate_array(sizeof(ModelMat),1);
    if (!modelMat)return NULL;
    gfc_matrix4_identity(modelMat->mat);
    gfc_vector3d_set(modelMat->scale,1,1,1);
    return modelMat;
}

MeshUBO gf3d_model_get_sky_ubo(
    GFC_Matrix4 modelMat,
    GFC_Vector4D colorMod)
{
    ModelViewProjection mvp;
    MeshUBO modelUBO;
    
    mvp = gf3d_vgraphics_get_mvp();
    
    gfc_matrix4_copy(modelUBO.model,modelMat);
    gfc_matrix4_copy(modelUBO.view,mvp.view);
     modelUBO.view[0][3] = 0;
     modelUBO.view[1][3] = 0;
     modelUBO.view[2][3] = 0;
     modelUBO.view[3][0] = 0;
     modelUBO.view[3][1] = 0;
     modelUBO.view[3][2] = 0;
    gfc_matrix4_copy(modelUBO.proj,mvp.proj);
    gfc_vector4d_copy(modelUBO.color,colorMod);
    return modelUBO;
}

void gf3d_model_draw_all_meshes(
    Model *model,
    GFC_Matrix4 modelMat,
    GFC_Color colorMod,
    Uint32 frame)
{
    int i,c;
    if (!model)return;
    c = gfc_list_get_count(model->mesh_list);
    for (i = 0;i < c; i++)
    {
        gf3d_model_draw_index(model,i,modelMat,colorMod,frame);
    }
}

void gf3d_model_draw(
    Model *model,
    GFC_Matrix4 modelMat,
    GFC_Color   colorMod,
    Uint32 frame)
{
    if (!model)return;
    if (model->mesh_as_frame)
    {
        gf3d_model_draw_index(
            model,
            frame,
            modelMat,
            colorMod,
            0);
        return;
    }
    gf3d_model_draw_all_meshes(
        model,
        modelMat,
        colorMod,
        frame);
}


void gf3d_model_draw_index(
    Model *model,
    Uint32 index,
    GFC_Matrix4 modelMat,
    GFC_Color   colorMod,
    Uint32 frame)
{
    Mesh *mesh;
    GFC_Matrix4 matrix = {0};
    GFC_Vector4D modColor = {0};
    ModelUBO uboData = {0};
    Texture *texture;
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    mesh = gfc_list_get_nth(model->mesh_list,index);
    if (!mesh)return;
    
    //factor in the matrix loaded from disk
    gfc_matrix4_multiply(matrix,model->matrix,modelMat);
    
    uboData.mesh = gf3d_mesh_get_ubo(matrix,colorMod);
    
    if (model->material)
    {
        uboData.material = gf3d_material_get_ubo(model->material);
        modColor = gfc_color_to_vector4f(colorMod);
        //modulate the diffuse color based on input colorMod
        gfc_vector4d_scale_by(uboData.material.diffuse,uboData.material.diffuse,modColor);
    }
    else uboData.material = gf3d_material_make_basic_ubo(colorMod);
    if (!model->texture)
    {
        texture = gf3d_model.defaultTexture;
    }
    else texture = model->texture;
    // queue up a render for batch rendering
    gf3d_mesh_queue_render(mesh,gf3d_model.pipe,&uboData,texture);
}

void gf3d_model_draw_sky(Model *model,GFC_Matrix4 modelMat,GFC_Color color)
{
    Mesh *mesh;
    GFC_Matrix4 matrix = {0};
    Texture *texture;
    MeshUBO uboData = {0};
    if (!gf3d_model.initiliazed)return;
    if (!model)return;
    gfc_matrix4_multiply(matrix,model->matrix,modelMat);
    uboData = gf3d_model_get_sky_ubo(matrix,gfc_color_to_vector4f(color));
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
