#include <string.h>
#include <stdio.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_pak.h"

#include "gf3d_config.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_shaders.h"
#include "gf3d_pipeline.h"

#include "gf3d_mesh.h"

extern int __DEBUG;

typedef struct
{
    Uint32              maxPipelines;
    Pipeline           *pipelineGFC_List;
    Uint32              chainLength;
}PipelineManager;

static PipelineManager gf3d_pipeline = {0};

void gf3d_pipeline_close();
void gf3d_pipeline_create_basic_descriptor_pool(
    Pipeline *pipe,
    SJson *descriptorSetLayouts,
    VkDescriptorPoolSize *poolSize,
    int poolSizeCount);
void gf3d_pipeline_create_basic_descriptor_pool_from_config(Pipeline *pipe,SJson *config);
void gf3d_pipeline_create_basic_descriptor_set_layout_from_config(Pipeline *pipe,SJson *config);
void gf3d_pipeline_create_descriptor_sets(Pipeline *pipe);
void gf3d_pipeline_ubo_data_free(PipelineUboData *pipeUbo);
VkFormat gf3d_pipeline_find_depth_format();

void gf3d_pipeline_init(Uint32 max_pipelines)
{
    if (max_pipelines == 0)
    {
        slog("cannot initialize zero pipelines");
        return;
    }
    gf3d_pipeline.pipelineGFC_List = (Pipeline *)gfc_allocate_array(sizeof(Pipeline),max_pipelines);
    if (!gf3d_pipeline.pipelineGFC_List)
    {
        slog("failed to allocate pipeline manager");
        return;
    }
    gf3d_pipeline.maxPipelines = max_pipelines;
    gf3d_pipeline.chainLength = gf3d_swapchain_get_swap_image_count();
    atexit(gf3d_pipeline_close);
    if (__DEBUG)slog("pipeline system initialized");
}

void gf3d_pipeline_close()
{
    int i;
    if (gf3d_pipeline.pipelineGFC_List != 0)
    {
        for (i = 0; i < gf3d_pipeline.maxPipelines; i++)
        {
            gf3d_pipeline_free(&gf3d_pipeline.pipelineGFC_List[i]);
        }
        free(gf3d_pipeline.pipelineGFC_List);
    }
    memset(&gf3d_pipeline,0,sizeof(PipelineManager));
    if (__DEBUG)slog("pipeline system closed");
}

void gf3d_pipeline_call_render(
    Pipeline *pipe,
    VkDescriptorSet * descriptorSet,
    VkBuffer vertexBuffer,
    Uint32 vertexCount,
    VkBuffer indexBuffer)
{
    VkDeviceSize offsets[] = {0};
    if ((!pipe)||(!descriptorSet))return;
    vkCmdBindVertexBuffers(pipe->commandBuffer, 0, 1, &vertexBuffer, offsets);
    if (indexBuffer != VK_NULL_HANDLE)vkCmdBindIndexBuffer(pipe->commandBuffer, indexBuffer, 0, pipe->indexType);
    vkCmdBindDescriptorSets(pipe->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe->pipelineLayout, 0, 1, descriptorSet, 0, NULL);
    if (indexBuffer != VK_NULL_HANDLE)vkCmdDrawIndexed(pipe->commandBuffer, vertexCount, 1, 0, 0, 0);
    else vkCmdDraw(pipe->commandBuffer, vertexCount,1,0,0);
}

void gf3d_pipeline_update_descriptor_set(Pipeline *pipe, PipelineDrawCall *drawCall)
{
    int i;
    Texture *texture;
    Uint32 uboCount = 0,textureCount = 0;
    int frame;
    PipelineUboData *ubuBufferData;
    PipelineTuple *tuple;
    UniformBuffer *buffer;
    VkDescriptorImageInfo imageInfo = {0};
    VkWriteDescriptorSet *descriptorWrite = NULL;
    VkDescriptorBufferInfo bufferInfo = {0};
    if ((!pipe)||(!drawCall))return;    

    frame = gf3d_vgraphics_get_current_buffer_frame();
    uboCount = gfc_list_get_count(drawCall->uboDataList);
    textureCount = gfc_list_get_count(drawCall->textureDataList);

    if (!uboCount)
    {
        if (__DEBUG)slog("no ubos for render call for pipeline %s",pipe->name);
        return;
    }
    
    descriptorWrite = gfc_allocate_array(sizeof(VkWriteDescriptorSet),uboCount + textureCount);
    if (!descriptorWrite)
    {
        if (__DEBUG)slog("failed to allocate descriptorWrite in render call for pipeline %s",pipe->name);
        return;
    }

    for (i = 0; i < uboCount; i++)
    {
        tuple = gfc_list_get_nth(drawCall->uboDataList,i);
        if (!tuple)continue;
        ubuBufferData = gfc_list_get_nth(pipe->uboList,tuple->uboIndex);
        if (!ubuBufferData)continue;
        //get the buffer for this frame for this ubo
        buffer = gf3d_uniform_buffer_list_get_nth_buffer(ubuBufferData->uboBigBuffer, 0, frame);
        if (!buffer)continue;
        //note: I may be able to pack my multiple UBOs in here with offsets.

        bufferInfo.buffer = buffer->uniformBuffer;
        bufferInfo.offset = drawCall->index * ubuBufferData->uboDataSize;
        bufferInfo.range = ubuBufferData->uboDataSize;

        descriptorWrite[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[i].dstSet = *(drawCall->descriptorSet);
        descriptorWrite[i].dstBinding = tuple->index;
        descriptorWrite[i].dstArrayElement = 0;
        descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[i].descriptorCount = 1;
        descriptorWrite[i].pBufferInfo = &bufferInfo;
        slog("descriptorWrite written for ubo %i at bind %i and memory size %i",i,tuple->index,bufferInfo.range);
    }    

    //this is going to iterate through the textureDataList
    for (i = 0; i < textureCount;i++)
    {
        tuple = gfc_list_get_nth(drawCall->textureDataList,i);
        if (!tuple)continue;
        texture = (Texture *)tuple->data;

        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->textureImageView;
        imageInfo.sampler = texture->textureSampler;
        descriptorWrite[i + uboCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[i + uboCount].dstSet = *drawCall->descriptorSet;
        descriptorWrite[i + uboCount].dstBinding = tuple->index;
        descriptorWrite[i + uboCount].dstArrayElement = 0;
        descriptorWrite[i + uboCount].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[i + uboCount].descriptorCount = 1;                        
        descriptorWrite[i + uboCount].pImageInfo = &imageInfo;
        descriptorWrite[i + uboCount].pTexelBufferView = NULL; // Optional
    }
    slog("updating %i elements in the pDescriptorWrites array.",uboCount + textureCount);
    vkUpdateDescriptorSets(pipe->device, uboCount + textureCount, descriptorWrite, 0, NULL);
    free(descriptorWrite);
}

void gf3d_pipeline_render_drawcall(Pipeline *pipe,PipelineDrawCall *drawCall)
{
    if ((!pipe)||(!drawCall))return;
    gf3d_pipeline_call_render(
        pipe,
        drawCall->descriptorSet,
        drawCall->vertexBuffer,
        drawCall->vertexCount,
        drawCall->indexBuffer);
}

void gf3d_pipeline_render_all_drawcalls(Pipeline *pipe)
{
    int i;
    if (!pipe)return;
    for (i = 0; i < pipe->drawCallCount; i++)
    {
        if (!pipe->drawCallList[i].inuse)continue;
        gf3d_pipeline_render_drawcall(pipe,&pipe->drawCallList[i]);
    }
}


void gf3d_pipeline_update_descriptor_sets(Pipeline *pipe)
{
    int i;
    for (i = 0;i < pipe->drawCallCount;i++)
    {
        if (!pipe->drawCallList[i].inuse)continue;
        gf3d_pipeline_update_descriptor_set(pipe, &pipe->drawCallList[i]);
    }    
}

PipelineDrawCall *gf3d_pipeline_draw_call_new(Pipeline *pipe)
{
    int i;
    if (!pipe)return NULL;
    for (i = 0;i < pipe->descriptorSetCount;i++)
    {
        if (pipe->drawCallList[i].inuse)continue;
        pipe->drawCallList[i].inuse = 1;
        pipe->drawCallCount++;
        pipe->drawCallList[i].index = i;
        return &pipe->drawCallList[i];
    }
    if (__DEBUG)slog("cannot queue up any more draw calls this frame");
    return NULL;
}

void gf3d_pipeline_queue_render(
    Pipeline *pipe,
    VkBuffer vertexBuffer,
    Uint32 vertexCount,
    VkBuffer indexBuffer,
    //list of PipelineTuples containing uboData
    GFC_List *uboDataList,
    //list of PipelineTuple containing texture data
    GFC_List *textureList)
{
    int i,c;
    char *ptr;
    MaterialUBO *material;
    MeshUBO *meshUbo;
    PipelineUboData *uboData;
    PipelineTuple *tuple;
    PipelineDrawCall *drawCall;
    if (!pipe)return;
    drawCall = gf3d_pipeline_draw_call_new(pipe);
    if (!drawCall)
    {
        slog("failed to get a drawcall for pipeline");
        return;
    }
    drawCall->descriptorSet = gf3d_pipeline_get_descriptor_set(pipe, gf3d_vgraphics_get_current_buffer_frame());
    drawCall->vertexBuffer = vertexBuffer;
    drawCall->vertexCount = vertexCount;
    drawCall->indexBuffer = indexBuffer;
    //handle lists
    drawCall->textureDataList = textureList;
    //need to copy the ubo data into the bigBuffer and update the tuple pointer to THAT data
    c = gfc_list_get_count(uboDataList);
    for (i = 0;i < c; i++)
    {
        tuple = gfc_list_get_nth(uboDataList,i);
        if (!tuple)continue;
        uboData = gfc_list_get_nth(pipe->uboList,tuple->uboIndex);
        if (!uboData)continue;//should probably remove this ubo from the call at this point
        ptr = uboData->uboData;
        ptr = ptr + (drawCall->index * uboData->uboDataSize);
        memcpy(ptr,tuple->data,uboData->uboDataSize);
    }
    drawCall->uboDataList = uboDataList;
}

void gf3_pipeline_update_ubos(Pipeline *pipe)
{
    int i,c;
    PipelineUboData *uboData;
    int frame;
    void *data;
    UniformBuffer *buffer;
    VkDevice device;
    if ((!pipe)||(!pipe->drawCallCount))return;//skip if there are no queued calls

    device = gf3d_vgraphics_get_default_logical_device();
    frame = gf3d_vgraphics_get_current_buffer_frame();
    
    //for each ubo data set
    c = gfc_list_get_count(pipe->uboList);
    for (i = 0; i < c;i++)
    {
        uboData = gfc_list_get_nth(pipe->uboList,i);
        if (!uboData)continue;
        buffer = gf3d_uniform_buffer_list_get_nth_buffer(uboData->uboBigBuffer, 0, frame);
        vkMapMemory(device, buffer->uniformBufferMemory, 0, buffer->bufferSize, 0, &data);
            memcpy(data, uboData->uboData, buffer->bufferSize);
        vkUnmapMemory(device, buffer->uniformBufferMemory);
    }
}

Pipeline *gf3d_pipeline_new()
{
    int i;
    for (i = 0; i < gf3d_pipeline.maxPipelines; i++)
    {
        if (gf3d_pipeline.pipelineGFC_List[i].inUse)continue;
        gf3d_pipeline.pipelineGFC_List[i].inUse = true;
        gf3d_pipeline.pipelineGFC_List[i].uboList = gfc_list_new();
        return &gf3d_pipeline.pipelineGFC_List[i];
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
    

    item = sj_object_get_value(config,"colorAttachment");
    if (item)
    {
        colorAttachment = gf3d_config_attachment_description(item,gf3d_swapchain_get_format());
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = colorAttachment.finalLayout;
    }
    item = sj_object_get_value(config,"depthAttachment");
    if (item)
    {
        depthAttachment = gf3d_config_attachment_description(item,gf3d_pipeline_find_depth_format());
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = depthAttachment.finalLayout;
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

void gf3d_pipeline_ubo_data_free(PipelineUboData *pipeUbo)
{
    if (!pipeUbo)return;
    
    if (pipeUbo->uboBigBuffer)
    {
        gf3d_uniform_buffer_list_free(pipeUbo->uboBigBuffer);
    }
    if (pipeUbo->uboData)free(pipeUbo->uboData);
    free(pipeUbo);
}

PipelineUboData *gf3d_pipeline_ubo_data_new()
{
    PipelineUboData *pipeUbo;
    pipeUbo = gfc_allocate_array(sizeof(PipelineUboData),1);
    return pipeUbo;
}

PipelineUboData *gf3d_pipeline_ubo_data(Pipeline *pipe,Uint32 descriptorCount,size_t bufferSize)
{
    PipelineUboData *pipeUbo;
    pipeUbo = gf3d_pipeline_ubo_data_new();
    if (!pipeUbo)return NULL;
    pipeUbo->uboData = gfc_allocate_array(bufferSize,descriptorCount);
    pipeUbo->uboDataSize = bufferSize;
    pipeUbo->uboBigBuffer = gf3d_uniform_buffer_list_new(pipe->device,bufferSize*descriptorCount,1,gf3d_swapchain_get_swap_image_count());
    return pipeUbo;
}

Pipeline *gf3d_pipeline_create_from_config(
    VkDevice device,
    const char *configFile,
    VkExtent2D extent,
    Uint32 descriptorCount,
    const VkVertexInputBindingDescription* vertexInputDescription,
    const VkVertexInputAttributeDescription * vertextInputAttributeDescriptions,
    Uint32 vertexAttributeCount,
    VkDeviceSize *bufferSizes,
    Uint32 unformBufferCount,
    VkIndexType indexType
)
{
    int i;
    PipelineUboData *pipeUbo;
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
    file = gfc_pak_load_json(configFile);
    if (!file)
    {
        slog("failed to load config file for pipeline %s",configFile);
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
    
    sj_object_get_value_as_uint32(config,"descriptorCount",&descriptorCount);
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
    
    gf3d_pipeline_create_basic_descriptor_pool_from_config(pipe,config);
    gf3d_pipeline_create_basic_descriptor_set_layout_from_config(pipe,config);
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
    pipe->drawCallList = gfc_allocate_array(sizeof(PipelineDrawCall),descriptorCount);
    
    for (i = 0; i < unformBufferCount;i++)
    {
        pipeUbo = gf3d_pipeline_ubo_data(pipe,descriptorCount,bufferSizes[i]);
        if (!pipeUbo)
        {
            slog("failed to create pipeline ubo buffers");
            continue;
        }
        gfc_list_append(pipe->uboList,pipeUbo);
    }

    gfc_line_cpy(pipe->name,configFile);
    pipe->indexType = indexType;
    if (__DEBUG)slog("pipeline created from file '%s'",configFile);
    return pipe;
}

void gf3d_pipeline_free(Pipeline *pipe)
{
    int i;
    if (!pipe)return;
    if (!pipe->inUse)return;
    if (pipe->drawCallList)
    {
        free(pipe->drawCallList);
    }
    if (pipe->uboList)
    {
        gfc_list_foreach(pipe->uboList,(gfc_work_func*)gf3d_pipeline_ubo_data_free);
        gfc_list_delete(pipe->uboList);
    }
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

void gf3d_pipeline_create_basic_descriptor_pool_from_config(Pipeline *pipe,SJson *config)
{
    const char *str;
    int i,c,pools = 0;
    VkDescriptorPoolSize *poolSize;
    SJson *list,*item;
    
    if ((!pipe)||(!config))
    {
        slog("no pipeline or config provided");
        return;
    }
    list = sj_object_get_value(config,"descriptorSetLayout");
    if (!list)
    {
        slog("no descriptorSetLayout found in config");
        return;
    }
    c = sj_array_get_count(list);
    if (!c)return;// no descriptorSetLayouts
    poolSize = gfc_allocate_array(sizeof(VkDescriptorPoolSize),c);
    for (i = 0,pools = 0;i < c;i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        str = sj_object_get_value_as_string(item,"descriptorType");
        if (!str)continue;
        poolSize[pools].type = gf3d_config_descriptor_type_from_str(str);
        poolSize[pools].descriptorCount = pipe->descriptorSetCount;//chain length
        pools++;
    }
    
    gf3d_pipeline_create_basic_descriptor_pool(pipe,list,poolSize,pools);
    free(poolSize);
}


void gf3d_pipeline_create_basic_descriptor_pool(
    Pipeline *pipe,
    SJson *descriptorSetLayouts,
    VkDescriptorPoolSize *poolSize,
    int poolSizeCount)
{
    int i,c;
    const char *str;
    SJson *item;
    VkDescriptorPoolCreateInfo poolInfo = {0};
    
    if (!pipe)
    {
        slog("no pipeline provided");
        return;
    }
    c = sj_array_get_count(descriptorSetLayouts);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(descriptorSetLayouts,i);
        if (!item)continue;
        str = sj_object_get_value_as_string(item,"descriptorType");
        if (!str)continue;
        poolSize[i].type = gf3d_config_descriptor_type_from_str(str);
        poolSize[i].descriptorCount = pipe->descriptorSetCount;
    }
    
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizeCount;
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

void gf3d_pipeline_reset_all_pipes()
{
    int i;
    Uint32 bufferFrame = gf3d_vgraphics_get_current_buffer_frame();
    for (i = 0; i < gf3d_pipeline.maxPipelines;i++)
    {
        if (!gf3d_pipeline.pipelineGFC_List[i].inUse)continue;
        gf3d_pipeline_reset_frame(&gf3d_pipeline.pipelineGFC_List[i],bufferFrame);
    }
}

void gf3d_pipeline_reset_frame(Pipeline *pipe,Uint32 frame)
{
    int i,c;
    PipelineUboData *uboData;
    if (!pipe)return;
    if (frame >= gf3d_pipeline.chainLength)
    {
        slog("frame %i outside the range of supported descriptor Pools (%i)",frame,gf3d_pipeline.chainLength);
        return;
    }
    pipe->descriptorCursor[frame] = 0;
    
    pipe->commandBuffer = gf3d_command_rendering_begin(frame,pipe);
    pipe->drawCallCount = 0;
    //need to clear all the tuples used this frame before wiping the list
    for (i = 0; i < pipe->descriptorSetCount;i++)
    {
        if (!pipe->drawCallList[i].inuse)continue;
        gf3d_pipeline_tuple_list_delete(pipe->drawCallList[i].uboDataList);
        gf3d_pipeline_tuple_list_delete(pipe->drawCallList[i].textureDataList);
    }
    memset(pipe->drawCallList,0,sizeof(PipelineDrawCall)*pipe->descriptorSetCount);//clear this out
    c = gfc_list_get_count(pipe->uboList);
    for (i = 0;i < c; i++)
    {
        uboData = gfc_list_get_nth(pipe->uboList,i);
        if (!uboData)continue;
        memset(uboData->uboData,0,uboData->uboDataSize*pipe->descriptorSetCount);
    }
}

void gf3d_pipeline_submit_commands(Pipeline *pipe)
{
    if (!pipe)return;
    gf3d_command_rendering_end(pipe->commandBuffer);
}

void gf3d_pipeline_submit_all_pipe_commands()
{
    int i;
    for (i = 0; i < gf3d_pipeline.maxPipelines;i++)
    {
        if (!gf3d_pipeline.pipelineGFC_List[i].inUse)continue;
        //Update UBOS
        gf3_pipeline_update_ubos(&gf3d_pipeline.pipelineGFC_List[i]);
        //Update Descriptor sets
        gf3d_pipeline_update_descriptor_sets(&gf3d_pipeline.pipelineGFC_List[i]);
        //Set commands
        gf3d_pipeline_render_all_drawcalls(&gf3d_pipeline.pipelineGFC_List[i]);
        //submit commands
        gf3d_pipeline_submit_commands(&gf3d_pipeline.pipelineGFC_List[i]);
    }
}

void gf3d_pipeline_create_descriptor_sets(Pipeline *pipe)
{
    int i;
    int r;
    VkDescriptorSetLayout *layouts = NULL;
    VkDescriptorSetAllocateInfo allocInfo = {0};

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

void gf3d_pipeline_create_basic_descriptor_set_layout_from_config(Pipeline *pipe,SJson *config)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};    
    VkDescriptorSetLayoutBinding *bindings;
    SJson *list,*item;
    int i,c;
    if (!pipe)
    {
        slog("no pipe specified");
        return;
    }
    if (!config)
    {
        slog("no config specified");
        return;
    }
    list = sj_object_get_value(config,"descriptorSetLayout");
    if (!list)
    {
        slog("config does not contain descriptorSetLayout");
        return;
    }
    c = sj_array_get_count(list);
    if (!c)
    {
        slog("descriptorSetLayout empty");
        return;
    }
    bindings = gfc_allocate_array(sizeof(VkDescriptorSetLayoutBinding),c);
    for (i = 0;i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        sj_object_get_value_as_uint32(item,"binding",&bindings[i].binding);
        sj_object_get_value_as_uint32(item,"descriptorCount",&bindings[i].descriptorCount);
        bindings[i].descriptorType = gf3d_config_descriptor_type_from_str(sj_object_get_value_as_string(item,"descriptorType"));
        bindings[i].stageFlags = gf3d_config_shader_stage_flags(sj_object_get_value(item,"stageFlags"));
    }

    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = c;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(pipe->device, &layoutInfo, NULL, &pipe->descriptorSetLayout) != VK_SUCCESS)
    {
        slog("failed to create descriptor set layout!");
    }
    free(bindings);
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
        slog("frame %i is out of the range of descriptor pools, limited to %i",frame,gf3d_pipeline.chainLength);
        return NULL;
    }
    if (pipe->descriptorCursor[frame] > pipe->descriptorSetCount)
    {
        slog("cannot allocate any more descriptor sets this frame!");
        return NULL;
    }
    return &pipe->descriptorSets[frame][pipe->descriptorCursor[frame]++];
}

void gf3d_pipeline_tuple_free(PipelineTuple *tuple)
{
    if (!tuple)return;
    free(tuple);//the tuple doesn't own its data
}

PipelineTuple *gf3d_pipeline_tuple_duplicate(PipelineTuple *tupleIn)
{
    PipelineTuple *tupleOut;
    if (!tupleIn)return NULL;
    tupleOut = gf3d_pipeline_tuple_new();
    if (!tupleOut)return NULL;
    memcpy(tupleOut,tupleIn,sizeof(PipelineTuple));
    return tupleOut;
}

PipelineTuple *gf3d_pipeline_tuple_new()
{
    PipelineTuple *tuple;
    tuple = gfc_allocate_array(sizeof(PipelineTuple),1);
    return tuple;
}

PipelineTuple *gf3d_pipeline_tuple(Uint32 index,void *data)
{
    PipelineTuple *tuple;
    tuple = gf3d_pipeline_tuple_new();
    if (!tuple)return NULL;
    tuple->index = index;
    tuple->data = data;
    return tuple;
}

GFC_List *gf3d_pipeline_duplicate_tuple_list(GFC_List *list)
{
    int i,c;
    PipelineTuple *tuple;
    GFC_List *outList;
    if (!list)return NULL;
    outList = gfc_list_new();
    c = gfc_list_get_count(list);
    for (i = 0;i < c; i++)
    {
        tuple = gfc_list_get_nth(list,i);
        if (!tuple)continue;
        gfc_list_append(outList,gf3d_pipeline_tuple_duplicate(tuple));
    }
    return outList;
}

void gf3d_pipeline_tuple_list_delete(GFC_List *list)
{
    if (!list)return;
    gfc_list_foreach(list,(gfc_work_func*)gf3d_pipeline_tuple_free);
    gfc_list_delete(list);
}

/*eol@eof*/
