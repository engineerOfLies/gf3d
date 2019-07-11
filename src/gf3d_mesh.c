#include <stddef.h>

#include "simple_logger.h"

#include "gf3d_vgraphics.h"
#include "gf3d_mesh.h"
#include "gf3d_obj_load.h"
#include "gf3d_swapchain.h"
#include "gf3d_commands.h"


#define ATTRIBUTE_COUNT 3


typedef struct
{
    Mesh *mesh_list;
    Uint32 mesh_max;
    VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription bindingDescription;
    Command *stagingCommandBuffer;
}MeshSystem;

static MeshSystem gf3d_mesh = {0};

void gf3d_mesh_close();
void gf3d_mesh_delete(Mesh *mesh);
Mesh *gf3d_mesh_get_by_filename(char *filename);

void gf3d_mesh_init(Uint32 mesh_max)
{
    if (!mesh_max)
    {
        slog("failed to initialize mesh system: cannot allocate 0 mesh_max");
        return;
    }
    atexit(gf3d_mesh_close);
    gf3d_mesh.mesh_max = mesh_max;
    
    gf3d_mesh.bindingDescription.binding = 0;
    gf3d_mesh.bindingDescription.stride = sizeof(Vertex);
    gf3d_mesh.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    gf3d_mesh.attributeDescriptions[0].binding = 0;
    gf3d_mesh.attributeDescriptions[0].location = 0;
    gf3d_mesh.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_mesh.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    gf3d_mesh.attributeDescriptions[1].binding = 0;
    gf3d_mesh.attributeDescriptions[1].location = 1;
    gf3d_mesh.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_mesh.attributeDescriptions[1].offset = offsetof(Vertex, normal);
    
    gf3d_mesh.attributeDescriptions[2].binding = 0;
    gf3d_mesh.attributeDescriptions[2].location = 2;
    gf3d_mesh.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_mesh.attributeDescriptions[2].offset = offsetof(Vertex, texel);

    gf3d_mesh.mesh_list = gfc_allocate_array(sizeof(Mesh),mesh_max);
    slog("mesh system initialized");
}

VkVertexInputAttributeDescription * gf3d_mesh_get_attribute_descriptions(Uint32 *count)
{
    if (count)*count = ATTRIBUTE_COUNT;
    return gf3d_mesh.attributeDescriptions;
}

VkVertexInputBindingDescription * gf3d_mesh_get_bind_description()
{
    return &gf3d_mesh.bindingDescription;
}

Mesh *gf3d_mesh_new()
{
    int i;
    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        if (gf3d_mesh.mesh_list[i]._inuse == 0)
        {
            gf3d_mesh.mesh_list[i]._inuse = 1;
            gf3d_mesh.mesh_list[i]._refCount = 1;
            return &gf3d_mesh.mesh_list[i];
        }
    }
    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        if (gf3d_mesh.mesh_list[i]._refCount == 0)
        {
            gf3d_mesh_delete(&gf3d_mesh.mesh_list[i]);
            gf3d_mesh.mesh_list[i]._inuse = 1;
            gf3d_mesh.mesh_list[i]._refCount = 1;
            return &gf3d_mesh.mesh_list[i];
        }
    }
    return NULL;
}

Mesh *gf3d_mesh_get_by_filename(char *filename)
{
    int i;
    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        if (!gf3d_mesh.mesh_list[i]._inuse)continue;
        if (gfc_line_cmp(gf3d_mesh.mesh_list[i].filename,filename) == 0)
        {
            return &gf3d_mesh.mesh_list[i];
        }
    }
    return NULL;
}

void gf3d_mesh_free(Mesh *mesh)
{
    if (!mesh)return;
    mesh->_refCount--;
}

void gf3d_mesh_free_all()
{
    int i;
    for (i = 0; i < gf3d_mesh.mesh_max; i++)
    {
        gf3d_mesh_delete(&gf3d_mesh.mesh_list[i]);
    }
}

void gf3d_mesh_close()
{
    slog("cleaning up mesh data");
    if (gf3d_mesh.mesh_list)
    {
        gf3d_mesh_free_all();
        // TODO: iterate through mesh data and free all data
        free(gf3d_mesh.mesh_list);
        gf3d_mesh.mesh_list = NULL;
    }
    slog("mesh system closed");
}

void gf3d_mesh_delete(Mesh *mesh)
{
    if ((!mesh)||(!mesh->_inuse))return;
    if (mesh->faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_vgraphics_get_default_logical_device(), mesh->faceBuffer, NULL);
        slog("mesh %s face buffer freed",mesh->filename);
    }
    if (mesh->faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_vgraphics_get_default_logical_device(), mesh->faceBufferMemory, NULL);
        slog("mesh %s face buffer memory freed",mesh->filename);
    }
    if (mesh->buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(gf3d_vgraphics_get_default_logical_device(), mesh->buffer, NULL);
        slog("mesh %s vert buffer freed",mesh->filename);
    }
    if (mesh->bufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_vgraphics_get_default_logical_device(), mesh->bufferMemory, NULL);
        slog("mesh %s vert buffer memory freed",mesh->filename);
    }
    memset(mesh,0,sizeof(Mesh));
}

void gf3d_mesh_scene_add(Mesh *mesh)
{
    if (!mesh)return;
}

void gf3d_mesh_render(Mesh *mesh,VkCommandBuffer commandBuffer, VkDescriptorSet * descriptorSet)
{
    VkDeviceSize offsets[] = {0};
    Pipeline *pipe;
    if (!mesh)
    {
        slog("cannot render a NULL mesh");
        return;
    }
    pipe = gf3d_vgraphics_get_graphics_pipeline();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->buffer, offsets);
    
    vkCmdBindIndexBuffer(commandBuffer, mesh->faceBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe->pipelineLayout, 0, 1, descriptorSet, 0, NULL);
    
    vkCmdDrawIndexed(commandBuffer, mesh->faceCount * 3, 1, 0, 0, 0);
}

void gf3d_mesh_setup_face_buffers(Mesh *mesh,Face *faces,Uint32 fcount)
{
    void* data;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    VkDeviceSize bufferSize = sizeof(Face) * fcount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, faces, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->faceBuffer, &mesh->faceBufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, mesh->faceBuffer, bufferSize);

    mesh->faceCount = fcount;
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
}

void gf3d_mesh_create_vertex_buffer_from_vertices(Mesh *mesh,Vertex *vertices,Uint32 vcount,Face *faces,Uint32 fcount)
{
    void *data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    size_t bufferSize;    
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    bufferSize = sizeof(Vertex) * vcount;
    
    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_vgraphics_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mesh->buffer, &mesh->bufferMemory);

    gf3d_vgraphics_copy_buffer(stagingBuffer, mesh->buffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
    
    mesh->vertexCount = vcount;
    mesh->bufferMemory = mesh->bufferMemory;
    
    gf3d_mesh_setup_face_buffers(mesh,faces,fcount);
    
    slog("created a mesh with %i vertices and %i face",vcount,fcount);
}

Mesh *gf3d_mesh_load(char *filename)
{
    Mesh *mesh;
    ObjData *obj;
    mesh = gf3d_mesh_get_by_filename(filename);
    if (mesh)return mesh;
    
    obj = gf3d_obj_load_from_file(filename);
    
    if (!obj)
    {
        return NULL;
    }
    
    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        return NULL;
    }
    gf3d_mesh_create_vertex_buffer_from_vertices(mesh,obj->faceVertices,obj->face_vert_count,obj->outFace,obj->face_count);
    gf3d_obj_free(obj);
    gfc_line_cpy(mesh->filename,filename);
    return mesh;
}

/*eol@eof*/
