#include "simple_logger.h"

#include "gf3d_buffers.h"
#include "gf3d_uniform_buffers.h"

void gf3d_uniform_buffer_setup(UniformBuffer *buffer,VkDeviceSize bufferSize)
{
    if (!buffer)return;
    buffer->_inuse = 1;
    buffer->bufferSize = bufferSize;
    gf3d_buffer_create(
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &buffer->uniformBuffer,
        &buffer->uniformBufferMemory);
}

UniformBufferList *gf3d_uniform_buffer_list_new(VkDevice device,VkDeviceSize bufferSize, Uint32 bufferCount,Uint32 bufferFrames)
{
    int i,j;
    UniformBufferList *bufferGFC_List;
    if ((!bufferCount)||(!bufferFrames))
    {
        slog("cannot allocate zero buffers!");
        return NULL;
    }
    bufferGFC_List = gfc_allocate_array(sizeof(UniformBufferList),1);
    if (!bufferGFC_List)
    {
        slog("failed to allocate unform buffers list");
        return NULL;
    }
    
    bufferGFC_List->device = device;
    
    bufferGFC_List->buffers = gfc_allocate_array(sizeof(UniformBuffer  *),bufferFrames);
    
    if (!bufferGFC_List->buffers)
    {
        gf3d_uniform_buffer_list_free(bufferGFC_List);
        slog("failed to allocate unform buffers list");
        return NULL;
    }
    
    for (j = 0; j < bufferFrames; j ++)
    {
        bufferGFC_List->buffers[j] = gfc_allocate_array(sizeof(UniformBuffer),bufferCount);
        if (!bufferGFC_List->buffers[j])
        {
            gf3d_uniform_buffer_list_free(bufferGFC_List);
            slog("failed to allocate unform buffers");
            return NULL;
        }
        for (i = 0; i < bufferCount; i++)
        {
            gf3d_buffer_create(
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &bufferGFC_List->buffers[j][i].uniformBuffer,
                &bufferGFC_List->buffers[j][i].uniformBufferMemory);
                bufferGFC_List->buffers[j][i].bufferSize = bufferSize;
        }
        
    }
    bufferGFC_List->buffer_count = bufferCount;
    bufferGFC_List->buffer_frames = bufferFrames;
    
    
    return bufferGFC_List;
}

void gf3d_uniform_buffer_list_free(UniformBufferList *list)
{
    int i,j;
    if (!list)return;
    for (j = 0; j < list->buffer_frames;j++)
    {
        for (i = 0; i < list->buffer_count; i++)
        {
            if (list->buffers[j][i].uniformBuffer)
            {
                vkDestroyBuffer(list->device, list->buffers[j][i].uniformBuffer, NULL);
            }
            if (list->buffers[j][i].uniformBufferMemory)
            {
                vkFreeMemory(list->device, list->buffers[j][i].uniformBufferMemory, NULL);
            }
        }
    }
}

UniformBuffer *gf3d_uniform_buffer_list_get_nth_buffer(UniformBufferList *list, Uint32 nth, Uint32 bufferFrame)
{
    if (!list)return NULL;
    if (bufferFrame >= list->buffer_frames)
    {
        slog("buffer frame out of range");
        return NULL;
    }
    if (nth >= list->buffer_count)
    {
        slog("index out of range");
        return NULL;
    }
    return &list->buffers[bufferFrame][nth];
}

UniformBuffer *gf3d_uniform_buffer_list_get_buffer(UniformBufferList *list, Uint32 bufferFrame)
{
    int i;
    if (!list)return NULL;
    if (bufferFrame >= list->buffer_frames)
    {
        slog("buffer frame out of range");
        return NULL;
    }
    for (i = 0;i < list->buffer_count;i++)
    {
        if (list->buffers[bufferFrame][i]._inuse)continue;// skip whats in use.
        list->buffers[bufferFrame][i]._inuse = 1;
        return &list->buffers[bufferFrame][i];
    }
    slog("out of uniform buffers");
    return NULL;
}

void gf3d_uniform_buffer_list_clear(UniformBufferList *list, Uint32 bufferFrame)
{
    int i;
    if (!list)return;
    if (bufferFrame >= list->buffer_frames)
    {
        slog("buffer frame out of range");
        return;
    }
    for (i = 0;i < list->buffer_count;i++)
    {
        list->buffers[bufferFrame][i]._inuse = 0;// just marked free here, cleaned up on assignment
    }
}

/*eol@eof*/
