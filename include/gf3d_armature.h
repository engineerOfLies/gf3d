#ifndef __GF3D_ARMATURE_H__
#define __GF3D_ARMATURE_H__

#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_matrix.h"

typedef struct Bone3D_S
{
    TextLine            name;           /**<name of bone*/
    Uint32              index;          /**<place in the list*/
    Uint32              nodeId;         /**<for parsing*/
    struct Bone3D_S    *parent;         /**<pointer to the parent of the bone*/
    List               *children;       /**<list of indicies to any children, no data is allocated for this*/
    Matrix4             mat;            /**<matrix describing the bone orientation*/
}Bone3D;

typedef struct
{
    Bone3D     *bone;       /**<original bone, referenced and not owned*/
    Matrix4     mat;        /**<matrix describing the bone orientation*/
}BonePose3D;

typedef struct
{
    List *poseBones;        /**<list of bone poses*/
}Pose3D;

typedef struct
{
    TextLine    filepath;       /**<the file that this has been loaded from / to*/
    TextLine    name;           /**<printing name*/
    Uint32      refCount;       /**<resurce management*/
    List       *bones;          /**<list of Bones in the base armature*/
    List       *poses;          /**<list of poses for the armature*/
    List       *actions;        /**<action list for managing animation of poses*/
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
 * @brief parse armature data out of a json file.
 * @note only gltf 2.0 is supported at this time
 * @param config the json to parse
 * @return NULL on error, or the loaded armature otherwise
 */
Armature3D *gf3d_armature_parse(SJson *config);

/**
 * @brief draw a primitive line representation of an armature
 * @param arm the armature to draw
 */
void gf3d_armature_draw_bones(Armature3D *arm);


#endif
