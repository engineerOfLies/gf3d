#include "simple_logger.h"

#include "gfc_types.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf3d_mesh.h"
#include "gf3d_obj_load.h"

extern int __DEBUG;

typedef struct
{
    Uint32  meshCount;
    Mesh    *meshList;
    VkDevice device;
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
}MeshManager;

static MeshManager mesh_manager = {0};

//forward declarations start
void gf3d_mesh_system_close();

/**
 * @brief initialize a new mesh in meshList
 * @param mesh_manager_index the index of the location in mesh_manager meshList for the mesh to be initialized
 * @return NULL on error, or an empty mesh
 */
Mesh* gf3d_mesh_initialize(Uint32 mesh_manager_index);

void gf3d_mesh_delete(Mesh* mesh);

void gf3d_mesh_primitive_free(MeshPrimitive* prim);

int gf3d_mesh_primitive_buffer_create(MeshPrimitive* prim);

void gf3d_mesh_primitive_buffer_free(MeshPrimitive* prim);

Mesh* gf3d_mesh_get_by_filename(const char* filename);
//forward declarations end

void gf3d_mesh_system_init(Uint32 max_meshes)
{
    if (mesh_manager.meshCount != 0)
    {
        slog("Cannot init mesh system, already initialized");
        return;
    }
    if (max_meshes == 0)
    {
        slog("Cannot initialize mesh system for 0 meshes");
        return;
    }
    mesh_manager.meshList = gfc_allocate_array(sizeof(Mesh), max_meshes);
    if (!mesh_manager.meshList)
    {
        slog("cannot allocate more memory for mesh_manager meshList");
        slog_sync();
        gf3d_mesh_system_close();
        exit(-1);
        return;
    }
    mesh_manager.device = gf3d_vgraphics_get_default_logical_device();
    mesh_manager.meshCount = max_meshes;

    //we dont initialize the attributeDescriptions + bindingDescription becuase
    //model_system_init does that since its making the pipeline
    atexit(gf3d_mesh_system_close);
    if (__DEBUG)slog("mesh manager initialized");
}

void gf3d_mesh_system_close()
{
    int i;
    //go through list of meshes and delete them all
    if (mesh_manager.meshList)
    {
        for (i = 0; i < mesh_manager.meshCount; i++)
        {
            gf3d_mesh_delete(&mesh_manager.meshList[i]);
        }
        free(mesh_manager.meshList);
    }
    memset(&mesh_manager, 0, sizeof(MeshManager));
    if (__DEBUG)slog("mesh manager closed");
}

Mesh* gf3d_mesh_new()
{
    int i;
    int emptyIndex;
    int foundEmptyIndex = 0;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if (mesh_manager.meshList[i]._refCount == 0)
        {
            if (!foundEmptyIndex)
            {
                emptyIndex = i;
                foundEmptyIndex = 1;
            }
            if (strlen(mesh_manager.meshList[i].filename) == 0)
            {
                return gf3d_mesh_initialize(i);
            }

        }
    }
    if (foundEmptyIndex)
    {
        if (strlen(mesh_manager.meshList[emptyIndex].filename) > 0)
        {
            gf3d_mesh_delete(&mesh_manager.meshList[emptyIndex]);
        }
        return gf3d_mesh_initialize(emptyIndex);
    }
    else
    {
        slog("gf3d_mesh_new: no free slots for new meshes");
        return NULL;
    }
}

Mesh* gf3d_mesh_initialize(Uint32 mesh_manager_index)
{
    //boiler plate initilization goes here
    mesh_manager.meshList[mesh_manager_index].primitives = gfc_list_new();
    if (mesh_manager.meshList[mesh_manager_index].primitives == NULL)
    {
        slog("cannot allocate more memory for a new mesh");
        return NULL;
    }
    mesh_manager.meshList[mesh_manager_index]._refCount = 1;
    return &mesh_manager.meshList[mesh_manager_index];
}

void gf3d_mesh_free(Mesh* mesh)
{
    if (!mesh)
    {
        return;
    }
    mesh->_refCount--;
    if (mesh->_refCount > 0)
    {
        return;
    }
    else 
    {
        gf3d_mesh_delete(mesh);
    }
}

void gf3d_mesh_delete(Mesh* mesh)
{
    int i, c;
    MeshPrimitive* prim;
    if (!mesh)
    {
        return;
    }
    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_nth(mesh->primitives, i);
        if (!prim) continue;
        gf3d_mesh_primitive_free(prim);
    }
    gfc_list_delete(mesh->primitives);
    memset(mesh, 0, sizeof(Mesh));
}

MeshPrimitive* gf3d_mesh_primitive_new()
{
    MeshPrimitive* prim;
    prim = gfc_allocate_array(sizeof(MeshPrimitive), 1);
    //defaults here
    if (!prim)
    {
        slog("cannot allocate more memory for new mesh primitive");
        return NULL;
    }
    return prim;
}

void gf3d_mesh_primitive_free(MeshPrimitive* prim)
{
    if (!prim)return;
    gf3d_mesh_primitive_buffer_free(prim);
    if (prim->objData)
    {
        gf3d_obj_free(prim->objData);
    }
    free(prim);
}

int gf3d_mesh_primitive_buffer_create(MeshPrimitive* prim)
{
    void* data = NULL;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    if ((!prim) || (!prim->objData))
    {
        slog("cannot create primitive buffers for a non-existent mesh primitive");
        return 0;
    }

    //face buffers
    bufferSize = sizeof(Face) * prim->objData->face_count;
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData->outFace, (size_t)bufferSize);
    vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer, &prim->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);

    vkDestroyBuffer(mesh_manager.device, stagingBuffer, NULL);
    vkFreeMemory(mesh_manager.device, stagingBufferMemory, NULL);

    prim->faceCount = prim->objData->face_count;

    //vertex buffers
    bufferSize = sizeof(Vertex) * prim->objData->face_vert_count;
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData->faceVertices, (size_t)bufferSize);
    vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->vertexBuffer, &prim->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

    vkDestroyBuffer(mesh_manager.device, stagingBuffer, NULL);
    vkFreeMemory(mesh_manager.device, stagingBufferMemory, NULL);

    prim->vertexCount = prim->objData->face_vert_count;

    return 1;
}

void gf3d_mesh_primitive_buffer_free(MeshPrimitive* prim)
{
    if (!prim)
    {
        return;
    }

    //free face buffers
    if (prim->faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->faceBuffer, NULL);
        prim->faceBuffer = VK_NULL_HANDLE;
    }
    if (prim->faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device, prim->faceBufferMemory, NULL);
        prim->faceBufferMemory = VK_NULL_HANDLE;
    }

    //free vertex buffers
    if (prim->vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->vertexBuffer, NULL);
        prim->vertexBuffer = VK_NULL_HANDLE;
    }
    if (prim->vertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device, prim->vertexBufferMemory, NULL);
        prim->vertexBufferMemory = VK_NULL_HANDLE;
    }
}

Mesh* gf3d_mesh_load_obj(const char* filename)
{
    //int now, later;
    Mesh* mesh;
    MeshPrimitive* prim;

    if (!filename)
    {
        return NULL;
    }
    mesh = gf3d_mesh_get_by_filename(filename);
    if (mesh)
    {
        mesh->_refCount++;
        return mesh;
    }
    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        slog("failed to allocate a new mesh for %s ", filename);
        return NULL;
    }
    prim = gf3d_mesh_primitive_new();
    if (!prim)
    {
        slog("failed to get new primitive for mesh for %s ", filename);
        gf3d_mesh_delete(mesh);
        return NULL;
    }
    //now = SDL_GetTicks();
    prim->objData = gf3d_obj_load_from_file(filename);
    //later = SDL_GetTicks();
    //slog("time: %d", later - now);
    //slog_sync();
    if (!prim->objData)
    {
        slog("failed to parse file %s for obj data", filename);
        gf3d_mesh_delete(mesh);
        gf3d_mesh_primitive_free(prim);
        return NULL;
    }
    if (!gf3d_mesh_primitive_buffer_create(prim))
    {
        slog("failed to build prim memory buffer for prim for filename %s", filename);
        gf3d_mesh_delete(mesh);
        gf3d_mesh_primitive_free(prim);
        return NULL;
    }
    gfc_list_append(mesh->primitives, prim);
    //if you want a mesh to have multiple primitives, do that here
    gfc_line_cpy(mesh->filename, filename);
    gfc_box_cpy(mesh->bounds, prim->objData->bounds);
    return mesh;
}

void gf3d_mesh_queue_render(Mesh* mesh, Pipeline* pipe, void* uboData, Texture* texture)
{
    int i, c;
    MeshPrimitive* prim;
    if (!mesh)
    {
        slog("cannot render a mesh that doesn't exist");
        return;
    }
    if (!pipe)
    {
        slog("cannot render a mesh with a pipe doesn't exist");
        return;
    }
    if (!uboData)
    {
        slog("cannot render a mesh with uboData doesn't exist");
        return;
    }
    if (!texture)
    {
        slog("cannot render a mesh with a texture doesn't exist");
        return;
    }
    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_nth(mesh->primitives, i);
        if (!prim)
        {
            continue;
        }

        gf3d_pipeline_queue_render(
            pipe,
            prim->vertexBuffer,
            prim->vertexCount,
            prim->faceBuffer,
            uboData,
            texture);
    }
}

VkVertexInputAttributeDescription* gf3d_mesh_get_attribute_descriptions(Uint32* count)
{

    mesh_manager.attributeDescriptions[0].binding = 0;
    mesh_manager.attributeDescriptions[0].location = 0;
    mesh_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    mesh_manager.attributeDescriptions[1].binding = 0;
    mesh_manager.attributeDescriptions[1].location = 1;
    mesh_manager.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[1].offset = offsetof(Vertex, normal);

    mesh_manager.attributeDescriptions[2].binding = 0;
    mesh_manager.attributeDescriptions[2].location = 2;
    mesh_manager.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    mesh_manager.attributeDescriptions[2].offset = offsetof(Vertex, texel);

    if (count)
    {
        *count = MESH_ATTRIBUTE_COUNT;
    }
    return mesh_manager.attributeDescriptions;
}

VkVertexInputBindingDescription* gf3d_mesh_get_bind_description()
{
    mesh_manager.bindingDescription.binding = 0;
    mesh_manager.bindingDescription.stride = sizeof(Vertex);
    mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return &mesh_manager.bindingDescription;
}

Mesh* gf3d_mesh_get_by_filename(const char* filename)
{
    int i;
    if (!filename)
    {
        return NULL;
    }
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if (mesh_manager.meshList[i]._refCount == 0 || strlen(mesh_manager.meshList[i].filename) == 0)
        {
            continue;
        }
        if (gfc_strlcmp(mesh_manager.meshList[i].filename, filename) == 0)
        {
            return &mesh_manager.meshList[i];
        }
    }
    return NULL;
}
