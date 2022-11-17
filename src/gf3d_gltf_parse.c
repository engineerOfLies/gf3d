#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_list.h"
#include "gfc_config.h"

#include "gf3d_mesh.h"
#include "gf3d_obj_load.h"

#include "gf3d_gltf_parse.h"


char *gfc_base64_decode (const char *in, size_t inLen, size_t *outLen);
char *gfc_base64_encode(const void* input, size_t inputLength, size_t *newSize);

void gf3d_gltf_reorg_obj(ObjData *obj);


SJson *gf3d_gltf_parse_get_accessor(SJson *gltf,Uint32 index)
{
    SJson *accessors;
    if (!gltf)return NULL;
    
    accessors = sj_object_get_value(gltf,"accessors");
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
    int byteLength;
    SJson *buffers,*buffer;
    const char *data;
    if (!gltf)return NULL;
    
    buffers = sj_object_get_value(gltf,"buffers");
    if (!buffers)return NULL;

    buffer = sj_array_get_nth(buffers,bufferIndex);
    if (!buffer)return NULL;
    
    data = sj_object_get_value_as_string(buffer,"uri");
    if (!data)return NULL;
    
    sj_get_integer_value(sj_object_get_value(buffer,"byteLength"),&byteLength);
    
    data = strchr(data, ',');
    data++;// move past the header
    return gfc_base64_decode (data, strlen(data), NULL);
}

const char *gf3d_gltf_parse_get_buffer_data(SJson *gltf,Uint32 index,size_t offset)
{
    SJson *buffers,*buffer;
    const char *data;
    
    if (!gltf)return NULL;
    
    buffers = sj_object_get_value(gltf,"buffers");
    if (!buffers)return NULL;
    buffer = sj_array_get_nth(buffers,index);
    if (!buffer)return NULL;
    data = sj_object_get_value_as_string(buffer,"decoded");
    if (!data)return NULL;
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

ObjData *gf3d_gltf_parse_primitive(SJson *gltf,SJson *primitive)
{
    ObjData *obj;
    Vector3D min,max;
    int index,byteLength,byteOffset;
    SJson *attributes,*position,*accessor,*bufferView,*normal,*texcoord,*indices;

    if ((!gltf)||(!primitive))return NULL;
    obj = (ObjData*)gfc_allocate_array(sizeof(ObjData),1);
    if (!obj)return NULL;
    
    attributes = sj_object_get_value(primitive,"attributes");
    
    if (!attributes)
    {
        free(obj);
        slog("primitive contains no attributes");
        return NULL;
    }
    
    position = sj_object_get_value(attributes,"POSITION");
    if (position)
    {
        sj_get_integer_value(position,&index);
        accessor = gf3d_gltf_parse_get_accessor(gltf,index);
        if (accessor)
        {
            sj_get_integer_value(sj_object_get_value(accessor,"count"),(int *)&obj->vertex_count);
            sj_get_integer_value(sj_object_get_value(accessor,"bufferView"),&index);
            
            bufferView = gf3d_gltf_parse_get_buffer_view(gltf,index);
            if (bufferView)
            {
                obj->vertices = (Vector3D *)gfc_allocate_array(sizeof(Vector3D),obj->vertex_count);
                sj_get_integer_value(sj_object_get_value(bufferView,"buffer"),&index);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteLength"),&byteLength);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteOffset"),&byteOffset);
                gf3d_gltf_parse_copy_buffer_data(gltf,index,byteOffset,byteLength, (char *)obj->vertices);
            }
            sj_value_as_vector3d(sj_object_get_value(accessor,"min"),&min);
            sj_value_as_vector3d(sj_object_get_value(accessor,"max"),&max);
            obj->bounds = gfc_box(min.x, min.y, min.z, max.x, max.y, max.z);
        }
    }
    normal = sj_object_get_value(attributes,"NORMAL");
    if (normal)
    {
        sj_get_integer_value(normal,&index);
        accessor = gf3d_gltf_parse_get_accessor(gltf,index);
        if (accessor)
        {
            sj_get_integer_value(sj_object_get_value(accessor,"count"),(int *)&obj->normal_count);
            sj_get_integer_value(sj_object_get_value(accessor,"bufferView"),&index);
            
            bufferView = gf3d_gltf_parse_get_buffer_view(gltf,index);
            if (bufferView)
            {
                obj->normals = (Vector3D *)gfc_allocate_array(sizeof(Vector3D),obj->normal_count);
                sj_get_integer_value(sj_object_get_value(bufferView,"buffer"),&index);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteLength"),&byteLength);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteOffset"),&byteOffset);
                gf3d_gltf_parse_copy_buffer_data(gltf,index,byteOffset,byteLength, (char *)obj->normals);
            }
        }
    }
    
    texcoord = sj_object_get_value(attributes,"TEXCOORD_0");
    if (texcoord)
    {
        sj_get_integer_value(texcoord,&index);
        accessor = gf3d_gltf_parse_get_accessor(gltf,index);
        if (accessor)
        {
            sj_get_integer_value(sj_object_get_value(accessor,"count"),(int *)&obj->texel_count);
            sj_get_integer_value(sj_object_get_value(accessor,"bufferView"),&index);
            
            bufferView = gf3d_gltf_parse_get_buffer_view(gltf,index);
            if (bufferView)
            {
                obj->texels = (Vector2D *)gfc_allocate_array(sizeof(Vector2D),obj->texel_count);
                sj_get_integer_value(sj_object_get_value(bufferView,"buffer"),&index);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteLength"),&byteLength);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteOffset"),&byteOffset);
                gf3d_gltf_parse_copy_buffer_data(gltf,index,byteOffset,byteLength, (char *)obj->texels);
            }
        }
    }
    
    indices = sj_object_get_value(primitive,"indices");
    
    if (indices)
    {
        sj_get_integer_value(indices,&index);
        accessor = gf3d_gltf_parse_get_accessor(gltf,index);
        if (accessor)
        {
            sj_get_integer_value(sj_object_get_value(accessor,"count"),(int *)&obj->face_count);
            obj->face_count /= 3;
            sj_get_integer_value(sj_object_get_value(accessor,"bufferView"),&index);
            
            bufferView = gf3d_gltf_parse_get_buffer_view(gltf,index);
            if (bufferView)
            {
                obj->outFace = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
                sj_get_integer_value(sj_object_get_value(bufferView,"buffer"),&index);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteLength"),&byteLength);
                sj_get_integer_value(sj_object_get_value(bufferView,"byteOffset"),&byteOffset);
                
                gf3d_gltf_parse_copy_buffer_data(gltf,index,byteOffset,byteLength, (char *)obj->outFace);
            }
        }
        
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
        vector3d_copy(obj->faceVertices[i].vertex,obj->vertices[i]);
        vector3d_copy(obj->faceVertices[i].normal,obj->normals[i]);
        vector2d_copy(obj->faceVertices[i].texel,obj->texels[i]);
        slog("Vertex %i : (%f,%f,%f), (%f,%f,%f),(%f,%f))",i,
             obj->faceVertices[i].vertex.x,obj->faceVertices[i].vertex.y,obj->faceVertices[i].vertex.z,
             obj->faceVertices[i].normal.x,obj->faceVertices[i].normal.y,obj->faceVertices[i].normal.z,
             obj->faceVertices[i].texel.x,obj->faceVertices[i].texel.y);
    }
}


Mesh *gf3d_gltf_parse_mesh(SJson *meshData,SJson *gltf)
{
    int i,c;
    Mesh *mesh;
    ObjData *obj;
    MeshPrimitive *primitive;
    SJson *primitives,*primitiveData;
    
    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        return NULL;
    }
    primitives = sj_object_get_value(meshData,"primitives");
    c = sj_array_get_count(primitives);
    for (i =0; i < c; i++)
    {
        primitiveData = sj_array_get_nth(primitives,i);
        if (!primitiveData)continue;
        obj = gf3d_gltf_parse_primitive(gltf,primitiveData);
        if (!obj)
        {
            continue;
        }
        primitive = gfc_allocate_array(sizeof(MeshPrimitive),1);
        if (!primitive)
        {
            gf3d_obj_free(obj);
            continue;
        }
        
        
        gf3d_mesh_create_vertex_buffer_from_vertices(primitive,obj->faceVertices,obj->face_vert_count,obj->outFace,obj->face_count);
        mesh->primitives = gfc_list_append(mesh->primitives,primitive);
        mesh->bounds.x = MIN(mesh->bounds.x,obj->bounds.x);
        mesh->bounds.y = MIN(mesh->bounds.y,obj->bounds.y);
        mesh->bounds.z = MIN(mesh->bounds.z,obj->bounds.z);

        mesh->bounds.w = MAX(mesh->bounds.w,obj->bounds.w);
        mesh->bounds.h = MAX(mesh->bounds.h,obj->bounds.h);
        mesh->bounds.d = MAX(mesh->bounds.d,obj->bounds.d);

        gf3d_obj_free(obj);
    }
    return mesh;
}


Model *gf3d_gltf_parse_model(const char *filename)
{
    int i,c;
    SJson *json,*meshes,*meshData;
    Mesh *mesh;
    Model *model;
    if (!filename)return NULL;
    json = sj_load(filename);
    if (!json)
    {
        slog("GLTF file '%s' Failed to load",filename);
        return NULL;
    }
    model = gf3d_model_new();
    if (!model)
    {
        sj_free(json);
        return NULL;
    }
    
    meshes = sj_object_get_value(json,"meshes");
    slog("loaded %i meshes.",sj_array_get_count(meshes));
    c = sj_array_get_count(meshes);
    for (i = 0; i< c; i++)
    {
        meshData = sj_array_get_nth(meshes,i);
        if (!meshData)continue;
        mesh = gf3d_gltf_parse_mesh(meshData,json);
        if (mesh)
        {
            gfc_line_cpy(mesh->filename,filename);
        }
        model->mesh_list = gfc_list_append(model->mesh_list,mesh);
    }
    gfc_line_cpy(model->filename,filename);
    sj_free(json);
    return model;
}

//TAKE FROM WIKI: https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
//ACCESSED: 15/9/2022

char *gfc_base64_encode(const void* input, size_t inputLength, size_t *newSize)
{
    const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *result;
    const uint8_t *data = (const uint8_t *)input;
    size_t resultSize = 0;
    size_t resultIndex = 0;
    size_t x;
    uint32_t n = 0;
    int padCount = inputLength % 3;
    uint8_t n0, n1, n2, n3;

    resultSize = inputLength + (inputLength/2) + 1;
    if (!input)return NULL;
    if (!resultSize) return NULL;
    result = gfc_allocate_array(sizeof(char),resultSize);
    if (!result)return NULL;
   
   /* increment over the length of the string, three characters at a time */
    for (x = 0; x < inputLength; x += 3) 
    {
        /* these three 8-bit (ASCII) characters become one 24-bit number */
        n = ((uint32_t)data[x]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
        
        if((x+1) < inputLength)
            n += ((uint32_t)data[x+1]) << 8;//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
        
        if((x+2) < inputLength)
            n += data[x+2];

        /* this 24-bit number gets separated into four 6-bit numbers */
        n0 = (uint8_t)(n >> 18) & 63;
        n1 = (uint8_t)(n >> 12) & 63;
        n2 = (uint8_t)(n >> 6) & 63;
        n3 = (uint8_t)n & 63;
            
        /*
        * if we have one byte available, then its encoding is spread
        * out over two characters
        */
        if(resultIndex >= resultSize)
        {
            free(result);
            return NULL;   /* indicate failure: buffer too small */
        }
        result[resultIndex++] = base64chars[n0];
        if(resultIndex >= resultSize)
        {
            free(result);
            return NULL;   /* indicate failure: buffer too small */
        }
        result[resultIndex++] = base64chars[n1];

        /*
        * if we have only two bytes available, then their encoding is
        * spread out over three chars
        */
        if((x+1) < inputLength)
        {
            if(resultIndex >= resultSize)
            {
                free(result);
                return NULL;   /* indicate failure: buffer too small */
            }
            result[resultIndex++] = base64chars[n2];
        }

        /*
        * if we have all three bytes available, then their encoding is spread
        * out over four characters
        */
        if((x+2) < inputLength)
        {
            if(resultIndex >= resultSize)
            {
                free(result);
                return NULL;   /* indicate failure: buffer too small */
            }
            result[resultIndex++] = base64chars[n3];
        }
    }

    /*
    * create and add padding that is required if we did not have a multiple of 3
    * number of characters available
    */
    if (padCount > 0) 
    { 
        for (; padCount < 3; padCount++) 
        { 
            if(resultIndex >= resultSize)
            {
                free(result);
                return NULL;   /* indicate failure: buffer too small */
            }
            result[resultIndex++] = '=';
        } 
    }
    if(resultIndex >= resultSize)
    {
        free(result);
        return NULL;   /* indicate failure: buffer too small */
    }
    result[resultIndex] = 0;
    
    if (newSize)*newSize = resultSize;
    return result;   /* indicate success */
}


#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

static const unsigned char d[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};

char *gfc_base64_decode (const char *in, size_t inLen, size_t *outLen)
{ 
    unsigned char *out,*outIt;
    const char *end = in + inLen;
    const char *it = in;
    size_t allocateSize;
    int iter = 0;
    uint32_t buf = 0;
    size_t len = 0;
    
    if (!in)
    {
        slog("no input data provided for decoding");
        return NULL;
    }
    if (!inLen)
    {
        slog("input length is zero, cannot decode");
        return NULL;
    }
    
    allocateSize = inLen;
    
    if (!allocateSize)
    {
        slog("output length is zero, cannot decode");
        return NULL;
    }
    out = gfc_allocate_array(sizeof(char),allocateSize);
    
    if (!out)
    {
        slog("failed to allocated output for decoding");
        return NULL;
    }
    outIt = out;
    
    while (it < end)
    {
        unsigned char c = d[(int)*it++];
        
        switch (c)
        {
            case WHITESPACE: 
                continue;   /* skip whitespace */
            case INVALID:
                slog("invalid character in base64 data, aborting");
                free(out);
                return NULL;   /* invalid input, return error */
            case EQUALS:    /* pad character, end of data */
                it = end;
                continue;
            default:
                buf = buf << 6 | c;
                iter++; // increment the number of iteration
                /* If the buffer is full, split it into bytes */
                if (iter >= 4)
                {
                    if ((len += 3) > allocateSize)
                    {
                        free(out);
                        return NULL; /* buffer overflow */
                    }
                    *(outIt++) = (buf >> 16) & 255;
                    *(outIt++) = (buf >> 8) & 255;
                    *(outIt++) = buf & 255;
                    buf = 0;
                    iter = 0;
                }   
        }
    }
   
    if (iter == 3)
    {
        if ((len += 2) > allocateSize)
        {
            slog("buffer overflow detected");
            free(out);
            return NULL; /* buffer overflow */
        }
        *(outIt++) = (buf >> 10) & 255;
        *(outIt++) = (buf >> 2) & 255;
    }
    else if (iter == 2)
    {
        if (++len > allocateSize)
        {
            free(out);
            return NULL; /* buffer overflow */
        }
        *(outIt++) = (buf >> 4) & 255;
    }
    if (outLen)*outLen = len; /* modify to reflect the actual output size */
    return (char *)out;
}


/*EOL@EOF*/
