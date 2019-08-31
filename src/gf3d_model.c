#include <assert.h>

#include "simple_logger.h"

#include "gf3d_model.h"
#include "gf3d_commands.h"
#include "gf3d_vgraphics.h"
#include "gf3d_obj_load.h"

typedef struct
{
    Model               *   model_list;
    Uint32                  max_models;
    Uint32                  chain_length;   /**<length of swap chain*/
    VkDevice                device;
    Pipeline            *   pipe;           /**<the pipeline associated with model rendering*/
}ModelManager;

static ModelManager gf3d_model = {0};

void gf3d_model_delete(Model *model);

void gf3d_model_create_uniform_buffer(Model *model);
void gf3d_model_create_descriptor_pool(Model *model);
void gf3d_model_create_descriptor_sets(Model *model);
void gf3d_model_create_descriptor_set_layout();
void gf3d_model_update_uniform_buffer(Model *model,uint32_t currentImage,Matrix4 modelMat);
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

void gf3d_model_manager_init(Uint32 max_models,Uint32 chain_length,VkDevice device)
{
    if (max_models == 0)
    {
        slog("cannot intilizat model manager for 0 models");
        return;
    }
    gf3d_model.chain_length = chain_length;
    gf3d_model.model_list = (Model *)gfc_allocate_array(sizeof(Model),max_models);
    gf3d_model.max_models = max_models;
    gf3d_model.device = device;
    gf3d_model.pipe = gf3d_vgraphics_get_graphics_pipeline();
    
    slog("model manager initiliazed");
    atexit(gf3d_model_manager_close);
}

Model * gf3d_model_new()
{
    int i;
    for (i = 0; i < gf3d_model.max_models;i++)
    {
        if (!gf3d_model.model_list[i]._inuse)
        {
            gf3d_model_delete(&gf3d_model.model_list[i]);
            gf3d_model.model_list[i]._inuse = 1;
            gf3d_model_create_uniform_buffer(&gf3d_model.model_list[i]);
            return &gf3d_model.model_list[i];
        }
    }
    slog("unable to make a new model, out of space");
    return NULL;
}

Model * gf3d_model_load(char * filename)
{
    TextLine assetname;
    Model *model;
    model = gf3d_model_new();
    if (!model)return NULL;
    snprintf(assetname,GFCLINELEN,"models/%s.obj",filename);
    model->mesh = gf3d_mesh_load(assetname);

    snprintf(assetname,GFCLINELEN,"images/%s.png",filename);
    model->texture = gf3d_texture_load(assetname);
    
    return model;
}

void gf3d_model_free(Model *model)
{
    gf3d_model_delete(model);
}

void gf3d_model_delete(Model *model)
{
    int i;
    if (!model)return;
    
    for (i = 0; i < model->uniformBufferCount; i++)
    {
        vkDestroyBuffer(gf3d_model.device, model->uniformBuffers[i], NULL);
        vkFreeMemory(gf3d_model.device, model->uniformBuffersMemory[i], NULL);
    }

    gf3d_mesh_free(model->mesh);
    gf3d_texture_free(model->texture);
}

void gf3d_model_draw(Model *model,Uint32 bufferFrame, VkCommandBuffer commandBuffer,Matrix4 modelMat)
{
    VkDescriptorSet *descriptorSet = NULL;
    if (!model)
    {
        slog("cannot render a NULL model");
        return;
    }
    descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_model.pipe, bufferFrame);
    if (descriptorSet == NULL)
    {
        slog("failed to get a free descriptor Set for model rendering");
        return;
    }
    gf3d_model_update_basic_model_descriptor_set(model,*descriptorSet,bufferFrame,modelMat);
    gf3d_mesh_render(model->mesh,commandBuffer,descriptorSet);
}

void gf3d_model_update_basic_model_descriptor_set(Model *model,VkDescriptorSet descriptorSet,Uint32 chainIndex,Matrix4 modelMat)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};

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

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = model->texture->textureImageView;
    imageInfo.sampler = model->texture->textureSampler;

    gf3d_model_update_uniform_buffer(model,chainIndex,modelMat);
    bufferInfo.buffer = model->uniformBuffers[chainIndex];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);        
    
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

void gf3d_model_update_uniform_buffer(Model *model,uint32_t currentImage,Matrix4 modelMat)
{
    void* data;
    UniformBufferObject ubo;
    ubo = gf3d_vgraphics_get_uniform_buffer_object();
    gfc_matrix_copy(ubo.model,modelMat);
    vkMapMemory(gf3d_model.device, model->uniformBuffersMemory[currentImage], 0, sizeof(UniformBufferObject), 0, &data);
    
        memcpy(data, &ubo, sizeof(UniformBufferObject));

    vkUnmapMemory(gf3d_model.device, model->uniformBuffersMemory[currentImage]);
}


void gf3d_model_create_uniform_buffer(Model *model)
{
    int i;
    Uint32 buffercount = gf3d_model.chain_length;
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    model->uniformBuffers = (VkBuffer*)gfc_allocate_array(sizeof(VkBuffer),buffercount);
    model->uniformBuffersMemory = (VkDeviceMemory*)gfc_allocate_array(sizeof(VkDeviceMemory),buffercount);
    model->uniformBufferCount = buffercount;

    for (i = 0; i < buffercount; i++)
    {
        gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &model->uniformBuffers[i], &model->uniformBuffersMemory[i]);
    }
}

/*eol@eof*/
