#ifndef __GF3D_OBJ_LOAD_H__
#define __GF3D_OBJ_LOAD_H__

#include "gf3d_mesh.h"

struct ObjData_S
{    
    GFC_Vector3D *vertices;
    Uint32 vertex_count;
    GFC_Vector3D *normals;
    Uint32 normal_count;
    GFC_Vector2D *texels;
    Uint32 texel_count;
    GFC_Vector4UI8 *boneIndices;
    Uint32 bone_count;
    GFC_Vector4D *boneWeights;
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
    GFC_Box     bounds;
};

/**
 * @brief allocate a blank obj
 * @return NULL on memory error or a zero initialized obj
 */
ObjData *gf3d_obj_new();

/**
 * @brief parse an OBJ file into ObjData;
 * @param filename the name of the file to parse
 * @return NULL on error or ObjData otherwise.  Note: this must be freed with gf3d_obj_free
 */
ObjData *gf3d_obj_load_from_file(const char *filename);

/**
 * @brief a copy constructor, duplicate the ObjData of in
 * @param in the ObjData to copy
 * @return NULL on error or a duplicate copy of in
 */
ObjData *gf3d_obj_duplicate(ObjData *in);

/**
 * @brief update the vertex positions of all faceVerticies of an obj
 * @param obj the obj to modify
 * @param offset how much to move the faceVertices
 * @param rotation apply this rotation to the vertices and normals
 */
void gf3d_obj_move(ObjData *obj,GFC_Vector3D offset,GFC_Vector3D rotation);

/**
 * @brief re-organize the vertices into faceVertices for use with the rendering pipeline
 * @param obj the object to reorg
 */
void gf3d_obj_load_reorg(ObjData *obj);

/**
 * @brief merge two obj's into a new one.
 * @param ObjA the first obj to merge
 * @param offsetA an offset to apply to all the vertices in the first Obj
 * @param ObjB the second obj to merge
 * @param offsetB an offset to apply to all the vertices in the second Obj
 * @param rotation apply this rotation to the vertices and normals
 * @return NULL if the input objs are missing or have not been re-organized yet, a new third OBJ containing all the geometry of the first two.
 * @note this does NOT free the input OBJs.  
 * @note gf3d_obj_load_reorg must have been called on both prior to calling this one
 */
ObjData *gf3d_obj_merge(ObjData *ObjA,GFC_Vector3D offsetA,ObjData *ObjB,GFC_Vector3D offsetB,GFC_Vector3D rotation);

/**
 * @brief free obj data
 * @param obj the data to free
 */
void gf3d_obj_free(ObjData *obj);

/**
 * @brief perform a collision test between the edge and an obj.  Searches through each triangle for collision, returning the FIRST triangle with collision (so convex hulls might return the wrong side)
 * @param obj the object to test
 * @param offset if the model has moved, rotated, scaled etc.  this is applied before the test 
 * @param e the edge to test with
 * @param contact [optional output] provides the point of impact
 */
int gf3d_obj_edge_test(ObjData *obj,GFC_Matrix4 offset, GFC_Edge3D e,GFC_Vector3D *contact);


#endif
