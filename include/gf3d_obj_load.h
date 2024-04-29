#ifndef __GF3D_OBJ_LOAD_H__
#define __GF3D_OBJ_LOAD_H__

#include "gf3d_mesh.h"

typedef struct
{    
    Vector3D *vertices;
    Uint32 vertex_count;
    Vector3D *normals;
    Uint32 normal_count;
    Vector2D *texels;
    Uint32 texel_count;
    Vector4UI8 *boneIndices;
    Uint32 bone_count;
    Vector4D *boneWeights;
    Uint32 weight_count;
    
    Face *faceVerts;
    Face *faceNormals;
    Face *faceTexels;
    Face *faceBones;
    Face *faceWeights;
    
    Uint32 face_count;

    Face *outFace;
    
    Vertex *faceVertices;
    Uint32  face_vert_count;
    Box     bounds;
}ObjData;

/**
 * @brief parse an OBJ file into ObjData;
 * @param filename the name of the file to parse
 * @return NULL on error or ObjData otherwise.  Note: this must be freed with gf3d_obj_free
 */
ObjData *gf3d_obj_load_from_file(const char *filename);

/**
 * @brief re-organize the vertices into faceVertices for use with the rendering pipeline
 * @param obj the object to reorg
 */
void gf3d_obj_load_reorg(ObjData *obj);

void gf3d_obj_free(ObjData *obj);

#endif
