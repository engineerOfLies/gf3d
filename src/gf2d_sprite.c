#include <stdalign.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_shape.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf2d_sprite.h"

#define SPRITE_ATTRIBUTE_COUNT 2

extern int __DEBUG;

typedef struct
{
    GFC_Matrix4 rotation;
    GFC_Vector4D colorMod;
    GFC_Vector4D clip;
    GFC_Vector2D size;
    GFC_Vector2D extent;
    GFC_Vector2D position;
    GFC_Vector2D scale;
    GFC_Vector2D frame_offset;
    GFC_Vector2D center;
    float        drawOrder;
    float        padding;
}SpriteUBO;

typedef struct
{
    GFC_Vector2D vertex;
    GFC_Vector2D texel;
}SpriteVertex;

typedef struct
{
    Uint16  verts[3];
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
    float           drawOrder;
}SpriteManager;


SpriteUBO gf2d_sprite_get_uniform_buffer(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    GFC_Vector3D rotation,
    GFC_Color color,
    GFC_Vector4D clip,
    GFC_Vector2D flip,
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
    }
    if (gf2d_sprite.faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf2d_sprite.device, gf2d_sprite.faceBufferMemory, NULL);
    }

    memset(&gf2d_sprite,0,sizeof(SpriteManager));
    if(__DEBUG)slog("sprite manager closed");
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
        sizeof(SpriteUBO),
        VK_INDEX_TYPE_UINT16
    );     
    
    if(__DEBUG)slog("sprite manager initiliazed");
    atexit(gf2d_sprite_manager_close);
}

void gf3d_sprite_reset_pipes()
{
    Uint32 bufferFrame = gf3d_vgraphics_get_current_buffer_frame();
    
    gf3d_pipeline_reset_frame(gf2d_sprite.pipe,bufferFrame);
    gf2d_sprite.drawOrder = 0;
}

void gf3d_sprite_submit_pipe_commands()
{
    gf3d_pipeline_submit_commands(gf2d_sprite.pipe);
}


Sprite *gf2d_sprite_get_by_filename(const char *filename)
{
    int i;
    if (!filename)return NULL;
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
    sprite->widthPercent = sprite->frameWidth / (float)sprite->texture->width;
    sprite->heightPercent = sprite->frameHeight/ (float)sprite->texture->height;
    if (frames_per_line)sprite->framesPerLine = frames_per_line;
    else sprite->framesPerLine = 1;
    gf2d_sprite_create_vertex_buffer(sprite);
    sprite->surface = surface;
    return sprite;
}

Sprite * gf2d_sprite_load_image(const char * filename)
{
    return gf2d_sprite_load(filename,0,0, 1);
}

Sprite *gf2d_sprite_parse(SJson *json)
{
    int frameWidth = -1,frameHeight = -1;
    int framesPerLine = 1;
    Sprite *sprite = NULL;
    const char *str;
    if (!json)return NULL;
    str = sj_object_get_value_as_string(json,"sprite");
    if (!str)
    {
        slog("cannot parse from sprite, bad json.  Missing sprite 'tag'");
        return NULL;
    }
    sprite = gf2d_sprite_get_by_filename(str);
    if (sprite)return sprite;// already loaded
    sj_object_get_value_as_int(json,"frameWidth",&frameWidth);
    sj_object_get_value_as_int(json,"frameHeight",&frameHeight);
    sj_object_get_value_as_int(json,"framesPerLine",&framesPerLine);
    return gf2d_sprite_load(str,frameWidth,frameHeight, framesPerLine);
}

Sprite * gf2d_sprite_load(const char * filename,int frame_width,int frame_height, Uint32 frames_per_line)
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
    sprite->surface = sprite->texture->surface;
    if (frame_width <= 0)frame_width = sprite->texture->width;
    if (frame_height <= 0)frame_height = sprite->texture->height;
    sprite->frameWidth = frame_width;
    sprite->frameHeight = frame_height;
    sprite->widthPercent = sprite->frameWidth / (float)sprite->texture->width;
    sprite->heightPercent = sprite->frameHeight/ (float)sprite->texture->height;
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
    }
    if (sprite->bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf2d_sprite.device, sprite->bufferMemory, NULL);
    }

    gf3d_texture_free(sprite->texture);
    memset(sprite,0,sizeof(Sprite));
}

void gf2d_sprite_draw_full(
    Sprite   * sprite,
    GFC_Vector2D   position,
    GFC_Vector2D   scale,
    GFC_Vector2D   center,
    float      rotation,
    GFC_Vector2D   flip,
    GFC_Color      colorShift,
    GFC_Vector4D   clip,
    Uint32     frame)
{
    gf2d_sprite_draw(
        sprite,
        position,
        &scale,
        &center,
        &rotation,
        &flip,
        &colorShift,
        &clip,
       frame);
}

void gf2d_sprite_draw_image(
    Sprite   * sprite,
    GFC_Vector2D   position)
{
    gf2d_sprite_draw(
        sprite,
        position,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,0);
}

void gf2d_sprite_draw_simple(
    Sprite   * sprite,
    GFC_Vector2D   position,
    Uint32     frame)
{
    gf2d_sprite_draw(
        sprite,
        position,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        frame);
}


void gf2d_sprite_draw(
    Sprite   * sprite,
    GFC_Vector2D   position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    float    * rotation,
    GFC_Vector2D * flip,
    GFC_Color    * colorShift,
    GFC_Vector4D * clip,
    Uint32     frame)
{
    SpriteUBO spriteUBO = {0};
    GFC_Vector2D drawScale = {1,1};
    GFC_Vector3D drawRotation = {0,0,0};
    GFC_Vector2D drawFlip = {0,0};
    GFC_Vector4D drawClip = {0,0,0,0};
    GFC_Color    drawGFC_ColorShift = gfc_color(1,1,1,1);

    if (!sprite)
    {
        slog("cannot render a NULL sprite");
        return;
    }
    
    if (scale)gfc_vector2d_copy(drawScale,(*scale));
    if (center)
    {
        gfc_vector2d_copy(drawRotation,(*center));
    }
    if (rotation)drawRotation.z = *rotation;
    if (flip)gfc_vector2d_copy(drawFlip,(*flip));
    if (colorShift)drawGFC_ColorShift = *colorShift;
    if (clip)gfc_vector4d_copy(drawClip,(*clip));
    
    
    spriteUBO = gf2d_sprite_get_uniform_buffer(
        sprite,
        position,
        drawScale,
        drawRotation,
        drawGFC_ColorShift,
        drawClip,
        drawFlip,
        frame);

    gf3d_pipeline_queue_render(
        gf2d_sprite.pipe,
        sprite->buffer,
        6,//its a single quad
        gf2d_sprite.faceBuffer,
        &spriteUBO,
        sprite->texture);
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
            {(sprite->frameWidth + 1)*2,0},
            {sprite->frameWidth/((float)sprite->texture->width + 0.01),0}
        },
        {
            {0,(sprite->frameHeight + 1)*2},
            {0,sprite->frameHeight/((float)sprite->texture->height + 0.01)}
        },
        {
            {(sprite->frameWidth + 1)*2,(sprite->frameHeight+ 1)*2},
            {sprite->frameWidth/((float)sprite->texture->width + 0.01),sprite->frameHeight/((float)sprite->texture->height + 0.01)}
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

void gf2d_sprite_draw_to_surface(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    Uint32 frame,
    SDL_Surface *surface
)
{
    SDL_Rect cell,target;
    int fpl;
    GFC_Vector2D scaleFactor = {1,1};
    GFC_Vector2D scaleOffset = {0,0};
    if (!sprite)
    {
        slog("no sprite provided to draw");
        return;
    }
    if (!sprite->surface)
    {
        slog("sprite does not contain surface to draw with");
        return;
    }
    if (!surface)
    {
        slog("no surface provided to draw to");
        return;
    }
    if (scale)
    {
        gfc_vector2d_copy(scaleFactor,(*scale));
    }
    if (center)
    {
        gfc_vector2d_copy(scaleOffset,(*center));
    }
    fpl = (sprite->framesPerLine)?sprite->framesPerLine:1;
    gfc_rect_set(
        cell,
        frame%fpl * sprite->frameWidth,
        frame/fpl * sprite->frameHeight,
        sprite->frameWidth,
        sprite->frameHeight);
    gfc_rect_set(
        target,
        position.x - (scaleFactor.x * scaleOffset.x),
        position.y - (scaleFactor.y * scaleOffset.y),
        sprite->frameWidth * scaleFactor.x,
        sprite->frameHeight * scaleFactor.y);
    SDL_BlitScaled(
        sprite->surface,
        &cell,
        surface,
        &target);
}

SpriteUBO gf2d_sprite_get_uniform_buffer(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    GFC_Vector3D rotation,
    GFC_Color color,
    GFC_Vector4D clip,
    GFC_Vector2D flip,
    Uint32 frame)
{
    SpriteUBO spriteUBO = {0};
    spriteUBO.size = gfc_vector2d(sprite->texture->width,sprite->texture->height);
    spriteUBO.extent = gf3d_vgraphics_get_view_extent_as_vector2d();;
    spriteUBO.colorMod = gfc_color_to_vector4f(color);
    spriteUBO.position = position;
    spriteUBO.scale = scale;
    gfc_matrix4_identity(spriteUBO.rotation);
    spriteUBO.rotation[0][0] = cos(rotation.z);
    spriteUBO.rotation[0][1] = sin(rotation.z);
    spriteUBO.rotation[1][0] = sin(rotation.z) * -1;//clockwise rotation
    spriteUBO.rotation[1][1] = cos(rotation.z);
    spriteUBO.drawOrder = gf2d_sprite.drawOrder;
    gf2d_sprite.drawOrder += 0.000000001;
    spriteUBO.center.x = rotation.x;
    spriteUBO.center.y = rotation.y;
    if (flip.x)
    {
        scale.x = fabs(scale.x)*-1;
    }
    if (flip.y)
    {
        scale.y = fabs(scale.y)*-1;
    }
    
    gfc_vector4d_copy(spriteUBO.clip,clip);
    spriteUBO.frame_offset.x = (frame%sprite->framesPerLine * sprite->frameWidth)/(float)sprite->texture->width;
    spriteUBO.frame_offset.y = (frame/sprite->framesPerLine * sprite->frameHeight)/(float)sprite->texture->height;
    return spriteUBO;
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
