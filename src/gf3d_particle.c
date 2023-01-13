#include "simple_logger.h"

#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_buffers.h"

#include "gf3d_particle.h"

#define PARTICLE_ATTRIBUTE_COUNT 1
extern int __DEBUG;
typedef struct
{
    Matrix4     model;
    Matrix4     view;
    Matrix4     proj;
    Vector4D    color;
    Vector2D    viewportSize;
    float       size;
    float       padding;
}ParticleUBO;

typedef struct
{
    VkVertexInputAttributeDescription   attributeDescriptions[PARTICLE_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
    VkBuffer                    buffer;                 /**<vertex buffer for particles (just one vertex)*/
    VkDeviceMemory              bufferMemory;           /**<memory handle for the vertex buffer*/
    Pipeline *pipe;
}ParticleManager;


static ParticleManager gf3d_particle_manager = {0};

void gf3d_particle_create_vertex_buffer();


Particle gf3d_particle(Vector3D position, Color color, float size)
{
    Particle p = {position, color, size};
    return p;
}


void gf3d_particles_manager_close()
{
    if (gf3d_particle_manager.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_vgraphics_get_default_logical_device(), gf3d_particle_manager.buffer, NULL);
    }
    if (gf3d_particle_manager.bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_vgraphics_get_default_logical_device(), gf3d_particle_manager.bufferMemory, NULL);
    }
    memset(&gf3d_particle_manager,0,sizeof(ParticleManager));
}

void gf3d_particle_manager_init(Uint32 max_particles)
{
    gf3d_particle_manager.bindingDescription.binding = 0;
    gf3d_particle_manager.bindingDescription.stride = sizeof(Vector3D);
    gf3d_particle_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gf3d_particle_manager.attributeDescriptions[0].binding = 0;
    gf3d_particle_manager.attributeDescriptions[0].location = 0;
    gf3d_particle_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_particle_manager.attributeDescriptions[0].offset = 0;

    gf3d_particle_create_vertex_buffer();
    gf3d_particle_manager.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/particle_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        max_particles,
        &gf3d_particle_manager.bindingDescription,
        gf3d_particle_manager.attributeDescriptions,
        PARTICLE_ATTRIBUTE_COUNT,
        sizeof(ParticleUBO),
        VK_INDEX_TYPE_UINT16
    );
    if(__DEBUG)slog("particle manager initiliazed");
    atexit(gf3d_particles_manager_close);
}

void gf3d_particle_reset_pipes()
{
    Uint32 bufferFrame = gf3d_vgraphics_get_current_buffer_frame();
    
    gf3d_pipeline_reset_frame(gf3d_particle_manager.pipe,bufferFrame);
}

void gf3d_particle_submit_pipe_commands()
{
    gf3d_pipeline_submit_commands(gf3d_particle_manager.pipe);
}


Pipeline *gf3d_particle_get_pipeline()
{
    return gf3d_particle_manager.pipe;
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

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gf3d_particle_manager.buffer, &gf3d_particle_manager.bufferMemory);

    gf3d_buffer_copy(stagingBuffer, gf3d_particle_manager.buffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);    
}


VkVertexInputBindingDescription * gf3d_particle_get_bind_description()
{
    return &gf3d_particle_manager.bindingDescription;
}

VkVertexInputAttributeDescription * gf3d_particle_get_attribute_descriptions(Uint32 *count)
{    
    if (count)*count = 1;
    return gf3d_particle_manager.attributeDescriptions;
}

ParticleUBO gf3d_particle_get_uniform_buffer(Particle *particle)
{
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
    return particleUBO;
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

void gf3d_particle_draw(Particle particle)
{
    ParticleUBO ubo = {0};
    ubo = gf3d_particle_get_uniform_buffer(&particle);
    gf3d_pipeline_queue_render(
        gf3d_particle_manager.pipe,
        gf3d_particle_manager.buffer,
        1,//its a single quad
        VK_NULL_HANDLE,
        &ubo,
        NULL);
}


void gf3d_particle_trail_draw(Color color, float size, Edge3D tail)
{
    
}

/*eol@eof*/
