#include <string.h>
#include <stdio.h>
#include "simple_logger.h"

#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_shaders.h"
#include "gf3d_model.h"

typedef struct
{
    Uint32              maxPipelines;
    Pipeline           *pipelineList;
    Uint32              chainLength;
}PipelineManager;

static PipelineManager gf3d_pipeline = {0};

void gf3d_pipeline_close();
void gf3d_pipeline_create_basic_model_descriptor_pool(Pipeline *pipe);
void gf3d_pipeline_create_basic_model_descriptor_set_layout(Pipeline *pipe);
void gf3d_pipeline_create_descriptor_sets(Pipeline *pipe);

void gf3d_pipeline_init(Uint32 max_pipelines)
{
    if (max_pipelines == 0)
    {
        slog("cannot initialize zero pipelines");
        return;
    }
    gf3d_pipeline.pipelineList = (Pipeline *)gfc_allocate_array(sizeof(Pipeline),max_pipelines);
    if (!gf3d_pipeline.pipelineList)
    {
        slog("failed to allocate pipeline manager");
        return;
    }
    gf3d_pipeline.maxPipelines = max_pipelines;
    gf3d_pipeline.chainLength = gf3d_swapchain_get_chain_length();
    slog("pipeline manager created with chain length %i",gf3d_pipeline.chainLength);
    atexit(gf3d_pipeline_close);
    slog("pipeline system initialized");
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

VkFormat gf3d_pipeline_find_supported_format(VkFormat * candidates, Uint32 candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    int i;
    VkFormatProperties props = {0};
    for (i = 0; i < candidateCount;i++)
    {
        vkGetPhysicalDeviceFormatProperties(gf3d_vgraphics_get_default_physical_device(), candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return candidates[i];
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return candidates[i];
        }
    }

    slog("failed to find supported format!");
    return VK_NULL_HANDLE;
}

VkFormat gf3d_pipeline_find_depth_format()
{
    VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return gf3d_pipeline_find_supported_format(
        formats,3,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void gf3d_pipeline_render_pass_setup(Pipeline *pipe)
{
    VkAttachmentDescription colorAttachment = {0};
    VkAttachmentReference colorAttachmentRef = {0};
    VkSubpassDescription subpass = {0};
    VkRenderPassCreateInfo renderPassInfo = {0};
    VkSubpassDependency dependency = {0};
    VkAttachmentDescription depthAttachment = {0};
    VkAttachmentReference depthAttachmentRef = {0};
    VkAttachmentDescription attachments[2];
    
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    depthAttachment.format = gf3d_pipeline_find_depth_format();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    
    memcpy(&attachments[0],&colorAttachment,sizeof(VkAttachmentDescription));
    memcpy(&attachments[1],&depthAttachment,sizeof(VkAttachmentDescription));
    
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
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

Pipeline *gf3d_pipeline_basic_model_create(VkDevice device,char *vertFile,char *fragFile,VkExtent2D extent,Uint32 descriptorCount)
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
    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    
    pipe = gf3d_pipeline_new();
    if (!pipe)return NULL;

    pipe->vertShader = gf3d_shaders_load_data(vertFile,&pipe->vertSize);
    pipe->fragShader = gf3d_shaders_load_data(fragFile,&pipe->fragSize);
    
    pipe->vertModule = gf3d_shaders_create_module(pipe->vertShader,pipe->vertSize,device);
    pipe->fragModule = gf3d_shaders_create_module(pipe->fragShader,pipe->fragSize,device);

    pipe->device = device;
    pipe->descriptorSetCount = descriptorCount;
    
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;    
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;

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
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = gf3d_mesh_get_bind_description(); // Optional
    vertexInputInfo.pVertexAttributeDescriptions = gf3d_mesh_get_attribute_descriptions(&vertexInputInfo.vertexAttributeDescriptionCount); // Optional    

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
//    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
    
    gf3d_pipeline_create_basic_model_descriptor_pool(pipe);
    gf3d_pipeline_create_basic_model_descriptor_set_layout(pipe);
    gf3d_pipeline_create_descriptor_sets(pipe);
    
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &pipe->descriptorSetLayout; // Optional
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
    pipelineInfo.pDepthStencilState = &depthStencil;
    
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipe->pipeline) != VK_SUCCESS)
    {   
        slog("failed to create graphics pipeline!");
        gf3d_pipeline_free(pipe);
        return NULL;
    }
    return pipe;
}

void gf3d_pipeline_free(Pipeline *pipe)
{
    int i;
    if (!pipe)return;
    if (!pipe->inUse)return;
    if (pipe->descriptorCursor)
    {
        free(pipe->descriptorCursor);
        pipe->descriptorCursor = NULL;
    }
    if (pipe->descriptorPool != NULL)
    {
        for (i = 0;i < gf3d_pipeline.chainLength;i++)
        {
            if (pipe->descriptorPool[i] != VK_NULL_HANDLE)
            {
                slog("cleaning up pipeline descriptor pool");
                vkDestroyDescriptorPool(pipe->device, pipe->descriptorPool[i], NULL);
            }
        }
        free(pipe->descriptorPool);
    }
    if (pipe->descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(pipe->device, pipe->descriptorSetLayout, NULL);
    }
    if (pipe->pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(pipe->device, pipe->pipeline, NULL);
    }
    if (pipe->pipelineLayout != VK_NULL_HANDLE)
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

// TODO move descriptor sets to this section

void gf3d_pipeline_create_basic_model_descriptor_pool(Pipeline *pipe)
{
    int i;
    VkDescriptorPoolSize poolSize[2] = {0};
    VkDescriptorPoolCreateInfo poolInfo = {0};
    
    if (!pipe)
    {
        slog("no pipeline provided");
        return;
    }
    slog("attempting to make %i descriptor pools of size %i",gf3d_pipeline.chainLength,pipe->descriptorSetCount);
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = pipe->descriptorSetCount;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[1].descriptorCount = pipe->descriptorSetCount;
    
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize;
    poolInfo.maxSets = pipe->descriptorSetCount;
    pipe->descriptorPool = (VkDescriptorPool *)gfc_allocate_array(sizeof(VkDescriptorPool),gf3d_pipeline.chainLength);

    for (i =0; i < gf3d_pipeline.chainLength;i++)
    {
        if (vkCreateDescriptorPool(pipe->device, &poolInfo, NULL, &pipe->descriptorPool[i]) != VK_SUCCESS)
        {
            slog("failed to create descriptor pool!");
            return;
        }
    }
    pipe->descriptorPoolCount = gf3d_pipeline.chainLength;
}

void gf3d_pipeline_reset_frame(Pipeline *pipe,Uint32 frame)
{
    if (!pipe)return;
    if (frame >= gf3d_pipeline.chainLength)
    {
        slog("frame %i outside the range of supported descriptor Pools (%i)",frame,gf3d_pipeline.chainLength);
        return;
    }
    pipe->descriptorCursor[frame] = 0;
}

void gf3d_pipeline_create_descriptor_sets(Pipeline *pipe)
{
    int i;
    int r;
    VkDescriptorSetLayout *layouts = NULL;
    VkDescriptorSetAllocateInfo allocInfo = {0};

    slog("making descriptor");
    layouts = (VkDescriptorSetLayout *)gfc_allocate_array(sizeof(VkDescriptorSetLayout),pipe->descriptorSetCount);
    for (i = 0; i < pipe->descriptorSetCount; i++)
    {
        memcpy(&layouts[i],&pipe->descriptorSetLayout,sizeof(VkDescriptorSetLayout));
    }
    
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = pipe->descriptorSetCount;
    allocInfo.pSetLayouts = layouts;
    
    pipe->descriptorCursor = (Uint32 *)gfc_allocate_array(sizeof(Uint32),gf3d_pipeline.chainLength);
    pipe->descriptorSets = (VkDescriptorSet **)gfc_allocate_array(sizeof(VkDescriptorSet*),gf3d_pipeline.chainLength);

    for (i = 0; i < gf3d_pipeline.chainLength; i++)
    {    
        pipe->descriptorSets[i] = (VkDescriptorSet *)gfc_allocate_array(sizeof(VkDescriptorSet),pipe->descriptorSetCount);
        allocInfo.descriptorPool = pipe->descriptorPool[i];
        slog("allocating descriptor sets");
        if ((r = vkAllocateDescriptorSets(pipe->device, &allocInfo, pipe->descriptorSets[i])) != VK_SUCCESS)
        {
            slog("failed to allocate descriptor sets!");
            if (r == VK_ERROR_OUT_OF_POOL_MEMORY)slog("out of pool memory");
            else if (r == VK_ERROR_FRAGMENTED_POOL)slog("fragmented pool");
            else if (r == VK_ERROR_OUT_OF_DEVICE_MEMORY)slog("out of device memory");
            else if (r == VK_ERROR_OUT_OF_HOST_MEMORY)slog("out of host memory");
            free(layouts);
            return;
        }
    }
    pipe->descriptorSetCount = gf3d_pipeline.chainLength;
}

void gf3d_pipeline_create_basic_model_descriptor_set_layout(Pipeline *pipe)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {0};
    
    VkDescriptorSetLayoutBinding bindings[2];
    
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    memcpy(&bindings[1],&samplerLayoutBinding,sizeof(VkDescriptorSetLayoutBinding));

    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL; // Optional

    memcpy(&bindings[0],&uboLayoutBinding,sizeof(VkDescriptorSetLayoutBinding));

    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(pipe->device, &layoutInfo, NULL, &pipe->descriptorSetLayout) != VK_SUCCESS)
    {
        slog("failed to create descriptor set layout!");
    }
}

VkDescriptorSet * gf3d_pipeline_get_descriptor_set(Pipeline *pipe, Uint32 frame)
{
    if (!pipe)
    {
        slog("pipe not provided");
        return NULL;
    }
    if (frame >= gf3d_pipeline.chainLength)
    {
        slog("frame %i us out of the range of descriptor pools, limited to %i",frame,gf3d_pipeline.chainLength);
        return NULL;
    }
    if (pipe->descriptorCursor[frame] > pipe->descriptorSetCount)
    {
        slog("cannot allocate any more descriptor sets this frame!");
        return NULL;
    }
    return &pipe->descriptorSets[frame][pipe->descriptorCursor[frame]++];
}

/*eol@eof*/
