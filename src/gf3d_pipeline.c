#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_shaders.h"

#include <string.h>
#include <stdio.h>
#include "simple_logger.h"

typedef struct
{
    Uint32      maxPipelines;
    Pipeline   *pipelineList;
}PipelineManager;

static PipelineManager gf3d_pipeline = {0};

void gf3d_pipeline_close();

void gf3d_pipeline_init(Uint32 max_pipelines)
{
    if (max_pipelines == 0)
    {
        slog("cannot initialize zero pipelines");
        return;
    }
    gf3d_pipeline.pipelineList = (Pipeline *)gf3d_allocate_array(sizeof(Pipeline),max_pipelines);
    if (!gf3d_pipeline.pipelineList)
    {
        slog("failed to allocate pipeline manager");
        return;
    }
    gf3d_pipeline.maxPipelines = max_pipelines;
    atexit(gf3d_pipeline_close);
}

void gf3d_pipeline_close()
{
    int i;
    slog("cleaning up pipelines");
    if (gf3d_pipeline.pipelineList != 0)
    {
        for (i = 0; i < gf3d_pipeline.maxPipelines; i++)
        {
            gf3d_pipeline_free(&gf3d_pipeline.pipelineList[i]);
        }
        free(gf3d_pipeline.pipelineList);
    }
    memset(&gf3d_pipeline,0,sizeof(PipelineManager));
}

Pipeline *gf3d_pipeline_new()
{
    int i;
    for (i = 0; i < gf3d_pipeline.maxPipelines; i++)
    {
        if (gf3d_pipeline.pipelineList[i].inUse)continue;
        gf3d_pipeline.pipelineList[i].inUse = true;
        return &gf3d_pipeline.pipelineList[i];
    }
    slog("no free pipelines");
    return NULL;
}

void gf3d_pipeline_render_pass_setup(Pipeline *pipe)
{
    VkAttachmentDescription colorAttachment = {0};
    VkAttachmentReference colorAttachmentRef = {0};
    VkSubpassDescription subpass = {0};
    VkRenderPassCreateInfo renderPassInfo = {0};
    VkSubpassDependency dependency = {0};
    
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    colorAttachment.format = gf3d_swapchain_get_format();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(pipe->device, &renderPassInfo, NULL, &pipe->renderPass) != VK_SUCCESS)
    {
        slog("failed to create render pass!");
        return;
    }
}


Pipeline *gf3d_pipeline_graphics_load(VkDevice device,char *vertFile,char *fragFile,VkExtent2D extent)
{
    Pipeline *pipe;
    VkRect2D scissor = {0};
    VkViewport viewport = {0};
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    VkPipelineViewportStateCreateInfo viewportState = {0};
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    VkPipelineShaderStageCreateInfo shaderStages[2];
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    VkPipelineColorBlendStateCreateInfo colorBlending = {0};

    pipe = gf3d_pipeline_new();
    if (!pipe)return NULL;

    pipe->vertShader = gf3d_shaders_load_data(vertFile,&pipe->vertSize);
    pipe->fragShader = gf3d_shaders_load_data(fragFile,&pipe->fragSize);
    
    pipe->vertModule = gf3d_shaders_create_module(pipe->vertShader,pipe->vertSize,device);
    pipe->fragModule = gf3d_shaders_create_module(pipe->fragShader,pipe->fragSize,device);

    pipe->device = device;
    
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = pipe->vertModule;
    vertShaderStageInfo.pName = "main";
    
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = pipe->fragModule;
    fragShaderStageInfo.pName = "main";
    
    shaderStages[0] = vertShaderStageInfo;
    shaderStages[1] = fragShaderStageInfo;
    
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL; // Optional    return pipe;

    // TODO: pull all this information from config file
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extent.width;
    viewport.height = (float) extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = extent;
    
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = NULL; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    gf3d_pipeline_render_pass_setup(pipe);
    
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipe->pipelineLayout) != VK_SUCCESS)
    {
        slog("failed to create pipeline layout!");
        gf3d_pipeline_free(pipe);
        return NULL;
    }
    
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = NULL; // Optional
    pipelineInfo.layout = pipe->pipelineLayout;
    pipelineInfo.renderPass = pipe->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipe->graphicsPipeline) != VK_SUCCESS)
    {   
        slog("failed to create graphics pipeline!");
        gf3d_pipeline_free(pipe);
        return NULL;
    }
    return pipe;
}

void gf3d_pipeline_free(Pipeline *pipe)
{
    if (!pipe)return;
    if (!pipe->inUse)return;
    if (pipe->graphicsPipeline)
    {
        vkDestroyPipeline(pipe->device, pipe->graphicsPipeline, NULL);
    }
    if (pipe->pipelineLayout)
    {
        vkDestroyPipelineLayout(pipe->device, pipe->pipelineLayout, NULL);
    }
    if (pipe->renderPass)
    {
        vkDestroyRenderPass(pipe->device, pipe->renderPass, NULL);
    }
    if (pipe->fragModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(pipe->device, pipe->fragModule, NULL);
    }
    if (pipe->vertModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(pipe->device, pipe->vertModule, NULL);
    }
    if (pipe->fragShader != NULL)
    {
        free(pipe->fragShader);
    }
    if (pipe->vertShader != NULL)
    {
        free (pipe->vertShader);
    }
    memset(pipe,0,sizeof(Pipeline));
}

/*eol@eof*/
