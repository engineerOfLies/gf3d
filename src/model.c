#include "simple_logger.h"

#include "gfc_types.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf3d_obj_load.h"

#include "model.h"

#define MAX_MESHES 1024

extern int __DEBUG;

typedef struct
{
	Model* modelList;
    Uint32 modelCount;
    Uint32 chainLength;
    Pipeline* pipe;
    Pipeline* sky_pipe;
    VkDevice device;
    Texture* defaultTexture;
}ModelManager;

static ModelManager model_manager = { 0 };

/**
 * @brief initialize a new model in modelList
 * @param model_manager_index the index of the location in model_manager_manager modelList for the model to be initialized
 * @return NULL on error, or an empty model
 */
Model* model_initialize(Uint32 model_manager_index);

void model_delete(Model* model);

void model_system_close()
{
    int i;

    if (!model_manager.defaultTexture)
    {
        slog("Achievement unlocked: How did we get here?");
    }
    else
    {
        gf3d_texture_free(model_manager.defaultTexture);
    }
    if (model_manager.modelList)
    {
        for (i = 0; i < model_manager.modelCount; i++)
        {
            model_delete(&model_manager.modelList[i]);
        }
        free(model_manager.modelList);
    }

    memset(&model_manager, 0, sizeof(ModelManager));
    if (__DEBUG)slog("model manager closed");
}

void model_manager_init(Uint32 max_models)
{
    Uint32 count;
    
    if (model_manager.modelCount != 0)
    {
        slog("Cannot init model system, already initialized");
        return;
    }
    if (max_models == 0)
    {
        slog("Cannot initialize model system for 0 models");
        return;
    }
    model_manager.chainLength = gf3d_swapchain_get_chain_length();
    model_manager.modelCount = max_models;
    model_manager.device = gf3d_vgraphics_get_default_logical_device();
    model_manager.modelList = gfc_allocate_array(sizeof(Model), max_models);

    if (!model_manager.modelList)
    {
        slog("cannot allocate more memory for model_manager meshList");
        slog_sync();
        model_system_close();
        exit(-1);
        return;
    }
    gf3d_mesh_system_init(MAX_MESHES);

    gf3d_mesh_get_attribute_descriptions(&count);

    //sky pipeline
    model_manager.sky_pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/sky_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        10,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        count,
        sizeof(ModelUBO),
        VK_INDEX_TYPE_UINT16
    );

    model_manager.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/model_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        max_models,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        count,
        sizeof(ModelUBO),
        VK_INDEX_TYPE_UINT16
    );

    if (!model_manager.pipe)
    {
        slog("failed to make pipeline for models!");
        slog_sync();
        model_system_close();
        exit(-1);
        return;
    }
    if (!model_manager.sky_pipe)
    {
        slog("failed to make pipeline for models!");
        slog_sync();
        model_system_close();
        exit(-1);
        return;
    }
    model_manager.defaultTexture = gf3d_texture_load("images/default.png");
    if (!model_manager.defaultTexture)
    {
        slog("OH NOES, DEFAULT TEXTURE DIDNT'T EXIST. CRASH INCOMING...");
        slog_sync();
    }
    if (__DEBUG)slog("model manager initiliazed");
    atexit(model_system_close);
}

Model* model_get_by_filename(const char* filename)
{
    int i;
    if (!filename)
    {
        return NULL;
    }
    for (i = 0; i < model_manager.modelCount; i++)
    {
        if (model_manager.modelList[i]._refCount == 0 || strlen(model_manager.modelList[i].filename) == 0)
        {
            continue;
        }
        if (gfc_strlcmp(model_manager.modelList[i].filename, filename) == 0)
        {
            return &model_manager.modelList[i];
        }
    }
    return NULL;
}

Model* model_new()
{
    int i;
    int emptyIndex;
    int foundEmptyIndex = 0;
    for (i = 0; i < model_manager.modelCount; i++)
    {
        if (model_manager.modelList[i]._refCount == 0)
        {
            if (!foundEmptyIndex)
            {
                emptyIndex = i;
                foundEmptyIndex = 1;
            }
            if (strlen(model_manager.modelList[i].filename) == 0)
            {
                return model_initialize(i);
            }
        }
    }
    if (foundEmptyIndex)
    {
        if (strlen(model_manager.modelList[emptyIndex].filename) > 0)
        {
            model_delete(&model_manager.modelList[emptyIndex]);
        }
        return model_initialize(emptyIndex);
    }
    else 
    {
        slog("model_new: no free slots for new models");
        return NULL;
    }
}

Model* model_initialize(Uint32 model_manager_index)
{
    //boiler plate initilization goes here
    model_manager.modelList[model_manager_index]._refCount = 1;
    return &model_manager.modelList[model_manager_index];
}


Model* model_load(const char* filename)
{
    const char* str = NULL;
    const char* str2 = NULL;
    Mesh* mesh;
    Texture* texture;
    SJson* json, * data;
    Model* model;

    if (!filename)
    {
        return NULL;
    }
    model = model_get_by_filename(filename);
    if (model)
    {
        model->_refCount++;
        return model;
    }
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load model json %s", filename);
        return NULL;
    }
    data = sj_object_get_value(json, "model");
    if (!data)
    {
        slog("failed to get model information from file %s", filename);
        sj_free(json);
        return NULL;
    }
    str = sj_object_get_string(data, "obj");
    if (!str)
    {
        slog("failed to find obj data in file %s", filename);
        sj_free(json);
        return NULL;
    }
    slog("obj to load %s",str);
    slog_sync();
    mesh = gf3d_mesh_load_obj(str);
    slog("mesh_obj_loaded");
    slog_sync();
    if (!mesh)
    {
        slog("failed to parese obj data for model file %s", filename);
        gf3d_mesh_free(mesh);
        sj_free(json);
        return NULL;
    }
    str2 = sj_object_get_string(data, "texture");
    if (str2)
    {
        texture = gf3d_texture_load(str2);
        if (!texture)
        {
            texture = model_manager.defaultTexture;
        }
    }
    else
    {
        texture = model_manager.defaultTexture;

    }
    model = model_new();
    if (!model)
    {
        slog("failed to get an empty space in memory for model %s", filename);
        gf3d_texture_free(texture);
        gf3d_mesh_free(mesh);
        sj_free(json);
        return NULL;
    }
    model->mesh = mesh;
    model->texture = texture;
    gfc_line_cpy(model->filename, filename);
    return model;
}

void model_free(Model* model)
{
    if (!model)
    {
        return;
    }
    model->_refCount--;
    //can optionally delete models when ref count is 0. tradeoff: deleting makes it
    //so the loop in model_new guarenteed doesnt loop the whole thing. not deleting
    //it saves reloading things time
}

void model_delete(Model* model)
{
    if (!model)
    {
        return;
    }
    gf3d_texture_free(model->texture);
    gf3d_mesh_free(model->mesh);

    memset(model, 0, sizeof(Model));
}

Pipeline* model_get_pipeline()
{
    return model_manager.pipe;
}

ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod)
{
    ModelUBO ubo = { 0 };
    GFC_Matrix4* view;
    gfc_matrix4_copy(ubo.model, modelMat);
    gf3d_vgraphics_get_view(&ubo.view);
    gf3d_vgraphics_get_projection_matrix(&ubo.proj);
    ubo.color = gfc_color_to_vector4f(colorMod);
    return ubo;
}

void model_queue_render_sky(Model* model, GFC_Matrix4 mat, GFC_Color colorMod)
{
    ModelUBO ubo;
    ubo = model_get_ubo(mat, colorMod);
    slog_sync();

    gf3d_mesh_queue_render(model->mesh, model_manager.sky_pipe, &ubo, model->texture);
}

void model_queue_render(Model* model, GFC_Matrix4 mat, GFC_Color colorMod)
{
    ModelUBO ubo;
    ubo = model_get_ubo(mat, colorMod);
    slog_sync();

    gf3d_mesh_queue_render(model->mesh, model_manager.pipe, &ubo, model->texture);
}