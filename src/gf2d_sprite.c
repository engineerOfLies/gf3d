#include "simple_logger.h"

#include "gfc_types.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf2d_sprite.h"

#define SPRITE_ATTRIBUTE_COUNT 2

typedef struct
{
    Matrix4 rotation;
    Vector4D colorMod;
    Vector2D size;
    Vector2D extent;
    Vector2D position;
    Vector2D scale;
    Vector2D frame_offset;
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
}SpriteManager;

void gf2d_sprite_update_basic_descriptor_set(
    Sprite *sprite,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex,
    Vector2D position,
    Vector2D scale,
    Vector3D rotation,
    Color color,
    Uint32 frame);
void gf2d_sprite_create_vertex_buffer(Sprite *sprite);
void gf2d_sprite_delete(Sprite *sprite);

static SpriteManager gf2d_sprite = {0};


void gf2d_sprite_manager_close()
{
    int i;
    for (i = 0; i < gf2d_sprite.max_sprites;i++)
    {
        gf2d_sprite_delete(&gf2d_sprite.sprite_list[i]);
    }
    if (gf2d_sprite.sprite_list)
    {
        free(gf2d_sprite.sprite_list);
    }
    if (gf2d_sprite.faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf2d_sprite.device, gf2d_sprite.faceBuffer, NULL);
        slog("sprite manager face buffer freed");
    }
    if (gf2d_sprite.faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf2d_sprite.device, gf2d_sprite.faceBufferMemory, NULL);
        slog("sprite manager face buffer memory freed");
    }

    memset(&gf2d_sprite,0,sizeof(SpriteManager));
    slog("sprite manager closed");
}

void gf2d_sprite_manager_init(Uint32 max_sprites)
{
    void* data;
    Uint32 count;
    SpriteFace faces[2];
    size_t bufferSize;    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (max_sprites == 0)
    {
        slog("cannot intilizat sprite manager for 0 sprites");
        return;
    }
    gf2d_sprite.chain_length = gf3d_swapchain_get_chain_length();
    gf2d_sprite.sprite_list = (Sprite *)gfc_allocate_array(sizeof(Sprite),max_sprites);
    gf2d_sprite.max_sprites = max_sprites;
    gf2d_sprite.device = gf3d_vgraphics_get_default_logical_device();
    
    // setup the face buffer, which will be used for ALL sprites
    faces[0].verts[0] = 2;
    faces[0].verts[1] = 1;
    faces[0].verts[2] = 0;
    faces[1].verts[0] = 1;
    faces[1].verts[1] = 3;
    faces[1].verts[2] = 2;

    bufferSize = sizeof(SpriteFace) * 2;
    
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(gf2d_sprite.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, faces, (size_t) bufferSize);
    vkUnmapMemory(gf2d_sprite.device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gf2d_sprite.faceBuffer, &gf2d_sprite.faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, gf2d_sprite.faceBuffer, bufferSize);

    vkDestroyBuffer(gf2d_sprite.device, stagingBuffer, NULL);
    vkFreeMemory(gf2d_sprite.device, stagingBufferMemory, NULL);

    gf2d_sprite_get_attribute_descriptions(&count);
    gf2d_sprite.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/overlay_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        max_sprites,
        gf2d_sprite_get_bind_description(),
        gf2d_sprite_get_attribute_descriptions(NULL),
        count,
        sizeof(SpriteUBO)
    );     
    
    slog("sprite manager initiliazed");
    atexit(gf2d_sprite_manager_close);
}

Sprite *gf2d_sprite_get_by_filename(char *filename)
{
    int i;
    for (i = 0; i < gf2d_sprite.max_sprites; i++)
    {
        if (!gf2d_sprite.sprite_list[i]._inuse)continue;
        if (gfc_line_cmp(gf2d_sprite.sprite_list[i].filename,filename) == 0)
        {
            return &gf2d_sprite.sprite_list[i];
        }
    }
    return NULL;
}

Sprite *gf2d_sprite_new()
{
    int i;
    for (i = 0; i < gf2d_sprite.max_sprites; i++)
    {
        if (gf2d_sprite.sprite_list[i]._inuse)continue;
        gf2d_sprite.sprite_list[i]._inuse = 1;
        return &gf2d_sprite.sprite_list[i];
    }
    slog("gf2d_sprite_new: no free slots for new sprites");
    return NULL;
}

Sprite * gf2d_sprite_from_surface(SDL_Surface *surface,int frame_width,int frame_height, Uint32 frames_per_line)
{
    Sprite *sprite = NULL;
    if (!surface)
    {
        slog("no surface provided to convert to a sprite");
        return NULL;
    }
    sprite = gf2d_sprite_new();
    if (!sprite)
    {
        return NULL;
    }
    sprite->texture = gf3d_texture_convert_surface(surface);
    if (!sprite->texture)
    {
        gf2d_sprite_free(sprite);
        return NULL;
    }
    if (frame_width <= 0)frame_width = sprite->texture->width;
    if (frame_height <= 0)frame_height = sprite->texture->height;
    sprite->frameWidth = frame_width;
    sprite->frameHeight = frame_height;
    if (frames_per_line)sprite->framesPerLine = frames_per_line;
    else sprite->framesPerLine = 1;
    gf2d_sprite_create_vertex_buffer(sprite);
    return sprite;
}


Sprite * gf2d_sprite_load(char * filename,int frame_width,int frame_height, Uint32 frames_per_line)
{
    Sprite *sprite;
    sprite = gf2d_sprite_get_by_filename(filename);
    if (sprite)
    {
        sprite->_inuse++;
        return sprite;
    }
    sprite = gf2d_sprite_new();
    if (!sprite)
    {
        return NULL;
    }
    sprite->texture = gf3d_texture_load(filename);
    if (!sprite->texture)
    {
        slog("gf2d_sprite_load: failed to load texture for sprite");
        gf2d_sprite_free(sprite);
        return NULL;
    }
    if (frame_width <= 0)frame_width = sprite->texture->width;
    if (frame_height <= 0)frame_height = sprite->texture->height;
    sprite->frameWidth = frame_width;
    sprite->frameHeight = frame_height;
    if (frames_per_line)sprite->framesPerLine = frames_per_line;
    else sprite->framesPerLine = 1;
    gfc_line_cpy(sprite->filename,filename);
    gf2d_sprite_create_vertex_buffer(sprite);
    return sprite;
}

void gf2d_sprite_free(Sprite *sprite)
{
    if (!sprite)return;
    sprite->_inuse--;
    if (sprite->_inuse <= 0)gf2d_sprite_delete(sprite);
}

void gf2d_sprite_delete(Sprite *sprite)
{
    if (!sprite)return;
    
    if (sprite->buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf2d_sprite.device, sprite->buffer, NULL);
        slog("sprite %s vert buffer freed",sprite->filename);
    }
    if (sprite->bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf2d_sprite.device, sprite->bufferMemory, NULL);
        slog("sprite %s vert buffer memory freed",sprite->filename);
    }

    gf3d_texture_free(sprite->texture);
    memset(sprite,0,sizeof(Sprite));
}


void gf2d_sprite_render(Sprite *sprite,VkCommandBuffer commandBuffer, VkDescriptorSet * descriptorSet)
{
    VkDeviceSize offsets[] = {0};
    Pipeline *pipe;
    if (!sprite)
    {
        slog("cannot render a NULL sprite");
        return;
    }
    pipe = gf2d_sprite_get_pipeline();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &sprite->buffer, offsets);
    
    vkCmdBindIndexBuffer(commandBuffer, gf2d_sprite.faceBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe->pipelineLayout, 0, 1, descriptorSet, 0, NULL);
    
    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}


void gf2d_sprite_draw(Sprite *sprite,Vector2D position,Vector2D scale,Vector3D rotation,Color color,Uint32 frame)
{
    VkDescriptorSet *descriptorSet = NULL;
    Uint32 buffer_frame;
    VkCommandBuffer commandBuffer;

    if (!sprite)
    {
        slog("cannot render a NULL sprite");
        return;
    }
    
    commandBuffer = gf3d_vgraphics_get_current_command_overlay_buffer();
    buffer_frame = gf3d_vgraphics_get_current_buffer_frame();

    descriptorSet = gf3d_pipeline_get_descriptor_set(gf2d_sprite.pipe, buffer_frame);
    if (descriptorSet == NULL)
    {
        slog("failed to get a free descriptor Set for sprite rendering");
        return;
    }
    
    gf2d_sprite_update_basic_descriptor_set(
        sprite,
        *descriptorSet,
        buffer_frame,
        position,
        scale,
        rotation,
        color,
        frame);
    gf2d_sprite_render(sprite,commandBuffer,descriptorSet);
}

void gf2d_sprite_create_vertex_buffer(Sprite *sprite)
{
    void *data = NULL;
    VkDevice device = gf2d_sprite.device;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    SpriteVertex vertices[] = {
        {
            {0,0},
            {0,0}
        },
        {
            {sprite->frameWidth,0},
            {sprite->frameWidth/(float)sprite->texture->width,0}
        },
        {
            {0,sprite->frameHeight},
            {0,sprite->frameHeight/(float)sprite->texture->height}
        },
        {
            {sprite->frameWidth,sprite->frameHeight},
            {sprite->frameWidth/(float)sprite->texture->width,sprite->frameHeight/(float)sprite->texture->height}
        }
    };
    bufferSize = sizeof(SpriteVertex) * 4;
    
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &sprite->buffer, &sprite->bufferMemory);

    gf3d_buffer_copy(stagingBuffer, sprite->buffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);    
}

void gf2d_sprite_update_uniform_buffer(
    Sprite *sprite,
    UniformBuffer *ubo,
    Vector2D position,
    Vector2D scale,
    Vector3D rotation,
    Color color,
    Uint32 frame)
{
    void* data;
    SpriteUBO spriteUBO = {0};
    spriteUBO.size = vector2d(sprite->frameWidth,sprite->frameHeight);
    spriteUBO.extent = gf3d_vgraphics_get_view_extent_as_vector2d();;
    spriteUBO.colorMod = gfc_color_to_vector4f(color);
    spriteUBO.position = position;
    spriteUBO.scale = scale;
    gfc_matrix_identity(spriteUBO.rotation);
    spriteUBO.rotation[0][0] = cos(rotation.z);
    spriteUBO.rotation[0][1] = sin(rotation.z);
    spriteUBO.rotation[1][0] = sin(rotation.z) * -1;//clockwise rotation
    spriteUBO.rotation[1][1] = cos(rotation.z);

    spriteUBO.frame_offset.x = (frame%sprite->framesPerLine * sprite->frameWidth)/(float)sprite->texture->width;
    spriteUBO.frame_offset.y = (frame/sprite->framesPerLine * sprite->frameHeight)/(float)sprite->texture->height;
    
    vkMapMemory(gf2d_sprite.device, ubo->uniformBufferMemory, 0, sizeof(SpriteUBO), 0, &data);
    
        memcpy(data, &spriteUBO, sizeof(SpriteUBO));

    vkUnmapMemory(gf2d_sprite.device, ubo->uniformBufferMemory);
}

void gf2d_sprite_update_basic_descriptor_set(
    Sprite *sprite,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex,
    Vector2D position,
    Vector2D scale,
    Vector3D rotation,
    Color color,
    Uint32 frame)
{
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};
    UniformBuffer *ubo;

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

    ubo = gf3d_uniform_buffer_list_get_buffer(gf2d_sprite.pipe->uboList, chainIndex);
    gf2d_sprite_update_uniform_buffer(sprite,ubo,position,scale,rotation,color,frame);

    bufferInfo.buffer = ubo->uniformBuffer;
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

    vkUpdateDescriptorSets(gf2d_sprite.device, 2, descriptorWrite, 0, NULL);
}

VkVertexInputBindingDescription * gf2d_sprite_get_bind_description()
{
    gf2d_sprite.bindingDescription.binding = 0;
    gf2d_sprite.bindingDescription.stride = sizeof(SpriteVertex);
    gf2d_sprite.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return &gf2d_sprite.bindingDescription;
}

VkVertexInputAttributeDescription * gf2d_sprite_get_attribute_descriptions(Uint32 *count)
{
    gf2d_sprite.attributeDescriptions[0].binding = 0;
    gf2d_sprite.attributeDescriptions[0].location = 0;
    gf2d_sprite.attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    gf2d_sprite.attributeDescriptions[0].offset = offsetof(SpriteVertex, vertex);
    
    gf2d_sprite.attributeDescriptions[1].binding = 0;
    gf2d_sprite.attributeDescriptions[1].location = 1;
    gf2d_sprite.attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    gf2d_sprite.attributeDescriptions[1].offset = offsetof(SpriteVertex, texel);
    if (count)*count = SPRITE_ATTRIBUTE_COUNT;
    return gf2d_sprite.attributeDescriptions;
}

Pipeline *gf2d_sprite_get_pipeline()
{
    return gf2d_sprite.pipe;
}

/*eol@eof*/
