#ifndef __GF3D_PIPELINE_H__
#define __GF3D_PIPELINE_H__

#include <vulkan/vulkan.h>

#include "gf3d_types.h"


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
 * @param device the logical device that the pipeline will be set up on
 * @param vertFile the filename of the vertex shader to use (expects spir-v byte code)
 * @param fragFile the filename of the fragment shader to use (expects spir-v byte code)
 * @param extent the viewport dimensions for this pipeline
 * @returns NULL on error (see logs) or a pointer to a pipeline
 */
Pipeline *gf3d_pipeline_graphics_load(VkDevice device,char *vertFile,char *fragFile,VkExtent2D extent);

VkFormat gf3d_pipeline_find_depth_format();

#endif
