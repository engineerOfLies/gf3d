#ifndef __GF3D_PIPELINE_H__
#define __GF3D_PIPELINE_H__

#include <vulkan/vulkan.h>

#include "gfc_types.h"


typedef struct
{
    Bool                    inUse;
    VkPipeline              pipeline;
    VkRenderPass            renderPass;
    VkPipelineLayout        pipelineLayout;
    size_t                  vertSize;
    char                   *vertShader;
    VkShaderModule          vertModule;
    size_t                  fragSize;
    char                   *fragShader;
    VkShaderModule          fragModule;
    VkDevice                device;
    Uint32                 *descriptorCursor;       /**<keeps track of which descriptors have been used per frame*/
    VkDescriptorPool       *descriptorPool;
    VkDescriptorSetLayout   descriptorSetLayout;
    VkDescriptorSet       **descriptorSets;
    Uint32                  descriptorPoolCount;
    Uint32                  descriptorSetCount;
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
Pipeline *gf3d_pipeline_graphics_load(VkDevice device,char *vertFile,char *fragFile,VkExtent2D extent);

/**
 * @brief setup a pipeline for rendering a basic model
 * @param device the logical device that the pipeline will be set up on
 * @param vertFile the filename of the vertex shader to use (expects spir-v byte code)
 * @param fragFile the filename of the fragment shader to use (expects spir-v byte code)
 * @param extent the viewport dimensions for this pipeline
 * @param descriptorCount the number of concurrent descriptSets to be suppert per command, ie: how many models you want to support for a draw call  This should be based on maximum number of supported entities or graphic assets
 * @returns NULL on error (see logs) or a pointer to a pipeline
 */
Pipeline *gf3d_pipeline_basic_model_create(VkDevice device,char *vertFile,char *fragFile,VkExtent2D extent,Uint32 descriptorCount);

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

VkFormat gf3d_pipeline_find_depth_format();

#endif
