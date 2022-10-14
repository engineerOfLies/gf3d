#include <string.h>
#include <stdio.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gf3d_config.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_shaders.h"
#include "gf3d_model.h"
#include "gf3d_sprite.h"
#include "gf3d_pipeline.h"

extern int __DEBUG;

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
VkFormat gf3d_pipeline_find_depth_format();

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
    return VK_FORMAT_UNDEFINED;
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

int gf3d_pipeline_render_pass_create(VkDevice device,SJson *config,VkRenderPass *renderPass)
{
    SJson *item;
    const char *str;
    VkAttachmentDescription colorAttachment = {0};
    VkAttachmentReference colorAttachmentRef = {0};
    VkSubpassDescription subpass = {0};
    VkRenderPassCreateInfo renderPassInfo = {0};
    VkSubpassDependency dependency = {0};
    VkAttachmentDescription depthAttachment = {0};
    VkAttachmentReference depthAttachmentRef = {0};
    VkAttachmentDescription attachments[2];

    if (!config)return 0;
    if (!renderPass)return 0;
    

    item = sj_object_get_value(config,"depthAttachment");
    if (item)
    {
        depthAttachment = gf3d_config_attachment_description(item,gf3d_pipeline_find_depth_format());
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = depthAttachment.finalLayout;
    }
    item = sj_object_get_value(config,"colorAttachment");
    if (item)
    {
        colorAttachment = gf3d_config_attachment_description(item,gf3d_swapchain_get_format());
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = colorAttachment.finalLayout;
    }
    
    item = sj_object_get_value(config,"dependency");
    if (item)
    {
        dependency = gf3d_config_subpass_dependency(item);
    }
    
    item = sj_object_get_value(config,"subpass");
    if (item)
    {
        str = sj_object_get_value_as_string(item,"pipelineBindPoint");
        subpass.pipelineBindPoint = gf3d_config_pipeline_bindpoint_from_str(str);
    }
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
    
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, renderPass) != VK_SUCCESS)
    {
        slog("failed to create render pass!");
        return 0;
    }
    if (__DEBUG)slog("created renderpass for pipeline");
    return 1;
}


int gf3d_pipelin_depth_stencil_create_info_from_json(SJson *json,VkPipelineDepthStencilStateCreateInfo *depthStencil)
{
    short int b = VK_FALSE;
    float f = 0;
    if ((!json)|| (!depthStencil))return 0;
    depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil->pNext = NULL;
#if defined(VkPipelineDepthStencilStateCreateFlagBits)
    depthStencil->flags = gf3d_config_depth_stencil_create_flags(sj_object_get_value(json,"flags"));
#endif
    b = VK_FALSE;
    sj_get_bool_value(sj_object_get_value(json,"depthTestEnable"),&b);
    depthStencil->depthTestEnable = b;
    b = VK_FALSE;
    sj_get_bool_value(sj_object_get_value(json,"depthWriteEnable"),&b);
    depthStencil->depthWriteEnable = b;
    depthStencil->depthCompareOp = gf3d_config_compar_op_flag_from_str(sj_object_get_value_as_string(json,"depthCompareOp"));
    b = VK_FALSE;
    sj_get_bool_value(sj_object_get_value(json,"depthBoundsTestEnable"),&b);
    depthStencil->depthBoundsTestEnable = b;
    b = VK_FALSE;
    sj_get_bool_value(sj_object_get_value(json,"stencilTestEnable"),&b);
    depthStencil->stencilTestEnable = b;
    f = 0;
    sj_get_float_value(sj_object_get_value(json,"minDepthBounds"),&f);
    depthStencil->minDepthBounds = f;
    f = 0;
    sj_get_float_value(sj_object_get_value(json,"maxDepthBounds"),&f);
    depthStencil->maxDepthBounds = f;
    return 1;
}

Pipeline *gf3d_pipeline_create_from_config(
    VkDevice device,
    const char *configFile,
    VkExtent2D extent,
    Uint32 descriptorCount,
    const VkVertexInputBindingDescription* vertexInputDescription,
    const VkVertexInputAttributeDescription * vertextInputAttributeDescriptions,
    Uint32 vertexAttributeCount)
{
    SJson *config,*file, *item;
    const char *str;
    Pipeline *pipe;
    const char *vertFile = NULL;
    const char *fragFile = NULL;
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
    
    if (!vertexInputDescription)
    {
        slog("must provide vertexInputDescription to create the pipeline");
        return NULL;
    }
    if (!configFile)return NULL;
    file = sj_load(configFile);
    if (!file)
    {
        slog("failed to load config file for pipeline");
        return NULL;
    }
    config = sj_object_get_value(file,"pipeline");
    if (!config)
    {
        slog("failed to load config file for pipeline, missing pipeline object");
        sj_free(file);
        return NULL;
    }
    
    pipe = gf3d_pipeline_new();
    if (!pipe)
    {
        slog("failed to get memory for a new pipeline");
        sj_free(file);
        return NULL;
    }

    vertFile = sj_object_get_value_as_string(config,"vertex_shader");
    if (vertFile)
    {
        pipe->vertShader = (char *)gf3d_shaders_load_data(vertFile,&pipe->vertSize);
        pipe->vertModule = gf3d_shaders_create_module(pipe->vertShader,pipe->vertSize,device);
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = pipe->vertModule;
        vertShaderStageInfo.pName = "main";
        shaderStages[0] = vertShaderStageInfo;
    }
    else
    {
        slog("no vertex_shader provided");
        sj_free(file);
        gf3d_pipeline_free(pipe);
        return NULL;
    }
    fragFile = sj_object_get_value_as_string(config,"fragment_shader");
    if (fragFile)
    {
        pipe->fragShader = (char *)gf3d_shaders_load_data(fragFile,&pipe->fragSize);
        pipe->fragModule = gf3d_shaders_create_module(pipe->fragShader,pipe->fragSize,device);
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = pipe->fragModule;
        fragShaderStageInfo.pName = "main";
        shaderStages[1] = fragShaderStageInfo;
    }
    else
    {
        slog("no vertex_shader provided");
        sj_free(file);
        gf3d_pipeline_free(pipe);
        return NULL;
    }

    pipe->device = device;
    pipe->descriptorSetCount = descriptorCount;
    
    gf3d_pipelin_depth_stencil_create_info_from_json(sj_object_get_value(config,"depthStencil"),&depthStencil);
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    str = sj_get_string_value(sj_object_get_value(config,"topology"));
    inputAssembly.topology = gf3d_config_primitive_topology_from_str(str);
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = vertexInputDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeCount;
    vertexInputInfo.pVertexAttributeDescriptions = vertextInputAttributeDescriptions;

    
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
    
    rasterizer = gf3d_config_pipline_rasterization_state_create_info(sj_object_get_value(config,"rasterizer"));

    multisampling = gf3d_config_pipline_multisample_state_create_info(sj_object_get_value(config,"multisampling"));

    colorBlendAttachment = gf3d_config_pipeline_color_blend_attachment(sj_object_get_value(config,"colorBlendAttachment"));

    
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional
    
    //NOTE: Really gonna have to look closer at these to see how they can be driven from config, or passed the
    // the information needed to make this work generically
    gf3d_pipeline_create_basic_model_descriptor_pool(pipe);
    gf3d_pipeline_create_basic_model_descriptor_set_layout(pipe);
    gf3d_pipeline_create_descriptor_sets(pipe);
    
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &pipe->descriptorSetLayout; // Optional 
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    item = sj_object_get_value(config,"renderPass");
    if (!item)
    {
        slog("failed to create pipeline layout!");
        sj_free(file);
        gf3d_pipeline_free(pipe);
        return NULL;
    }
    if (!gf3d_pipeline_render_pass_create(device,item,&pipe->renderPass))
    {
        slog("failed to create pipeline layout!");
        sj_free(file);
        gf3d_pipeline_free(pipe);
        return NULL;
    }
    
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipe->pipelineLayout) != VK_SUCCESS)
    {
        slog("failed to create pipeline layout!");
        sj_free(file);
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
    
    sj_free(file);
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipe->pipeline) != VK_SUCCESS)
    {   
        slog("failed to create pipeline!");
        
        gf3d_pipeline_free(pipe);
        return NULL;
    }
    if (__DEBUG)slog("pipeline created from file '%s'",configFile);
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
