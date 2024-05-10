#include <stdio.h>

#include "simple_logger.h"

#include "gfc_pak.h"

#include "gf3d_obj_load.h"

void gf3d_obj_get_counts_from_file(ObjData *obj, const char *mem,size_t fileSize);
void gf3d_obj_load_get_data_from_file(ObjData *obj, const char *mem,size_t fileSize);

void gf3d_obj_free(ObjData *obj)
{
    if (!obj)return;
    
    if (obj->vertices != NULL)
    {
        free(obj->vertices);
    }
    if (obj->normals != NULL)
    {
        free(obj->normals);
    }
    if (obj->texels != NULL)
    {
        free(obj->texels);
    }
    if (obj->boneIndices!= NULL)
    {
        free(obj->boneIndices);
    }
    if (obj->boneWeights != NULL)
    {
        free(obj->boneWeights);
    }
    
    if (obj->faceVerts != NULL)
    {
        free(obj->faceVerts);
    }
    if (obj->faceNormals != NULL)
    {
        free(obj->faceNormals);
    }
    if (obj->faceTexels != NULL)
    {
        free(obj->faceTexels);
    }
    if (obj->faceBones != NULL)
    {
        free(obj->faceBones);
    }
    
    if (obj->faceWeights != NULL)
    {
        free(obj->faceWeights);
    }
    
    if (obj->outFace != NULL)
    {
        free(obj->outFace);
    }
    
    free(obj);
}

//while normal obj files don't support bones, the obj structure is used as a staging area for gltf loading.

void gf3d_obj_load_reorg(ObjData *obj)
{
    int i,f;
    int vert = 0;
    int vertexIndex,normalIndex,texelIndex,boneIndex,weightIndex;
    
    if (!obj)return;
    
    obj->face_vert_count = obj->face_count*3;
    obj->faceVertices = (Vertex *)gfc_allocate_array(sizeof(Vertex),obj->face_vert_count);
    obj->outFace = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    
    for (i = 0; i < obj->face_count;i++)
    {
        for (f = 0; f < 3;f++,vert++)
        {
            vertexIndex = obj->faceVerts[i].verts[f];
            gfc_vector3d_copy(obj->faceVertices[vert].vertex,obj->vertices[vertexIndex]);

            if (obj->faceNormals)
            {
                normalIndex = obj->faceNormals[i].verts[f];
                gfc_vector3d_copy(obj->faceVertices[vert].normal,obj->normals[normalIndex]);
            }
            if (obj->faceTexels)
            {
                texelIndex = obj->faceTexels[i].verts[f];
                gfc_vector2d_copy(obj->faceVertices[vert].texel,obj->texels[texelIndex]);
            }
            if (obj->faceBones)
            {
                boneIndex = obj->faceBones[i].verts[f];
                gfc_vector4d_copy(obj->faceVertices[vert].bones,obj->boneIndices[boneIndex]);
            }
            if (obj->faceWeights)
            {
                weightIndex = obj->faceWeights[i].verts[f];
                gfc_vector4d_copy(obj->faceVertices[vert].weights,obj->boneWeights[weightIndex]);
            }
            
            obj->outFace[i].verts[f] = vert;
        }
    }
}

void gf3d_obj_get_bounds(ObjData *obj)
{
    int i;
    if (!obj)return;
    for (i = 0; i < obj->vertex_count; i++)
    {
        if (obj->vertices[i].x < obj->bounds.x)obj->bounds.x = obj->vertices[i].x;
        if (obj->vertices[i].y < obj->bounds.y)obj->bounds.y = obj->vertices[i].y;
        if (obj->vertices[i].z < obj->bounds.z)obj->bounds.z = obj->vertices[i].z;
        if (obj->vertices[i].x > obj->bounds.w)obj->bounds.w = obj->vertices[i].x;
        if (obj->vertices[i].y > obj->bounds.h)obj->bounds.h = obj->vertices[i].y;
        if (obj->vertices[i].z > obj->bounds.d)obj->bounds.d = obj->vertices[i].z;
    }
    obj->bounds.w -= obj->bounds.x;
    obj->bounds.h -= obj->bounds.y;
    obj->bounds.d -= obj->bounds.z;
}

ObjData *gf3d_obj_load_from_file(const char *filename)
{
    ObjData *obj;
    void *mem = NULL;
    size_t fileSize;
    
    mem = gfc_pak_file_extract(filename,&fileSize);
    
    if (!mem)return NULL;
        
    obj = (ObjData*)gfc_allocate_array(sizeof(ObjData),1);
    if (!obj)return NULL;
    
    gf3d_obj_get_counts_from_file(obj, mem,fileSize);
    
    obj->vertices = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),obj->vertex_count);
    obj->normals = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),obj->normal_count);
    obj->texels = (GFC_Vector2D *)gfc_allocate_array(sizeof(GFC_Vector2D),obj->texel_count);
    
    obj->faceVerts = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    obj->faceNormals = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    obj->faceTexels = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    
    gf3d_obj_load_get_data_from_file(obj, mem,fileSize);
    
    
    gf3d_obj_get_bounds(obj);
    gf3d_obj_load_reorg(obj);
    return obj;
}

void gf3d_obj_get_counts_from_file(ObjData *obj, const char *mem,size_t fileSize)
{
    char buf[256];
    const char *p;
    const char *c;
    int  numvertices = 0;
    int  numtexels = 0;
    int  numnormals = 0;
    int  numfaces = 0;

    if ((!obj)||(!mem))
    {
        return;
    }
    p = mem;
    while(sscanf(p, "%s", buf) != EOF)
    {
        c = strchr(p,'\n');//go to the end of line
        p = c + 1;//and then the next line
        switch(buf[0])
        {
            case 'v':
              switch(buf[1])
              {
                case '\0':
                  numvertices++;
                  break;
                case 'n':
                  numnormals++;
                  break;
                case 't':
                  numtexels++;
                  break;
                default:
                  break;
              }
              break;
            case 'f':
              numfaces++;
              break;
            default:
              break;
        }
    }
    obj->vertex_count  = numvertices;
    obj->texel_count  = numtexels;
    obj->normal_count  = numnormals;
    obj->face_count = numfaces;
}

void gf3d_obj_load_get_data_from_file(ObjData *obj, const char *mem,size_t fileSize)
{
    const char *p;
    const char *c;
    int  numvertices = 0;
    int  numnormals = 0;
    int  numtexcoords = 0;
    int  numfaces = 0;
    char buf[128];
    float x,y,z;
    int f[3][3];

    if ((!obj)||(!mem))return;

    p = mem;
    while(sscanf(p, "%s", buf) != EOF)
    {
        p += strlen(buf);//skip what was checked so far
        switch(buf[0])
        {
            case 'v':
              switch(buf[1])
              {
                case '\0':
                  sscanf(
                      p,
                      "%f %f %f",
                      &x,
                      &y,
                      &z
                    );
                  obj->vertices[numvertices].x = x;
                  obj->vertices[numvertices].y = y;
                  obj->vertices[numvertices].z = z;
                  numvertices++;
                  break;
                case 'n':
                  sscanf(
                      p,
                      "%f %f %f",
                      &x,
                      &y,
                      &z
                  );
                  obj->normals[numnormals].x = x;
                  obj->normals[numnormals].y = y;
                  obj->normals[numnormals].z = z;
                  numnormals++;
                  break;
                case 't':
                  sscanf(
                      p,
                      "%f %f",
                      &x,
                      &y
                    );
                  obj->texels[numtexcoords].x = x;
                  obj->texels[numtexcoords].y = 1 - y;
                  numtexcoords++;
                  break;
                default:
                  break;
              }
              break;
            case 'f':
              sscanf(
                  p,
                  "%d/%d/%d %d/%d/%d %d/%d/%d",
                  &f[0][0],
                  &f[0][1],
                  &f[0][2],
                  
                  &f[1][0],
                  &f[1][1],
                  &f[1][2],
                  
                  &f[2][0],
                  &f[2][1],
                  &f[2][2]);
              
              obj->faceVerts[numfaces].verts[0]   = f[0][0] - 1;
              obj->faceTexels[numfaces].verts[0]  = f[0][1] - 1;
              obj->faceNormals[numfaces].verts[0] = f[0][2] - 1;
              
              obj->faceVerts[numfaces].verts[1]   = f[1][0] - 1;
              obj->faceTexels[numfaces].verts[1]  = f[1][1] - 1;
              obj->faceNormals[numfaces].verts[1] = f[1][2] - 1;
              
              obj->faceVerts[numfaces].verts[2]   = f[2][0] - 1;
              obj->faceTexels[numfaces].verts[2]  = f[2][1] - 1;
              obj->faceNormals[numfaces].verts[2] = f[2][2] - 1;
              numfaces++;
              break;
            default:
              break;
        }
        c = strchr(p,'\n');//go to the end of line
        p = c + 1;//and then the next line
    }
}

/*eol@eof*/
