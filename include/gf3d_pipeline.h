#ifndef __GF3D_PIPELINE_H__
#define __GF3D_PIPELINE_H__

#include <vulkan/vulkan.h>

#include "gfc_types.h"

#include "gf3d_uniform_buffers.h"
#include "gf3d_texture.h"

typedef struct
{
    Uint8                   inuse;
    Uint32                  index;          //which index in the UBO are we
    VkDescriptorSet        *descriptorSet;  //pointer to the descriptorSet we will use for this draw call
    VkBuffer                vertexBuffer;
    Uint32                  vertexCount;
    VkBuffer                indexBuffer;
    void                   *uboData;        //pointer to corresponding memory in the pipeline uboData
    Texture                *texture;        //optional!!
}PipelineDrawCall;

typedef struct
{
    Bool                    inUse;
    GFC_TextLine            name;                   /**<name of pipeline for debugging*/
    VkPipeline              pipeline;               /**<pipeline handle*/
    VkRenderPass            renderPass;
    VkPipelineLayout        pipelineLayout;
    char                   *vertShader;             /**<the shader loaded from disk*/
    size_t                  vertSize;               /**<memory size of the shader*/
    VkShaderModule          vertModule;             /**<the index of the shader module within the device*/
    char                   *fragShader;             /**<the shader loaded from disk*/
    size_t                  fragSize;               /**<memory size of the shader*/
    VkShaderModule          fragModule;             /**<the index of the shader module within the device*/
    VkDevice                device;
    Uint32                 *descriptorCursor;       /**<keeps track of which descriptors have been used per frame*/
    VkDescriptorPool       *descriptorPool;
    VkDescriptorSetLayout   descriptorSetLayout;
    VkDescriptorSet       **descriptorSets;
    Uint32                  descriptorPoolCount;
    Uint32                  descriptorSetCount;
    Uint32                  drawCallCount;          /**<how many drawCalls have been queued*/
    PipelineDrawCall       *drawCallList;           /**<cached draw calls for this frame*/
    Uint32                  drawCallListCount;      /**<how many drawCalls are available*/
    
    char                   *uboData;                /**<pre-allocated cpu side UBO data*/
    size_t                  uboBufferSize;          /**<how large the whole buffer is*/
    size_t                  uboDataSize;            /**<size of a single UBO for this pipeline*/
    UniformBufferList      *uboBigBuffer;           /**<for batched draws.  This is the memory for ALL draws one per frame*/
    
    VkCommandBuffer         commandBuffer;          /**<for current command*/
    VkIndexType             indexType;              /**<size of the indices in the index buffer*/
}Pipeline;

/**
 * @brief setup pipeline system
 */
void gf3d_pipeline_init();

/**
 * @brief free a created pipeline
 */
void gf3d_pipeline_free(Pipeline *pipe);

/**
 * @brief setup a pipeline for rendering
 * @note depricating
 * @param device the logical device that the pipeline will be set up on
 * @param vertFile the filename of the vertex shader to use (expects spir-v byte code)
 * @param fragFile the filename of the fragment shader to use (expects spir-v byte code)
 * @param extent the viewport dimensions for this pipeline
 * @returns NULL on error (see logs) or a pointer to a pipeline
 */
Pipeline *gf3d_pipeline_graphics_load(VkDevice device,const char *vertFile,const char *fragFile,VkExtent2D extent);


/**
 * @brief create a pipeline from config
 * @param device the logical device to create the pipeline for
 * @param configFile the filepath to the config file
 * @param extent the screen resolution this pipeline will be working towards
 * @param descriptorCount the number of concurrent descriptSets to be supported per command, ie: how many models you want to support for a draw call  This should be based on maximum number of supported entities or graphic assets
 * @param vertexInputDescription the vertex input description to use
 * @param vertextInputAttributeDescriptions list of how the attributes are described
 * @param vertexAttributeCount how many of the above are provided in the list
 * @param bufferSize the sizeof() the ubo to be used with this pipeline
 * @param indexType VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_UINT8_EXT
 * @returns NULL on error (see logs) or a pointer to a pipeline
*/
Pipeline *gf3d_pipeline_create_from_config(
    VkDevice device,
    const char *configFile,
    VkExtent2D extent,
    Uint32 descriptorCount,
    const VkVertexInputBindingDescription* vertexInputDescription,
    const VkVertexInputAttributeDescription * vertextInputAttributeDescriptions,
    Uint32 vertexAttributeCount,
    VkDeviceSize bufferSize,
    VkIndexType indexType);

/**
 * @brief setup a pipeline for rendering a basic sprite
 * @param device the logical device that the pipeline will be set up on
 * @param vertFile the filename of the vertex shader to use (expects spir-v byte code)
 * @param fragFile the filename of the fragment shader to use (expects spir-v byte code)
 * @param extent the viewport dimensions for this pipeline
 * @param descriptorCount the number of concurrent descriptSets to be suppert per command, ie: how many models you want to support for a draw call  This should be based on maximum number of supported entities or graphic assets
 * @returns NULL on error (see logs) or a pointer to a pipeline
 */
Pipeline *gf3d_pipeline_basic_sprite_create(VkDevice device,const char *vertFile,const char *fragFile,VkExtent2D extent,Uint32 descriptorCount);

/**
 * @brief get a descriptor set to be used for the pipeline.  Provide the swap chain rendering frame.
 * @param pipe the pipeline to get a descriptSet for
 * @param frame the swap chain rendering frame to get a descriptor set for.
 */
VkDescriptorSet * gf3d_pipeline_get_descriptor_set(Pipeline *pipe, Uint32 frame);

/**
 * @brief reset the descriptor Set cursor for the given swap chain frame
 * @param pipe the pipeline to reset
 * @param frame the swap chain rendering frame to reset the cursor for
 */
void gf3d_pipeline_reset_frame(Pipeline *pipe,Uint32 frame);

/**
 * @brief queue up a render for a pipeline
 * @param pipe the pipeline to queue up for
 * @param vertexBuffer which buffer to bind
 * @param vertexCount how many vertices to draw (usually 3 per face)
 * @param indexBuffer which face buffer to use for the draw
 * @param uboData the UBO data to draw with.  Note this is copied by the function, feel free to change it after use
 * @param texture [optional] if you have a texture to render with, provide it here.  Note if the pipeline needs one, you MUST provide one
 */
void gf3d_pipeline_queue_render(
    Pipeline *pipe,
    VkBuffer vertexBuffer,
    Uint32 vertexCount,
    VkBuffer indexBuffer,
    void *uboData,
    Texture *texture);

/**
 * @brief bind a draw call to the current command
 */
void gf3d_pipeline_call_render(
    Pipeline *pipe,
    VkDescriptorSet * descriptorSet,
    VkBuffer vertexBuffer,
    Uint32 vertexCount,
    VkBuffer indexBuffer);

/**
 * @brief resets ALL pipelines currently in use
 */
void gf3d_pipeline_reset_all_pipes();

/**
 * @brief submit the render calls to the pipeline for this frame.  Called after reset_frame and all draw calls
 * @param pipe for the pipe in question
 */
void gf3d_pipeline_submit_commands(Pipeline *pipe);

/**
 * @brief submit the commands for ALL pipelines in the order in which they were created
 * @note order might be messed up if any were destroyed and recreated during the life of the program
 */
void gf3d_pipeline_submit_all_pipe_commands();

VkFormat gf3d_pipeline_find_depth_format();

#endif
