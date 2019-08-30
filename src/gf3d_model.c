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

void gf3d_model_create_descriptor_pool(Model *model);
void gf3d_model_create_descriptor_sets(Model *model);
void gf3d_model_create_descriptor_set_layout();
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
            gf3d_model.model_list[i]._inuse = 1;
            gf3d_model.model_list[i]._refcount = 1;
            return &gf3d_model.model_list[i];
        }
    }
    for (i = 0; i < gf3d_model.max_models;i++)
    {
        if (!gf3d_model.model_list[i]._refcount)
        {
            gf3d_model_delete(&gf3d_model.model_list[i]);
            gf3d_model.model_list[i]._inuse = 1;
            gf3d_model.model_list[i]._refcount = 1;
            return &gf3d_model.model_list[i];
        }
    }
    slog("unable to make a new model, out of space");
    return NULL;
}

Model *gf3d_model_get_by_filename(char *filename)
{
    int i;
    if (!filename)return NULL;
    for (i = 0; i < gf3d_model.max_models;i++)
    {
        if (!gf3d_model.model_list[i]._inuse)continue;
        if (gfc_line_cmp(filename,gf3d_model.model_list[i].filename)==0)
        {
            gf3d_model.model_list[i]._refcount++;
            return &gf3d_model.model_list[i];
        }
    }
    return NULL;
}

Model * gf3d_model_load(char * filename)
{
    TextLine assetname;
    Model *model;
    model = gf3d_model_get_by_filename(filename);
    if (model)return model;
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
    if ((!model)||(!model->_refcount))return;
    model->_refcount--;
}

void gf3d_model_delete(Model *model)
{
    if (!model)return;
    
    gf3d_mesh_free(model->mesh);
    gf3d_texture_free(model->texture);
}

void gf3d_model_draw(Model *model,Uint32 bufferFrame, VkCommandBuffer commandBuffer)
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
    gf3d_model_update_basic_model_descriptor_set(model,*descriptorSet,bufferFrame);
    gf3d_mesh_render(model->mesh,commandBuffer,descriptorSet);
}

void gf3d_model_update_basic_model_descriptor_set(Model *model,VkDescriptorSet descriptorSet,Uint32 chainIndex)
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

    slog("updating descriptor sets");
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = model->texture->textureImageView;
    imageInfo.sampler = model->texture->textureSampler;

    bufferInfo.buffer = gf3d_vgraphics_get_uniform_buffer_by_index(chainIndex);
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

/*eol@eof*/
