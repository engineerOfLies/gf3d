#ifndef __GF3D_ARMATURE_H__
#define __GF3D_ARMATURE_H__

#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_matrix.h"
#include "gfc_actions.h"

#include "gf3d_gltf_parse.h"

//this must be kept in sync with the shader
#define MAX_SHADER_BONES 100


/**
 * @brief the armature information sent to the gpu.
 * Binding point 2, sent to vertex shader
 */
typedef struct
{
    GFC_Matrix4 bones[MAX_SHADER_BONES];
}ArmatureUBO;

typedef struct Bone3D_S
{
    GFC_TextLine        name;           /**<name of bone*/
    Uint32              index;          /**<place in the list*/
    Uint32              nodeId;         /**<for parsing*/
    struct Bone3D_S    *parent;         /**<pointer to the parent of the bone*/
    GFC_List           *children;       /**<list of indicies to any children, no data is allocated for this*/
    GFC_Matrix4         mat;            /**<matrix describing the bone orientation*/
    GFC_Matrix4         localMat;       /**<local bone orientation*/
    GFC_List           *poses;          /**<list of bonePose3Ds,when parsed, they are initially added here,*/
    //staging area for extraction
    Uint32              translationCount;
    float              *translationTimestamps;
    GFC_Vector3D       *translations;
    
    Uint32              rotationCount;
    float              *rotationTimestamps;
    GFC_Vector4D       *rotations;

    Uint32              scaleCount;
    float              *scaleTimestamps;
    GFC_Vector3D       *scales;
}Bone3D;

typedef struct
{
    Uint8       set;                /**<true if set already*/
    float       timestamp;          /**<for tweening*/
    GFC_Matrix4 localMat;           /**<local bone space*/
    GFC_Matrix4 globalMat;          /**<armature space*/
    GFC_Matrix4 jointMat;           /**<what gets sent to the gpu for mesh transforming*/
    Bone3D     *bone;               /**<link back to the bone*/
}BonePose3D;

typedef struct
{
    Uint32 boneCount;
    GFC_Matrix4 *bones;     //list of bone poses in index order
}Pose3D;

typedef struct
{
    GFC_TextLine    filepath;       /**<the file that this has been loaded from / to*/
    GFC_TextLine    name;           /**<printing name*/
    Uint32          refCount;       /**<resurce management*/
    Uint32          bindCount;      /**<how many bones in the inverseBindGFC_Matrix*/
    GFC_Matrix4    *inverseBindMatrices;    /**<to make the math math*/
    GFC_List       *bones;          /**<list of Bones in the base armature*/
    GFC_ActionList *actions;        /**<action list for managing animation of poses*/
    GFC_List       *poses;          /**<list of Pose3Ds.*/
    Uint32          maxFrames;      /**<how many poses we store*/
    float           maxTimestamp;   /**<timestamp of the last from (starting from 0.0)*/
}Armature3D;

/**
 * @brief initialize the 3D armature system
 * @param maxArmatures how many you want to be able to hold in memory at once
 */
void gf3d_armature_system_init(Uint32 maxArmatures);

/**
 * @brief get a blank armature for use
 * @return NULL on error, or a blank armature
 */
Armature3D *gf3d_armature_new();

/**
 * @brief free an armature no longer needed
 * @param arm the armature to free
 */
void gf3d_armature_free(Armature3D *arm);

/**
 * @brief load an armature from a file
 * @note only gltf 2.0 is supported at this time
 * @param filename the file to load
 * @return NULL on error, or the loaded armature otherwise
 */
Armature3D *gf3d_armature_load(const char *filename);

/**
 * @brief get the array of bone pose matrices for a given armature's frame
 * @param armature the armature in question
 * @param frame the frame to get
 * @param bonecount [output] if provided, this will be set to the number of bones in the matrix array
 * @return NULL on error, or an array of matrices for each bone in index order.
 */
GFC_Matrix4 *gf3d_armature_get_pose_matrices(Armature3D *armature,Uint32 frame,Uint32 *boneCount);

/**
 * @brief parse armature data out of a json file.
 * @note only gltf 2.0 is supported at this time
 * @param gltf the gltf file to parse
 * @return NULL on error, or the loaded armature otherwise
 */
Armature3D *gf3d_armature_parse(GLTF *gltf);

/**
 * @brief get a bone from an armature by its name
 * @param armature the armature to search
 * @param name the search critera
 * @return NULL on error or not found. A pointer to the bone otherwise
 */
Bone3D *gf3d_armature_get_bone_by_name(Armature3D *armature,const char *name);

/**
 * @brief get the global matrix (in model space) for a bone.
 * @param armature the armature to search
 * @param name the name of the bone to find
 * @return NULL if not found or error.  A pointer to the GFC_Matrix otherwise
 */
GFC_Matrix4 *gf3d_armature_get_bone_matrix_by_name(Armature3D *armature,const char *name);

/**
 * @brief get the global matrix (in model space) for a bone pose
 * @param armature the armature to search
 * @param frame the pose frame to grab
 * @param name the bone to find
 * @return NULL on error or not found, the matrix otherwise.  
 * @note this should be used to snap a model to a bone.  Further adjustments may be necessary
 */
GFC_Matrix4 *gf3d_armature_get_bone_pose_matrix_by_name(Armature3D *armature,Uint32 frame,const char *name);

/**
 * @brief draw a primitive line representation of an armature
 * @param arm the armature to draw
 */
void gf3d_armature_draw_bones(Armature3D *arm);

/**
 * @brief draw a primitive line representation of an armature pose
 * @param arm the armature to draw
 * @param frame the pose frame to draw
 */
void gf3d_armature_draw_bone_poses(Armature3D *arm,Uint32 frame);

/**
 * @brief translate a bone ID to its index in the bone list.
 * @note needed to sync poses to mesh bones/weights
 * @param armature the armature to poll
 * @param bone the bone in question.
 * @return -1 on error or not found, the index starting from zero otherwise
 */
int gf3d_armature_get_bone_index(Armature3D *armature,Uint32 boneId);

/**
 * @brief get the armature uniform buffer for the given pose
 * @param armature the armature in question
 * @param frame the pose to use
 * @return the armature UBO populated if there were any bones
 */
ArmatureUBO gf3d_armature_get_ubo(
    Armature3D *armature,
    Uint32      frame);

#endif
