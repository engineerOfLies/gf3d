#ifndef __GF3D_OBJ_LOAD_H__
#define __GF3D_OBJ_LOAD_H__

#include "gf3d_mesh.h"

typedef struct
{
    Face *faces;
    Uint32 face_count;
    Vertex *vertices;
    Uint32 vertex_count;
}ObjData;

/**
 * @brief parse an OBJ file into ObjData;
 * @param filename the name of the file to parse
 * @return NULL on error or ObjData otherwise.  Note: this must be freed with gf3d_obj_free
 */
ObjData *gf3d_obj_load_from_file(char *filename);

void gf3d_obj_free(ObjData *obj);

#endif
