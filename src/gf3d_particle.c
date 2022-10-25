#include "simple_logger.h"

#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_buffers.h"

#include "gf3d_particle.h"

#define PARTICLE_ATTRIBUTE_COUNT 1

typedef struct
{
    Matrix4     model;
    Matrix4     view;
    Matrix4     proj;
    Vector4D    color;
    Vector2D    viewportSize;
    float       size;
}ParticleUBO;

typedef struct
{
    VkVertexInputAttributeDescription   attributeDescriptions[PARTICLE_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
    VkBuffer                    buffer;                 /**<vertex buffer for particles (just one vertex)*/
    VkDeviceMemory              bufferMemory;           /**<memory handle for the vertex buffer*/
    Pipeline *pipe;
}ParticleManager;


static ParticleManager gf3d_particle = {0};

void gf3d_particle_create_vertex_buffer();

void gf3d_particles_manager_close()
{
    if (gf3d_particle.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_vgraphics_get_default_logical_device(), gf3d_particle.buffer, NULL);
    }
    if (gf3d_particle.bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_vgraphics_get_default_logical_device(), gf3d_particle.bufferMemory, NULL);
    }
    memset(&gf3d_particle,0,sizeof(ParticleManager));
}

void gf3d_particle_manager_init(Uint32 max_particles)
{
    gf3d_particle.bindingDescription.binding = 0;
    gf3d_particle.bindingDescription.stride = sizeof(Vector3D);
    gf3d_particle.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gf3d_particle.attributeDescriptions[0].binding = 0;
    gf3d_particle.attributeDescriptions[0].location = 0;
    gf3d_particle.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_particle.attributeDescriptions[0].offset = 0;

    gf3d_particle_create_vertex_buffer();
    gf3d_particle.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/particle_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        max_particles,
        &gf3d_particle.bindingDescription,
        gf3d_particle.attributeDescriptions,
        PARTICLE_ATTRIBUTE_COUNT,
        sizeof(ParticleUBO)
    );
    slog("particle manager initiliazed");
    atexit(gf3d_particles_manager_close);
}

void gf3d_particle_reset_pipes()
{
    Uint32 bufferFrame = gf3d_vgraphics_get_current_buffer_frame();
    
    gf3d_pipeline_reset_frame(gf3d_particle.pipe,bufferFrame);
}

void gf3d_particle_submit_pipe_commands()
{
    gf3d_pipeline_submit_commands(gf3d_particle.pipe);
}


Pipeline *gf3d_particle_get_pipeline()
{
    return gf3d_particle.pipe;
}

void gf3d_particle_create_vertex_buffer()
{
    void *data = NULL;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    Vector3D particle = {0};

    bufferSize = sizeof(Vector3D);
    
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, &particle, (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gf3d_particle.buffer, &gf3d_particle.bufferMemory);

    gf3d_buffer_copy(stagingBuffer, gf3d_particle.buffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);    
}


VkVertexInputBindingDescription * gf3d_particle_get_bind_description()
{
    return &gf3d_particle.bindingDescription;
}

VkVertexInputAttributeDescription * gf3d_particle_get_attribute_descriptions(Uint32 *count)
{    
    if (count)*count = 1;
    return gf3d_particle.attributeDescriptions;
}

void gf3d_particle_update_uniform_buffer(Particle *particle,UniformBuffer *ubo)
{
    void* data;
    ParticleUBO particleUBO = {0};
    UniformBufferObject graphics_ubo;
    
    graphics_ubo = gf3d_vgraphics_get_uniform_buffer_object();
    
    gfc_matrix_identity(particleUBO.model);
    gfc_matrix_translate(particleUBO.model,particle->position);    
    
    gfc_matrix_copy(particleUBO.view,graphics_ubo.view);
    gfc_matrix_copy(particleUBO.proj,graphics_ubo.proj);
    
    vector4d_copy(particleUBO.color,gfc_color_to_vector4f(particle->color));
    particleUBO.size = particle->size;
    particleUBO.viewportSize = gf3d_vgraphics_get_view_extent_as_vector2d();
    
    vkMapMemory(gf3d_vgraphics_get_default_logical_device(), ubo->uniformBufferMemory, 0, sizeof(ParticleUBO), 0, &data);
    
        memcpy(data, &particleUBO, sizeof(ParticleUBO));

    vkUnmapMemory(gf3d_vgraphics_get_default_logical_device(), ubo->uniformBufferMemory);
}

void gf3d_particle_update_basic_descriptor_set(
    Particle *particle,
    VkDescriptorSet descriptorSet,
    Uint32 chainIndex)
{
    VkWriteDescriptorSet descriptorWrite[2] = {0};
    VkDescriptorBufferInfo bufferInfo = {0};
    UniformBuffer *ubo;

    if (!particle)
    {
        slog("no particle provided for descriptor set update");
        return;
    }
    if (descriptorSet == VK_NULL_HANDLE)
    {
        slog("null handle provided for descriptorSet");
        return;
    }

    ubo = gf3d_uniform_buffer_list_get_buffer(gf3d_particle.pipe->uboList, chainIndex);
    if (!ubo)
    {
        slog("failed to get a uniform buffer for particle descriptor");
        return;
    }
    gf3d_particle_update_uniform_buffer(particle,ubo);

    bufferInfo.buffer = ubo->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(ParticleUBO);        
    
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = descriptorSet;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(gf3d_vgraphics_get_default_logical_device(), 1, descriptorWrite, 0, NULL);
}

void gf3d_particle_render(Particle *particle,VkCommandBuffer commandBuffer, VkDescriptorSet * descriptorSet)
{
    VkDeviceSize offsets[] = {0};
    if (!particle)
    {
        slog("cannot render a NULL particle");
        return;
    }
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &gf3d_particle.buffer, offsets);
        
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        gf3d_particle.pipe->pipelineLayout, 0, 1, descriptorSet, 0, NULL);
    
    vkCmdDrawIndexed(commandBuffer, 1, 1, 0, 0, 0);
}

void gf3d_particle_draw(Particle *particle)
{
    VkDescriptorSet *descriptorSet = NULL;
    Uint32 buffer_frame;
    VkCommandBuffer commandBuffer;

    if (!particle)
    {
        slog("cannot render a NULL particle");
        return;
    }
    commandBuffer = gf3d_particle.pipe->commandBuffer;
    buffer_frame = gf3d_vgraphics_get_current_buffer_frame();

    descriptorSet = gf3d_pipeline_get_descriptor_set(gf3d_particle.pipe, buffer_frame);
    if (descriptorSet == NULL)
    {
        slog("failed to get a free descriptor Set for sprite rendering");
        return;
    }

    gf3d_particle_update_basic_descriptor_set(
        particle,
        *descriptorSet,
        buffer_frame);
    gf3d_particle_render(particle,commandBuffer,descriptorSet);
}

/**
 * @brief draw a list of particles this frame
 * @param list list of GF3D_Particle's
 */
void gf3d_particles_draw_list(List *list);


/*eol@eof*/
