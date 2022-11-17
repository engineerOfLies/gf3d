#include <assert.h>

#include "simple_logger.h"

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

typedef struct
{
    Model               *   model_list;
    Uint32                  max_models;
    Uint32                  chain_length;   /**<length of swap chain*/
    VkDevice                device;
    Pipeline            *   pipe;           /**<the pipeline associated with model rendering*/
    Texture             *   defaultTexture; /**<if a model has no texture, use this one*/

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
void gf3d_model_update_highlight_uniform_buffer(
    Model *model,
    UniformBuffer *ubo,
    Matrix4 modelMat,
    Vector4D highlightColor);

void gf3d_model_update_highlight_model_descriptor_set(
    Model *model,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex,
    Matrix4 modelMat,
    Vector4D highlightColor);
void gf3d_model_update_basic_model_descriptor_set(
    Model *model,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex,
    Matrix4 modelMat,
    Vector4D colorMod,
    Vector4D ambientLight);


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
    slog("model manager closed");
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
    
    slog("model manager initiliazed");
    atexit(gf3d_model_manager_close);
}

Model * gf3d_model_get_by_filename(const char *filename)
{
    int i;
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

Model * gf3d_model_load(const char * filename)
{    
    SJson *json,*config;
    Model *model;
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
    model = gf3d_model_load_from_config(config);
    if (model)
    {
        gfc_line_cpy(model->filename,filename);
    }
    sj_free(json);
    return model;
}

Model * gf3d_model_load_from_config(SJson *json)
{
    int i,c;
    Mesh *mesh;
    SJson *array,*item;
    Model *model;
    const char *modelFile;
    const char *textureFile;
    if (!json)return NULL;

    modelFile = sj_get_string_value(sj_object_get_value(json,"gltf"));
    if (modelFile)
    {
        slog("parsing gltf for model file");
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
    model->refCount--;
    if (!model->refCount)
    {
        gf3d_model_delete(model);
    }
}

void gf3d_model_delete(Model *model)
{
    int i,c;
    Mesh *mesh;
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


void gf3d_model_draw(Model *model,Uint32 index,Matrix4 modelMat,Vector4D colorMod,Vector4D ambientLight)
{
    Mesh *mesh;
    VkDescriptorSet *descriptorSet = NULL;
    VkCommandBuffer commandBuffer;
    Uint32 bufferFrame;
    if (!model)
    {
        slog("no model provided to draw");
        return;
    }
    commandBuffer = gf3d_mesh_get_model_command_buffer();
    bufferFrame = gf3d_vgraphics_get_current_buffer_frame();
    descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_model.pipe, bufferFrame);
    if (descriptorSet == NULL)
    {
        slog("failed to get a free descriptor Set for model rendering");
        return;
    }
    gf3d_model_update_basic_model_descriptor_set(model,*descriptorSet,bufferFrame,modelMat,colorMod,ambientLight);
    mesh = gfc_list_get_nth(model->mesh_list,index);
    if (!mesh)
    {
        slog("no mesh for index %i for model %s",index,model->filename);
        return;
    }
    gf3d_mesh_render(mesh,commandBuffer,descriptorSet);
}

void gf3d_model_draw_highlight(Model *model,Uint32 index,Matrix4 modelMat,Vector4D highlight)
{
    Mesh *mesh;
    VkDescriptorSet *descriptorSet = NULL;
    VkCommandBuffer commandBuffer;
    Uint32 bufferFrame;
    if (!model)
    {
        return;
    }
    commandBuffer = gf3d_mesh_get_highlight_command_buffer();
    bufferFrame = gf3d_vgraphics_get_current_buffer_frame();
    descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_mesh_get_highlight_pipeline(), bufferFrame);
    if (descriptorSet == NULL)
    {
        slog("failed to get a free descriptor Set for model rendering");
        return;
    }
    gf3d_model_update_highlight_model_descriptor_set(model,*descriptorSet,bufferFrame,modelMat,highlight);
    mesh = gfc_list_get_nth(model->mesh_list,index);
    gf3d_mesh_render_highlight(mesh,commandBuffer,descriptorSet);
}

void gf3d_model_update_sky_uniform_buffer(
    Model *model,
    UniformBuffer *ubo,
    Matrix4 modelMat,
    Vector4D colorMod)
{
    void* data;
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
        
    vkMapMemory(gf3d_model.device, ubo->uniformBufferMemory, 0, sizeof(MeshUBO), 0, &data);
    
        memcpy(data, &modelUBO, sizeof(SkyUBO));

    vkUnmapMemory(gf3d_model.device, ubo->uniformBufferMemory);
}

void gf3d_model_update_sky_model_descriptor_set(
    Model *model,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex,
    Matrix4 modelMat,
    Vector4D colorMod)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};
    Pipeline *pipe;
    UniformBuffer *ubo = NULL;
    
    if (!model)
    {
        slog("no model provided for descriptor set update");
        return;
    }
    if (descriptorSet == VK_NULL_HANDLE)
    {
        slog("null handle provided for descriptorSet");
        return;
    }
    pipe = gf3d_mesh_get_sky_pipeline();
    ubo = gf3d_uniform_buffer_list_get_buffer(pipe->uboList, chainIndex);
    if (!ubo)
    {
        slog("failed to get a free uniform buffer for draw call");
        return;
    }
    
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = model->texture->textureImageView;
    imageInfo.sampler = model->texture->textureSampler;

    gf3d_model_update_sky_uniform_buffer(model,ubo,modelMat,colorMod);
    bufferInfo.buffer = ubo->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(SkyUBO);        
    
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = descriptorSet;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &bufferInfo;

    descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[1].dstSet = descriptorSet;
    descriptorWrite[1].dstBinding = 1;
    descriptorWrite[1].dstArrayElement = 0;
    descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[1].descriptorCount = 1;                        
    descriptorWrite[1].pImageInfo = &imageInfo;
    descriptorWrite[1].pTexelBufferView = NULL; // Optional

    vkUpdateDescriptorSets(gf3d_model.device, 2, descriptorWrite, 0, NULL);
}

void gf3d_model_draw_sky(Model *model,Matrix4 modelMat,Color color)
{
    Mesh *mesh;
    VkDescriptorSet *descriptorSet = NULL;
    VkCommandBuffer commandBuffer;
    Uint32 bufferFrame;
    if (!model)
    {
        return;
    }
    commandBuffer = gf3d_mesh_get_sky_command_buffer();
    bufferFrame = gf3d_vgraphics_get_current_buffer_frame();
    descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_mesh_get_sky_pipeline(), bufferFrame);
    if (descriptorSet == NULL)
    {
        slog("failed to get a free descriptor Set for model rendering");
        return;
    }
    gf3d_model_update_sky_model_descriptor_set(model,*descriptorSet,bufferFrame,modelMat,gfc_color_to_vector4f(color));
    mesh = gfc_list_get_nth(model->mesh_list,0);
    gf3d_mesh_render_sky(mesh,commandBuffer,descriptorSet);
}


void gf3d_model_update_basic_model_descriptor_set(
    Model *model,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex,
    Matrix4 modelMat,
    Vector4D colorMod,
    Vector4D ambientLight)
{
    Texture *texture = NULL;
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};
    UniformBuffer *ubo = NULL;
    
    if (!model)
    {
        slog("no model provided for descriptor set update");
        return;
    }
    if (descriptorSet == VK_NULL_HANDLE)
    {
        slog("null handle provided for descriptorSet");
        return;
    }
    ubo = gf3d_uniform_buffer_list_get_buffer(gf3d_model.pipe->uboList, chainIndex);
    if (!ubo)
    {
        slog("failed to get a free uniform buffer for draw call");
        return;
    }
    
    if (model->texture)texture = model->texture;
    else texture = gf3d_model.defaultTexture;
    
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->textureImageView;
    imageInfo.sampler = texture->textureSampler;

    gf3d_model_update_uniform_buffer(model,ubo,modelMat,colorMod,ambientLight);
    bufferInfo.buffer = ubo->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(MeshUBO);        
    
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = descriptorSet;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &bufferInfo;

    descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[1].dstSet = descriptorSet;
    descriptorWrite[1].dstBinding = 1;
    descriptorWrite[1].dstArrayElement = 0;
    descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[1].descriptorCount = 1;                        
    descriptorWrite[1].pImageInfo = &imageInfo;
    descriptorWrite[1].pTexelBufferView = NULL; // Optional

    vkUpdateDescriptorSets(gf3d_model.device, 2, descriptorWrite, 0, NULL);
}

void gf3d_model_update_highlight_model_descriptor_set(
    Model *model,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex,
    Matrix4 modelMat,
    Vector4D highlightColor)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};
    Pipeline *pipe;
    UniformBuffer *ubo = NULL;
    
    if (!model)
    {
        slog("no model provided for descriptor set update");
        return;
    }
    if (descriptorSet == VK_NULL_HANDLE)
    {
        slog("null handle provided for descriptorSet");
        return;
    }
    pipe = gf3d_mesh_get_highlight_pipeline();
    ubo = gf3d_uniform_buffer_list_get_buffer(pipe->uboList, chainIndex);
    if (!ubo)
    {
        slog("failed to get a free uniform buffer for draw call");
        return;
    }
    
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = model->texture->textureImageView;
    imageInfo.sampler = model->texture->textureSampler;

    gf3d_model_update_highlight_uniform_buffer(model,ubo,modelMat,highlightColor);
    bufferInfo.buffer = ubo->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(HighlightUBO);        
    
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = descriptorSet;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &bufferInfo;

    descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[1].dstSet = descriptorSet;
    descriptorWrite[1].dstBinding = 1;
    descriptorWrite[1].dstArrayElement = 0;
    descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[1].descriptorCount = 1;                        
    descriptorWrite[1].pImageInfo = &imageInfo;
    descriptorWrite[1].pTexelBufferView = NULL; // Optional

    vkUpdateDescriptorSets(gf3d_model.device, 2, descriptorWrite, 0, NULL);
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

void gf3d_model_update_uniform_buffer(
    Model *model,
    UniformBuffer *ubo,
    Matrix4 modelMat,
    Vector4D colorMod,
    Vector4D ambient)
{
    void* data;
    Vector3D cameraPosition;
    UniformBufferObject graphics_ubo;
    MeshUBO modelUBO;
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    
    gfc_matrix_copy(modelUBO.model,modelMat);
    gfc_matrix_copy(modelUBO.view,graphics_ubo.view);
    gfc_matrix_copy(modelUBO.proj,graphics_ubo.proj);
    
    vector4d_copy(modelUBO.color,colorMod);
    
    
    cameraPosition = gf3d_camera_get_position();
    vector3d_copy(modelUBO.cameraPosition,cameraPosition);
    modelUBO.cameraPosition.w = 1;
    

    gf3d_lights_get_global_light(&modelUBO.ambientColor, &modelUBO.ambientDir);
    
    vector4d_scale_by(modelUBO.ambientColor,modelUBO.ambientColor,ambient);
        
    vkMapMemory(gf3d_model.device, ubo->uniformBufferMemory, 0, sizeof(MeshUBO), 0, &data);
    
        memcpy(data, &modelUBO, sizeof(MeshUBO));

    vkUnmapMemory(gf3d_model.device, ubo->uniformBufferMemory);
}

void gf3d_model_update_highlight_uniform_buffer(
    Model *model,
    UniformBuffer *ubo,
    Matrix4 modelMat,
    Vector4D highlightColor)
{
    void* data;
    UniformBufferObject graphics_ubo;
    HighlightUBO modelUBO;
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    
    gfc_matrix_copy(modelUBO.model,modelMat);
    gfc_matrix_copy(modelUBO.view,graphics_ubo.view);
    gfc_matrix_copy(modelUBO.proj,graphics_ubo.proj);
    
    vector4d_copy(modelUBO.color,highlightColor);
        
    vkMapMemory(gf3d_model.device, ubo->uniformBufferMemory, 0, sizeof(MeshUBO), 0, &data);
    
        memcpy(data, &modelUBO, sizeof(HighlightUBO));

    vkUnmapMemory(gf3d_model.device, ubo->uniformBufferMemory);
}

/*eol@eof*/
