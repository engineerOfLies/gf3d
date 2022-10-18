#ifndef __GF3D_UNIFORM_BUFFERS_H__
#define __GF3D_UNIFORM_BUFFERS_H__

#include <vulkan/vulkan.h>

#include "gfc_types.h"

typedef struct
{
    Uint8                   _inuse;                 /**<if this buffer is currently being used*/
    VkBuffer                uniformBuffer;      /**<buffer handle passed to render calls*/
    VkDeviceMemory          uniformBufferMemory;/**<buffer memory for updating the data*/
}UniformBuffer;

typedef struct
{
    VkDevice         device;            /**<which device this is configured for*/
    Uint32           buffer_count;
    Uint32           buffer_frames;
    UniformBuffer  **buffers;
}UniformBufferList;

/**
 * @brief create a new list for uniform buffers
 * @param device the device to build this list of uniformBuffers for
 * @param bufferSize the sizeof() the data to be stored
 * @param bufferCount how many buffers in the list.  T
 * his should be large enough to support the number of calls per frame you will need
 * @param bufferFrames how many buffer frames to support.  This should match the swap chain length
 * @return NULL on error, or a new list of uniform buffers
 */
UniformBufferList *gf3d_uniform_buffer_list_new(VkDevice device,VkDeviceSize bufferSize,Uint32 bufferCount,Uint32 bufferFrames);

/**
 * @brief free a previously created uniform buffer list
 * @param list the list to free
 */
void gf3d_uniform_buffer_list_free(UniformBufferList *list);

/**
 * @brief get an unused uniform buffer from the list
 * @param list the list to get it from
 * @param bufferFrame the frame to get it from
 * @return NULL if no more buffers left, or a valid InformBuffer pointer
 */
UniformBuffer *gf3d_uniform_buffer_list_get_buffer(UniformBufferList *list, Uint32 bufferFrame);

/**
 * @brief clear all of the uniform buffers that have been used for the buffer frame
 */
void gf3d_uniform_buffer_list_clear(UniformBufferList *list, Uint32 bufferFrame);



#endif
