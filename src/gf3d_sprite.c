#include "simple_logger.h"

#include "gfc_types.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf3d_sprite.h"

#define SPRITE_ATTRIBUTE_COUNT 2

typedef struct
{
    Matrix4     model;          /**<how the sprite should be transformed each frame*/
    Vector2D    frame_offset;   /**<where to sample the texture from this frame*/
}SpriteUBO;

typedef struct
{
    Vector2D vertex;
    Vector2D texel;
}SpriteVertex;

typedef struct
{
    Uint32  verts[3];
}SpriteFace;

typedef struct
{
    Sprite         *sprite_list;      /**<pre-allocated space for sprites*/
    Uint32          max_sprites;      /**<maximum concurrent sprites supported*/
    Uint32          chain_length;     /**<length of swap chain*/
    VkDevice        device;           /**<logical vulkan device*/
    Pipeline       *pipe;             /**<the pipeline associated with sprite rendering*/
    VkBuffer        faceBuffer;       /**<memory handle for the face buffer (always two faces)*/
    VkDeviceMemory  faceBufferMemory; /**<memory habdle for tge face memory*/
    VkVertexInputAttributeDescription   attributeDescriptions[SPRITE_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
    Command                            *stagingCommandBuffer;
}SpriteManager;

void gf3d_sprite_update_basic_descriptor_set(Sprite *model,VkDescriptorSet descriptorSet,Uint32 chainIndex,Matrix4 modelMat,Uint32 frame);
void gf3d_sprite_create_uniform_buffer(Sprite *sprite);
void gf3d_sprite_create_vertex_buffer(Sprite *sprite);
void gf3d_sprite_delete(Sprite *sprite);

static SpriteManager gf3d_sprite = {0};


void gf3d_sprite_manager_close()
{
    int i;
    for (i = 0; i < gf3d_sprite.max_sprites;i++)
    {
        gf3d_sprite_delete(&gf3d_sprite.sprite_list[i]);
    }
    if (gf3d_sprite.sprite_list)
    {
        free(gf3d_sprite.sprite_list);
    }
    if (gf3d_sprite.faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_sprite.device, gf3d_sprite.faceBuffer, NULL);
        slog("sprite manager face buffer freed");
    }
    if (gf3d_sprite.faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_sprite.device, gf3d_sprite.faceBufferMemory, NULL);
        slog("sprite manager face buffer memory freed");
    }

    memset(&gf3d_sprite,0,sizeof(SpriteManager));
    slog("sprite manager closed");
}

void gf3d_sprite_manager_init(Uint32 max_sprites,Uint32 chain_length,VkDevice device)
{
    void* data;
    SpriteFace faces[2];
    size_t bufferSize;    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (max_sprites == 0)
    {
        slog("cannot intilizat sprite manager for 0 sprites");
        return;
    }
    gf3d_sprite.chain_length = chain_length;
    gf3d_sprite.sprite_list = (Sprite *)gfc_allocate_array(sizeof(Sprite),max_sprites);
    gf3d_sprite.max_sprites = max_sprites;
    gf3d_sprite.device = device;
    gf3d_sprite.pipe = gf3d_vgraphics_get_graphics_overlay_pipeline();
    
    // setup the face buffer, which will be used for ALL sprites
    faces[0].verts[0] = 2;
    faces[0].verts[1] = 1;
    faces[0].verts[2] = 0;
    faces[1].verts[0] = 1;
    faces[1].verts[1] = 3;
    faces[1].verts[2] = 2;


    bufferSize = sizeof(SpriteFace) * 2;
    
    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, faces, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gf3d_sprite.faceBuffer, &gf3d_sprite.faceBufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, gf3d_sprite.faceBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    
    slog("sprite manager initiliazed");
    atexit(gf3d_sprite_manager_close);
}

Sprite *gf3d_sprite_get_by_filename(char *filename)
{
    int i;
    for (i = 0; i < gf3d_sprite.max_sprites; i++)
    {
        if (!gf3d_sprite.sprite_list[i]._inuse)continue;
        if (gfc_line_cmp(gf3d_sprite.sprite_list[i].filename,filename) == 0)
        {
            return &gf3d_sprite.sprite_list[i];
        }
    }
    return NULL;
}

Sprite *gf3d_sprite_new()
{
    int i;
    for (i = 0; i < gf3d_sprite.max_sprites; i++)
    {
        if (gf3d_sprite.sprite_list[i]._inuse)continue;
        gf3d_sprite.sprite_list[i]._inuse = 1;
        gf3d_sprite_create_uniform_buffer(&gf3d_sprite.sprite_list[i]);
        return &gf3d_sprite.sprite_list[i];
    }
    slog("gf3d_sprite_new: no free slots for new sprites");
    return NULL;
}

Sprite * gf3d_sprite_load(char * filename,int frame_width,int frame_height, Uint32 frames_per_line)
{
    Sprite *sprite;
    sprite = gf3d_sprite_get_by_filename(filename);
    if (sprite)
    {
        sprite->_inuse++;
        return sprite;
    }
    sprite = gf3d_sprite_new();
    if (!sprite)
    {
        return NULL;
    }
    sprite->texture = gf3d_texture_load(filename);
    if (!sprite->texture)
    {
        slog("gf3d_sprite_load: failed to load texture for sprite");
        gf3d_sprite_free(sprite);
        return NULL;
    }
    if (frame_width < 0)frame_width = sprite->texture->width;
    if (frame_height < 0)frame_height = sprite->texture->height;
    sprite->frameWidth = frame_width;
    sprite->frameHeight = frame_height;
    if (frames_per_line)sprite->framesPerLine = frames_per_line;
    else sprite->framesPerLine = 1;
    gfc_line_cpy(sprite->filename,filename);
    gf3d_sprite_create_vertex_buffer(sprite);
    return sprite;
}

void gf3d_sprite_free(Sprite *sprite)
{
    if (!sprite)return;
    sprite->_inuse--;
    if (sprite->_inuse <= 0)gf3d_sprite_delete(sprite);
}

void gf3d_sprite_delete(Sprite *sprite)
{
    int i;
    if (!sprite)return;
    
    for (i = 0; i < sprite->uniformBufferCount; i++)
    {
        vkDestroyBuffer(gf3d_sprite.device, sprite->uniformBuffers[i], NULL);
        vkFreeMemory(gf3d_sprite.device, sprite->uniformBuffersMemory[i], NULL);
    }
    if (sprite->buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_sprite.device, sprite->buffer, NULL);
        slog("sprite %s vert buffer freed",sprite->filename);
    }
    if (sprite->bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_sprite.device, sprite->bufferMemory, NULL);
        slog("sprite %s vert buffer memory freed",sprite->filename);
    }

    gf3d_texture_free(sprite->texture);
    memset(sprite,0,sizeof(Sprite));
}


void gf3d_sprite_render(Sprite *sprite,VkCommandBuffer commandBuffer, VkDescriptorSet * descriptorSet)
{
    VkDeviceSize offsets[] = {0};
    Pipeline *pipe;
    if (!sprite)
    {
        slog("cannot render a NULL sprite");
        return;
    }
    pipe = gf3d_vgraphics_get_graphics_overlay_pipeline();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &sprite->buffer, offsets);
    
    vkCmdBindIndexBuffer(commandBuffer, gf3d_sprite.faceBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe->pipelineLayout, 0, 1, descriptorSet, 0, NULL);
    
    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}


void gf3d_sprite_draw(Sprite *sprite,Vector2D position,Vector2D scale,Uint32 frame)
{
    VkDescriptorSet *descriptorSet = NULL;
    Matrix4 modelMat;
    Uint32 buffer_frame;
    VkCommandBuffer commandBuffer;
    VkExtent2D extent = gf3d_vgraphics_get_view_extent();

    if (!sprite)
    {
        slog("cannot render a NULL sprite");
        return;
    }
    commandBuffer = gf3d_vgraphics_get_current_command_overlay_buffer();
    buffer_frame = gf3d_vgraphics_get_current_buffer_frame();

    descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_sprite.pipe, buffer_frame);
    if (descriptorSet == NULL)
    {
        slog("failed to get a free descriptor Set for sprite rendering");
        return;
    }
    gfc_matrix_scale(
        modelMat,
        vector3d(scale.x,scale.y,1));
    gfc_matrix_make_translation(
        modelMat,
        vector3d(position.x*2/(float)extent.width,position.y*2/(float)extent.height,0));

    gf3d_sprite_update_basic_descriptor_set(sprite,*descriptorSet,buffer_frame,modelMat,frame);
    gf3d_sprite_render(sprite,commandBuffer,descriptorSet);
}

void gf3d_sprite_create_vertex_buffer(Sprite *sprite)
{
    void *data = NULL;
    VkDevice device = gf3d_sprite.device;
    size_t bufferSize;
    VkExtent2D extent = gf3d_vgraphics_get_view_extent();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    SpriteVertex vertices[] = {
        {
            {-1,-1},
            {0,0}
        },
        {
            {-1+sprite->frameWidth/(float)extent.width,-1},
            {sprite->frameWidth/(float)sprite->texture->width,0}
        },
        {
            {-1,-1+sprite->frameHeight/(float)extent.height},
            {0,sprite->frameHeight/(float)sprite->texture->height}
        },
        {
            {-1+sprite->frameWidth/(float)extent.width,-1+sprite->frameHeight/(float)extent.height},
            {sprite->frameWidth/(float)sprite->texture->width,sprite->frameHeight/(float)sprite->texture->height}
        }
    };
    bufferSize = sizeof(SpriteVertex) * 4;
    
    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &sprite->buffer, &sprite->bufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, sprite->buffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);    
}

void gf3d_sprite_update_uniform_buffer(Sprite *sprite,uint32_t currentImage,Matrix4 modelMat,Uint32 frame)
{
    void* data;
    SpriteUBO ubo;
    gfc_matrix_copy(ubo.model,modelMat);
    ubo.frame_offset.x = (frame%sprite->framesPerLine * sprite->frameWidth)/(float)sprite->texture->width;
    ubo.frame_offset.y = (frame/sprite->framesPerLine * sprite->frameHeight)/(float)sprite->texture->height;
    vkMapMemory(gf3d_sprite.device, sprite->uniformBuffersMemory[currentImage], 0, sizeof(SpriteUBO), 0, &data);
    
        memcpy(data, &ubo, sizeof(SpriteUBO));

    vkUnmapMemory(gf3d_sprite.device, sprite->uniformBuffersMemory[currentImage]);
}

void gf3d_sprite_update_basic_descriptor_set(Sprite *sprite,VkDescriptorSet descriptorSet,Uint32 chainIndex,Matrix4 modelMat,Uint32 frame)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};

    if (!sprite)
    {
        slog("no sprite provided for descriptor set update");
        return;
    }
    if (descriptorSet == VK_NULL_HANDLE)
    {
        slog("null handle provided for descriptorSet");
        return;
    }

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = sprite->texture->textureImageView;
    imageInfo.sampler = sprite->texture->textureSampler;

    gf3d_sprite_update_uniform_buffer(sprite,chainIndex,modelMat,frame);
    bufferInfo.buffer = sprite->uniformBuffers[chainIndex];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(SpriteUBO);        
    
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

    vkUpdateDescriptorSets(gf3d_sprite.device, 2, descriptorWrite, 0, NULL);
}

VkVertexInputBindingDescription * gf3d_sprite_get_bind_description()
{
    gf3d_sprite.bindingDescription.binding = 0;
    gf3d_sprite.bindingDescription.stride = sizeof(SpriteVertex);
    gf3d_sprite.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return &gf3d_sprite.bindingDescription;
}

VkVertexInputAttributeDescription * gf3d_sprite_get_attribute_descriptions(Uint32 *count)
{
    gf3d_sprite.attributeDescriptions[0].binding = 0;
    gf3d_sprite.attributeDescriptions[0].location = 0;
    gf3d_sprite.attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_sprite.attributeDescriptions[0].offset = offsetof(SpriteVertex, vertex);
    
    gf3d_sprite.attributeDescriptions[1].binding = 0;
    gf3d_sprite.attributeDescriptions[1].location = 1;
    gf3d_sprite.attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_sprite.attributeDescriptions[1].offset = offsetof(SpriteVertex, texel);
    if (count)*count = SPRITE_ATTRIBUTE_COUNT;
    return gf3d_sprite.attributeDescriptions;
}


void gf3d_sprite_create_uniform_buffer(Sprite *sprite)
{
    int i;
    Uint32 buffercount = gf3d_sprite.chain_length;
    VkDeviceSize bufferSize = sizeof(SpriteUBO);

    if (!sprite)
    {
        slog("Failed to provide valid sprite pointer");
        return;
    }
    sprite->uniformBuffers = (VkBuffer*)gfc_allocate_array(sizeof(VkBuffer),buffercount);
    sprite->uniformBuffersMemory = (VkDeviceMemory*)gfc_allocate_array(sizeof(VkDeviceMemory),buffercount);
    sprite->uniformBufferCount = buffercount;

    for (i = 0; i < buffercount; i++)
    {
        gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &sprite->uniformBuffers[i], &sprite->uniformBuffersMemory[i]);
    }
}

/*eol@eof*/
