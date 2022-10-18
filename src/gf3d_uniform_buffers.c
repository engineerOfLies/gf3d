#include "simple_logger.h"

#include "gf3d_buffers.h"
#include "gf3d_uniform_buffers.h"

UniformBufferList *gf3d_uniform_buffer_list_new(VkDevice device,VkDeviceSize bufferSize, Uint32 bufferCount,Uint32 bufferFrames)
{
    int i,j;
    UniformBufferList *bufferList;
    if ((!bufferCount)||(!bufferFrames))
    {
        slog("cannot allocate zero buffers!");
        return NULL;
    }
    bufferList = gfc_allocate_array(sizeof(UniformBufferList),1);
    if (!bufferList)return NULL;
    
    bufferList->device = device;
    
    bufferList->buffers = gfc_allocate_array(sizeof(UniformBuffer  *),bufferFrames);
    
    if (!bufferList->buffers)
    {
        gf3d_uniform_buffer_list_free(bufferList);
        return NULL;
    }
    
    for (j = 0; j < bufferFrames; j ++)
    {
        bufferList->buffers[j] = gfc_allocate_array(sizeof(UniformBuffer),bufferCount);
        if (!bufferList->buffers[j])
        {
            gf3d_uniform_buffer_list_free(bufferList);
            return NULL;
        }
        for (i = 0; i < bufferCount; i++)
        {
            gf3d_buffer_create(
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &bufferList->buffers[j][i].uniformBuffer,
                &bufferList->buffers[j][i].uniformBufferMemory);
        }
    }
    bufferList->buffer_count = bufferCount;
    bufferList->buffer_frames = bufferFrames;
    
    
    return bufferList;
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
