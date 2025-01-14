#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_decode.h"
#include "gfc_list.h"
#include "gfc_config.h"
#include "gfc_pak.h"

#include "gf3d_mesh.h"
#include "gf3d_model.h"
#include "gf3d_obj_load.h"

#include "gf3d_gltf_parse.h"


void gf3d_gltf_reorg_obj(ObjData *obj);
char *gf3d_gltf_decode(SJson *gltf, Uint32 bufferIndex);


GLTF *gf3d_gltf_new()
{
    GLTF *gltf = gfc_allocate_array(sizeof(GLTF),1);
    gltf->buffers = gfc_list_new();
    return gltf;
}

void gf3d_gltf_free(GLTF *gltf)
{
    int i,c;
    char *buffer;
    if (!gltf)return;
    c = gfc_list_get_count(gltf->buffers);
    for (i = 0; i < c; i++)
    {
        buffer = gfc_list_get_nth(gltf->buffers,i);
        if (!buffer)continue;
        free(buffer);
    }
    gfc_list_delete(gltf->buffers);
    sj_free(gltf->json);
    free(gltf);
}

GLTF *gf3d_gltf_load(const char *filename)
{
    SJson *json;
    SJson *buffers;
    int i,c;
    GLTF *gltf;
    if (!filename)return NULL;
    json = gfc_pak_load_json(filename);
    if (!json)return NULL;
    gltf = gf3d_gltf_new();
    if (!gltf)
    {
        sj_free(json);
        return NULL;
    }
    gltf->json = json;
    gfc_line_cpy(gltf->filename,filename);
    buffers = sj_object_get_value(json,"buffers");
    c = sj_array_get_count(buffers);
    for (i = 0;i < c; i++)
    {
        gfc_list_append(gltf->buffers,gf3d_gltf_decode(json, i));
    }
//    slog("decoded %i buffers from %s",c,filename);
    return gltf;
}


SJson *gf3d_gltf_parse_get_accessor(GLTF *gltf,Uint32 index)
{
    SJson *accessors;
    if (!gltf)return NULL;
    
    accessors = sj_object_get_value(gltf->json,"accessors");
    if (!accessors)return NULL;
    return sj_array_get_nth(accessors,index);
}

SJson *gf3d_gltf_parse_get_buffer_view(SJson *gltf,Uint32 index)
{
    SJson *bufferViews;
    if (!gltf)return NULL;
    
    bufferViews = sj_object_get_value(gltf,"bufferViews");
    if (!bufferViews)return NULL;
    return sj_array_get_nth(bufferViews,index);
}

char *gf3d_gltf_decode(SJson *gltf, Uint32 bufferIndex)
{
    SJson *buffers,*buffer;
    const char *data;
    if (!gltf)return NULL;
    
    buffers = sj_object_get_value(gltf,"buffers");
    if (!buffers)return NULL;

    buffer = sj_array_get_nth(buffers,bufferIndex);
    if (!buffer)return NULL;
    
    data = sj_object_get_value_as_string(buffer,"uri");
    if (!data)return NULL;
        
    data = strchr(data, ',');
    data++;// move past the header
    return gfc_base64_decode (data, strlen(data), NULL);
}

const char *gf3d_gltf_get_buffer(GLTF *gltf,Uint32 index)
{
    if (!gltf)return NULL;
    return gfc_list_get_nth(gltf->buffers,index);
}

const char *gf3d_gltf_get_buffer_data(GLTF *gltf,Uint32 index,size_t offset)
{
    const char *data;
    
    if (!gltf)return NULL;
    data = gf3d_gltf_get_buffer(gltf,index);    
    if (!data)
    {
        slog("failed to get buffer %i from file %s",index,gltf->filename);
        return NULL;
    }
    
    return &data[offset];
}

Uint8 gf3d_gltf_parse_copy_buffer_data(SJson *gltf,Uint32 index,size_t offset,size_t length, char *output)
{
    char *data;
    
    if (!output)
    {
        slog("no output parameter provided");
        return 0;
    }
    data = gf3d_gltf_decode(gltf, index);    
    if (!data)return 0;    
    memcpy(output,&data[offset],length);
    free(data);
    return 1;
}

Uint8 gf3d_gltf_get_data_from_buffer(GLTF *gltf,Uint32 buffer,size_t offset,size_t length, char *output)
{
    const char *data;
    
    if (!output)
    {
        slog("no output parameter provided");
        return 0;
    }
    data = gf3d_gltf_get_buffer_data(gltf,buffer,offset);
    if (!data)return 0;
    memcpy(output,data,length);
    return 1;
}


void gf3d_gltf_get_buffer_view_data(GLTF *gltf,Uint32 viewIndex,char *buffer)
{
    SJson *bufferView;
    int index,byteLength,byteOffset;
    if ((!gltf)||(!buffer))return;
    bufferView = gf3d_gltf_parse_get_buffer_view(gltf->json,viewIndex);
    if (!bufferView)
    {
        slog("failed to find buffer view %i in %s",viewIndex,gltf->filename);
        return;
    }
    sj_object_get_value_as_int(bufferView,"buffer",&index);
    sj_object_get_value_as_int(bufferView,"byteLength",&byteLength);
    sj_object_get_value_as_int(bufferView,"byteOffset",&byteOffset);
    gf3d_gltf_get_data_from_buffer(gltf,index,byteOffset,byteLength, buffer);
}

//TODO: report on componentType so the data can be parsed correctly
const char *gf3d_gltf_accessor_get_details(GLTF* gltf,Uint32 accessorIndex, int *bufferIndex, int *count)
{
    SJson *accessor;
    if (!gltf)return NULL;
    accessor = gf3d_gltf_parse_get_accessor(gltf,accessorIndex);
    if (!accessor)return NULL;
    if (bufferIndex)
    {
        sj_object_get_value_as_int(accessor,"bufferView",bufferIndex);
    }
    if (count)
    {
        sj_object_get_value_as_int(accessor,"count",count);
    }
    return sj_object_get_value_as_string(accessor,"type");
}

ObjData *gf3d_gltf_parse_primitive(GLTF *gltf,SJson *primitive)
{
    ObjData *obj;
    GFC_Vector3D min,max;
    int index,bufferIndex;
    SJson *attributes,*accessor;

    if ((!gltf)||(!primitive))return NULL;
    obj = gf3d_obj_new();
    if (!obj)return NULL;
    
    attributes = sj_object_get_value(primitive,"attributes");
    
    if (!attributes)
    {
        free(obj);
        slog("primitive contains no attributes");
        return NULL;
    }
    
    if (sj_object_get_value_as_int(attributes,"POSITION",&index))
    {
        if (gf3d_gltf_accessor_get_details(gltf,index, &bufferIndex, (int *)&obj->vertex_count))
        {
            obj->vertices = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),obj->vertex_count);
            
            gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)obj->vertices);
            
            accessor = gf3d_gltf_parse_get_accessor(gltf,index);
            gfc_vector3d_clear(min);
            gfc_vector3d_clear(max);
            sj_value_as_vector3d(sj_object_get_value(accessor,"min"),&min);
            sj_value_as_vector3d(sj_object_get_value(accessor,"max"),&max);
            obj->bounds = gfc_box(min.x, min.y, min.z, max.x, max.y, max.z);
        }
        else slog("failed to get accessor detials");        
    }
    if (sj_object_get_value_as_int(attributes,"NORMAL",&index))
    {
        if (gf3d_gltf_accessor_get_details(gltf,index, &bufferIndex, (int *)&obj->normal_count))
        {
            obj->normals = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),obj->normal_count);
            
            gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)obj->normals);            
        }
        else slog("failed to get accessor detials");
    }
    
    if (sj_object_get_value_as_int(attributes,"TEXCOORD_0",&index))
    {
        if (gf3d_gltf_accessor_get_details(gltf,index, &bufferIndex, (int *)&obj->texel_count))
        {
            obj->texels = (GFC_Vector2D *)gfc_allocate_array(sizeof(GFC_Vector2D),obj->texel_count);
            
            gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)obj->texels);            
        }
        else slog("failed to get accessor detials");
    }
    
    //bone indices
    if (sj_object_get_value_as_int(attributes,"JOINTS_0",&index))
    {
        if (gf3d_gltf_accessor_get_details(gltf,index, &bufferIndex, (int *)&obj->bone_count))
        {
            obj->boneIndices = (GFC_Vector4UI8 *)gfc_allocate_array(sizeof(GFC_Vector4UI8),obj->bone_count);
            
            gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)obj->boneIndices);
            
        }
        else slog("failed to get accessor detials");
    }
    //bone weights
    if (sj_object_get_value_as_int(attributes,"WEIGHTS_0",&index))
    {
        if (gf3d_gltf_accessor_get_details(gltf,index, &bufferIndex, (int *)&obj->weight_count))
        {
            obj->boneWeights = (GFC_Vector4D *)gfc_allocate_array(sizeof(GFC_Vector4D),obj->weight_count);
            
            gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)obj->boneWeights);
        }
        else slog("failed to get accessor detials");
    }

    if (sj_object_get_value_as_int(primitive,"indices",&index))
    {
        if (gf3d_gltf_accessor_get_details(gltf,index, &bufferIndex, (int *)&obj->face_count))
        {
            obj->face_count /= 3;
            obj->outFace = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);

            gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)obj->outFace);            
        }
        else slog("failed to get accessor detials");
    }
    gf3d_gltf_reorg_obj(obj);
    return obj;
}

void gf3d_gltf_reorg_obj(ObjData *obj)
{
    int i;
    
    if (!obj)return;
    
    obj->face_vert_count = obj->vertex_count;
    obj->faceVertices = (Vertex *)gfc_allocate_array(sizeof(Vertex),obj->face_vert_count);

    for (i = 0; i< obj->vertex_count;i++)
    {
        if (obj->vertices)gfc_vector3d_copy(obj->faceVertices[i].vertex,obj->vertices[i]);
        if (obj->normals)gfc_vector3d_copy(obj->faceVertices[i].normal,obj->normals[i]);
        if (obj->texels)gfc_vector2d_copy(obj->faceVertices[i].texel,obj->texels[i]);
        if (obj->boneIndices)gfc_vector4d_copy(obj->faceVertices[i].bones,(float)obj->boneIndices[i]);
        if (obj->boneWeights)gfc_vector4d_copy(obj->faceVertices[i].weights,obj->boneWeights[i]);
    }
}


Mesh *gf3d_gltf_parse_mesh(SJson *meshData,GLTF *gltf)
{
    int i,c;
    Mesh *mesh;
    ObjData *obj;
    MeshPrimitive *primitive;
    SJson *primitives,*primitiveData;
    
    if ((!meshData)||(!gltf))return NULL;
    
    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        return NULL;
    }
    primitives = sj_object_get_value(meshData,"primitives");
    c = sj_array_get_count(primitives);
    for (i = 0; i < c; i++)
    {
        primitiveData = sj_array_get_nth(primitives,i);
        if (!primitiveData)continue;
        obj = gf3d_gltf_parse_primitive(gltf,primitiveData);
        if (!obj)
        {
            continue;
        }
        primitive = gf3d_mesh_primitive_new();
        if (!primitive)
        {
            gf3d_obj_free(obj);
            continue;
        }
        
        primitive->objData = obj;
        gf3d_mesh_create_vertex_buffer_from_vertices(primitive);
        
        gfc_list_append(mesh->primitives,primitive);
        mesh->bounds.x = MIN(mesh->bounds.x,obj->bounds.x);
        mesh->bounds.y = MIN(mesh->bounds.y,obj->bounds.y);
        mesh->bounds.z = MIN(mesh->bounds.z,obj->bounds.z);

        mesh->bounds.w = MAX(mesh->bounds.w,obj->bounds.w);
        mesh->bounds.h = MAX(mesh->bounds.h,obj->bounds.h);
        mesh->bounds.d = MAX(mesh->bounds.d,obj->bounds.d);
    }
    return mesh;
}


Model *gf3d_gltf_parse_model(const char *filename)
{
    int i,c;
    GLTF *gltf;
    SJson *meshes,*meshData;
    Mesh *mesh;
    Model *model;
    if (!filename)return NULL;
    
    //from here start rewriting everything around using the GLTF wrapper
    gltf = gf3d_gltf_load(filename);
    if (!gltf)
    {
        slog("GLTF file '%s' Failed to load",filename);
        return NULL;
    }
    model = gf3d_model_new();
    if (!model)
    {
        gf3d_gltf_free(gltf);
        return NULL;
    }
    
    meshes = sj_object_get_value(gltf->json,"meshes");
    c = sj_array_get_count(meshes);
    for (i = 0; i< c; i++)
    {
        meshData = sj_array_get_nth(meshes,i);
        if (!meshData)continue;
        mesh = gf3d_gltf_parse_mesh(meshData,gltf);
        if (mesh)
        {
            gfc_line_cpy(mesh->filename,filename);
            gfc_box_cpy(model->bounds,mesh->bounds);
        }
        gfc_list_append(model->mesh_list,mesh);
    }
    gfc_line_cpy(model->filename,filename);
    gf3d_gltf_free(gltf);
    return model;
}

/*EOL@EOF*/
