#include <stdio.h>
#include "simple_logger.h"

#include "gf3d_obj_load.h"

void gf3d_obj_get_counts_from_file(ObjData *obj, FILE* file);
void gf3d_obj_load_get_data_from_file(ObjData *obj, FILE* file);

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
    
    if (obj->faceVertices != NULL)
    {
        free(obj->faceVertices);
    }
    
    if (obj->outFace != NULL)
    {
        free(obj->outFace);
    }
    
    free(obj);
}

void gf3d_obj_load_reorg(ObjData *obj)
{
    int i,f;
    int vert = 0;
    int vertexIndex,normalIndex,texelIndex;
    
    if (!obj)return;
    
    obj->face_vert_count = obj->face_count*3;
    obj->faceVertices = (Vertex *)gfc_allocate_array(sizeof(Vertex),obj->face_vert_count);
    obj->outFace = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    
    for (i = 0; i < obj->face_count;i++)
    {
        for (f = 0; f < 3;f++,vert++)
        {
            vertexIndex = obj->faceVerts[i].verts[f];
            normalIndex = obj->faceNormals[i].verts[f];
            texelIndex = obj->faceTexels[i].verts[f];
            
            vector3d_copy(obj->faceVertices[vert].vertex,obj->vertices[vertexIndex]);
            vector3d_copy(obj->faceVertices[vert].normal,obj->normals[normalIndex]);
            vector2d_copy(obj->faceVertices[vert].texel,obj->texels[texelIndex]);
            
            obj->outFace[i].verts[f] = vert;
        }
    }
}

ObjData *gf3d_obj_load_from_file(char *filename)
{
    FILE *file;
    ObjData *obj;
    file = fopen(filename,"r");
    if (!file)
    {
        slog("failed to open obj file %s",filename);
        return NULL;
    }
    obj = (ObjData*)gfc_allocate_array(sizeof(ObjData),1);
    if (!obj)return NULL;
    
    gf3d_obj_get_counts_from_file(obj, file);
    
    obj->vertices = (Vector3D *)gfc_allocate_array(sizeof(Vector3D),obj->vertex_count);
    obj->normals = (Vector3D *)gfc_allocate_array(sizeof(Vector3D),obj->normal_count);
    obj->texels = (Vector2D *)gfc_allocate_array(sizeof(Vector2D),obj->texel_count);
    
    obj->faceVerts = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    obj->faceNormals = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    obj->faceTexels = (Face *)gfc_allocate_array(sizeof(Face),obj->face_count);
    
    gf3d_obj_load_get_data_from_file(obj, file);
    fclose(file);
    gf3d_obj_load_reorg(obj);
    return obj;
}

void gf3d_obj_get_counts_from_file(ObjData *obj, FILE* file)
{
  char buf[256];
  int  numvertices = 0;
  int  numtexels = 0;
  int  numnormals = 0;
  int  numfaces = 0;

  if ((file == NULL) ||
     (obj == NULL))
  {
    return;
  }
  while(fscanf(file, "%s", buf) != EOF)
  {
    switch(buf[0])
    {
      case 'v':
        switch(buf[1])
        {
          case '\0':
            fgets(buf, sizeof(buf), file);
            numvertices++;
            break;
          case 'n':
            fgets(buf, sizeof(buf), file);
            numnormals++;
            break;
          case 't':
            fgets(buf, sizeof(buf), file);
            numtexels++;
            break;
          default:
            break;
        }
        break;
      case 'f':
        fgets(buf, sizeof(buf), file);
        numfaces++;
        break;
      default:
        fgets(buf, sizeof(buf), file);
        break;
    }
  }
  obj->vertex_count  = numvertices;
  obj->texel_count  = numtexels;
  obj->normal_count  = numnormals;
  obj->face_count = numfaces;
}

void gf3d_obj_load_get_data_from_file(ObjData *obj, FILE* file)
{
  int  numvertices = 0;
  int  numnormals = 0;
  int  numtexcoords = 0;
  int  numfaces = 0;
  char buf[128];
  float x,y,z;
  int f[3][3];

  if (file == NULL)
    return;
  
  rewind(file);
  while(fscanf(file, "%s", buf) != EOF)
  {
    switch(buf[0])
    {
      case 'v':
        switch(buf[1])
        {
          case '\0':
            fscanf(
                file,
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
            fscanf(
                file,
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
            fscanf(
                file,
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
        fscanf(
            file,
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
        fgets(buf, sizeof(buf), file);
        break;
    }
  }
}

/*eol@eof*/
