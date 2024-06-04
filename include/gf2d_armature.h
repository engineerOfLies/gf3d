#ifndef __GF2D_ARMATURE_H__
#define __GF2D_ARMATURE_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_actions.h"
#include "gfc_vector.h"

#include "gf2d_sprite.h"

typedef struct Bone_S
{
    GFC_TextLine        name;           /**<name of bone*/
    Uint32          index;          /**<place in the list*/
    struct Bone_S  *parent;         /**<pointer to the parent of the bone*/
    GFC_List           *children;       /**<list of indicies to any children, no data is allocated for this*/
    float           baseAngle;      /**<rest angle for the bone*/
    float           length;         /**<length of the bone*/
    GFC_Vector2D        rootPosition;   /**<root position of the bone*/
}Bone;

typedef struct
{
    Bone       *bone;       /**<original bone, referenced and not allocated*/
    float       angle;      /**<rotation for the bone*/
    float       length;     /**<length delta from original bone*/
    GFC_Vector2D    position;   /**<position to move the bone to*/
}BonePose;

typedef struct
{
    GFC_List *poseBones;    /**<list of bone poses*/
}Pose;

typedef struct
{
    GFC_TextLine    filepath;       /**<the file that this has been loaded from / to*/
    GFC_TextLine    name;           /**<printing name*/
    Uint32          refCount;       /**<resurce management*/
    GFC_List       *bones;          /**<list of Bones in the base armature*/
    GFC_List       *poses;          /**<list of poses for the armature*/
    GFC_ActionList *actions;        /**<action list for managing animation of poses*/
}Armature2D;

/**
 * @brief initialize the armature system
 * @param maxArmatures how many concurrent armatures to support
 */
void gf2d_armature_init(Uint32 maxArmatures);

/**
 * @brief get a pointer to a blank armature
 * @returns NULL on error or out of space, a blank armature otherwise
 */
Armature2D *gf2d_armature_new();

/**
 * @brief load an armature from a json file
 * @param filepath the filename and path to the armature file
 * @returns NULL on file not found or error (see logs), A setup armature otherwise
 */
Armature2D *gf2d_armature_load(const char *filepath);

/**
 * @brief save an armature to json
 * @param armature the armature to save
 * @param filepath the file and path to save to
 */
void gf2d_armature_save(Armature2D *armature, const char *filepath);

/**
 * @brief draw a sprite to the screen based on an armature's bone pose
 * @param armature the armature to draw with
 * @param sprite the sprite to draw
 * @param position here on the screen to draw it
 * @param scale (optional) if you want to scale the sprite
 * @param center (optional) the center point for scaling and rotating
 * @param rotation (optional) the angle in degrees to rotate
 * @param flip (optional) set to 1 if you want to flip in the horizontal,vertical axis
 * @param colorShift (optional) if you want to gamma shift the sprite or set an alpha value
 * @param frame which frame to draw
 * @param pose which armature pose to draw
 * @param bone which bone to draw to
 */
void gf2d_armature_draw_sprite_to_bone_pose(
    Armature2D *armature,
    Sprite *sprite,
    Uint32 frame,
    GFC_Vector2D position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    float    * rotation,
    GFC_Vector2D * flip,
    GFC_Color    * colorShift,
    Uint32 pose,
    Uint32 bone);

/**
 * @brief draw a sprite to the screen based on an armature's bone pose
 * @param armature the armature to draw with
 * @param sprite the sprite to draw
 * @param position here on the screen to draw it
 * @param scale (optional) if you want to scale the sprite
 * @param center (optional) the center point for scaling and rotating
 * @param rotation (optional) the angle in degrees to rotate
 * @param flip (optional) set to 1 if you want to flip in the horizontal,vertical axis
 * @param colorShift (optional) if you want to gamma shift the sprite or set an alpha value
 * @param frame which frame to draw
 * @param pose which armature pose to draw
 * @param name which bone to draw to
 */
void gf2d_armature_draw_sprite_to_named_bone_pose(
    Armature2D *armature,
    Sprite *sprite,
    Uint32 frame,
    GFC_Vector2D position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    float    * rotation,
    GFC_Vector2D * flip,
    GFC_Color    * colorShift,
    Uint32 pose,
    const char *name);

/**
 * @brief free a previously loaded armature
 * @note memory address will be re-used, do no access the addres anymore
 * @param armature the armature to free
 */
void gf2d_armature_free(Armature2D *armature);

/**
 * @brief get the action for an armature by name
 * @param armature the armature to set an action with
 * @param name the name of the action to search for
 * @param frame (optional output) set the starting frame for the named action if found
 * @return NULL on not found or error, the action information otherwise
 */
GFC_Action *gf2d_armature_get_action(Armature2D *armature, const char *name,float *frame);

/**
 * @brief get the action from the action list by index
 * @param armature the armature to query
 * @param index the index of the action to get
 * @return NULL on not found, or the action in question
 */
GFC_Action *gf2d_armature_get_action_by_index(Armature2D *armature,Uint32 index);


/**
 * @brief draw the skeletal outline of the armature
 * @param armature the armature to draw
 * @param position the screen position to draw to
 * @param scale scaling factor for the draw call
 * @param rotation angle to draw it at
 * @param color the color to use to draw
 */
void gf2d_armature_draw_bones(Armature2D *armature,GFC_Vector2D position, GFC_Vector2D scale, float rotation, GFC_Color color);

/**
 * @brief draw a pose of an armature
 * @param armature the armature to draw
 * @param poseindex the index of the pose to draw
 * @param position the draw offset for the entire armature
 * @param scale scaling factor for the draw call
 * @param rotation angle to draw it at
 * @param color the color to draw with
 */
void gf2d_armature_draw_pose(Armature2D *armature,Uint32 poseindex,GFC_Vector2D position,GFC_Vector2D scale, float rotation, GFC_Color color);

/**
 * @brief get an interpolation of a bonepose between two bones
 * @param armature the armature to query
 * @param poseA the first pose
 * @param poseB the second pose
 * @param index which bone
 * @param weight percentage of the way from poseA to boseB between 0 (all A) and 1 (All B)
 */
BonePose gf2d_armature_get_tweened_pose_bone(Armature2D *armature,Uint32 poseA, Uint32 poseB,Uint32 index, float weight);

/**
 * @brief draw an interpolation between two poses
 * @param armature the armature to draw
 * @param poseA the first pose
 * @param poseB the second pose
 * @param fraction percentage of the way from poseA to boseB between 0 (all A) and 1 (All B)
 * @param position where to draw the armature
 * @param scale how to scale it
 * @param rotation if we rotate it
 * @param color in what color
 */
void gf2d_armature_draw_tweened_pose(
    Armature2D *armature,
    Uint32 poseA,
    Uint32 poseB,
    float fraction,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    float rotation,
    GFC_Color color);

/**
 * @brief draw a pose bone to the screen
 * @param bonePose the bonepose to draw
 * @param position the position of the base armature
 * @param scale scaling factor for the draw call
 * @param rotation angle to draw it at
 * @param color the color to draw with
 */
void gf2d_armature_draw_pose_bone(BonePose *bonePose,GFC_Vector2D position,GFC_Vector2D scale, float rotation,GFC_Color color);

/**
 * @brief draw a single bone
 * @param position this is the position of the armature center
 * @param angle at this angle
 * @param length of the bone
 * @param color in this color
 */
void gf2d_armature_bone_draw(GFC_Vector2D position, float angle, float length, GFC_Color color);

/**
 * @brief draw a specific bone with respect to draw scale
 * @param bone the bone to draw
 * @param position draw offset for the bone
 * @param scale the draw scale
 * @param rotation draw rotation
 * @param color the color to draw with
 */
void gf2d_armature_draw_bone(Bone *bone,GFC_Vector2D position, GFC_Vector2D scale, float rotation, GFC_Color color);

/**
 * @brief get a bone by its tip position
 * @param armature the armature to query
 * @param position the position (in armature space) to check for
 * @param scale the drawscale of the armature
 * @param ignore if not NULL, this will bone will be skipped in the search
 * @return NULL on error or no bones, the bone otherwise
 */
Bone *gf2d_armature_get_bone_by_position(Armature2D *armature,GFC_Vector2D position,GFC_Vector2D scale,Bone *ignore);

/**
 * @brief get a copy of a bone
 * @param bone the bone to copy
 * @return an empty bone if it was null
 */
Bone gf2d_armature_bone_copy(Bone *bone);

/**
 * @brief set the parenting for the two bones
 * @param parent the bone to become the parent
 * @param child the bone to become the child
 */
void gf2d_armature_add_bone_to_parent(Bone *parent,Bone *child);

/**
 * @brief get a posebone by its tip position
 * @param armature the armature to query
 * @param poseindex which pose to check
 * @param position the position (in armature space) to check for
 * @param scale the drawscale to adjust the bone by
 * @param rotation the draw rotation to ajust the bone by
 * @param ignore if not null this bone will be skipped in the selection
 * @return NULL on error or no bones, the posebone otherwise
 */
BonePose *gf2d_armature_get_bonepose_by_position(
    Armature2D *armature,
    Uint32 poseindex,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    float rotation,
    BonePose *ignore);

/**
 * @brief get a bone by its name
 * @param armature the armature to query
 * @param name the name of the bone to find
 * @returns NULL on error or not found, a valid pointer otherwise
 */
Bone *gf2d_armature_get_bone_by_name(Armature2D *armature, const char *name);

/**
 * @brief get a bone by its index
 * @param armature the armature to query
 * @param index the index of the bone to find
 * @returns NULL on error or not found, a valid pointer otherwise
 */
Bone *gf2d_armature_get_bone_by_index(Armature2D *armature, Uint32 index);

/**
 * @brief get a bone pose based on specific pose and bone index
 * @param armature the armature to query
 * @param pose the pose index
 * @param index the bone index
 * @return NULL on not found, or the BonePose information
 */
BonePose *gf2d_armature_get_bone_pose(Armature2D *armature,Uint32 pose, Uint32 index);

/**
 * @brief get a bone pose based on specific pose and bone name
 * @param armature the armature to query
 * @param pose the pose index
 * @param name the bone name
 * @return NULL on not found, or the BonePose information
 */
BonePose *gf2d_armature_get_bone_pose_by_name(Armature2D *armature,Uint32 pose, const char *name);

/**
 * @brief get the position of the bone's tip by name
 * @param armature the armature containing the bone
 * @param bonename the name of the bone to search for
 * @param pose which pose to check
 * @param scale scale values by
 * @param rotation rotated by this much
 */
GFC_Vector2D gf2d_armature_get_bonepose_tip_by_name(
    Armature2D *armature,
    const char *bonename,
    Uint32 pose,
    GFC_Vector2D scale,
    float rotation);

//manipulation

/**
 * @brief rotate a bone and all its children by the angle provided
 * @param armature the armature to rotate the bones of
 * @param name the name of the bone to rotate
 * @param angle the amount to rotate by in radians
 */
void gf2d_armature_bone_rotate_by_name(Armature2D *armature,const char *name, float angle);

/**
 * @brief move a bonepose and all of its children by delta provided
 * @param armature the armature owning the posebone to rotate
 * @param bonepose the bonepose to rotate
 * @param poseindex the pose index for the bone
 * @param delta the amount to move
 */
void gf2d_armature_bonepose_move(Armature2D *armature, BonePose *bonepose,Uint32 poseindex, GFC_Vector2D delta);

/**
 * @brief rotate a bonepose and all of its children by the angle provided
 * @param armature the armature owning the posebone to rotate
 * @param bonepose the bonepose to rotate
 * @param poseindex the pose index for the bone
 * @param center the point about which to rotate
 * @param angle how much to rotate the bone in radians
 */
void gf2d_armature_bonepose_rotate(Armature2D *armature, BonePose *bonepose,Uint32 poseindex, GFC_Vector2D center, float angle);

/**
 * @brief get the root position for the bonepose given that it has been scaled
 * @param posebone the bonepose to query
 * @param scale the scaling factor for drawing
 * @param rotation if it has been rotated as well
 */
GFC_Vector2D gf2d_armature_get_pose_bone_draw_position(BonePose *posebone,GFC_Vector2D scale,float rotation);

/**
 * @brief get the bonepose position in armature space
 * @param posebone the bonepose to get
 * @return a zero gfc_vector on error, or the position relative to the armature (could still be zero)
 */
GFC_Vector2D gf2d_armature_get_pose_bone_position(BonePose *posebone);

/**
 * @brief rotate the given bone by the amount
 * @param bone the bone to rotate (also handles children
 * @param center the point around to rotate
 * @param angle how much to rotate by
 */
void gf2d_armature_bone_rotate(Bone *bone, GFC_Vector2D center, float angle);

/**
 * @brief move the bone (and its children) to the new position relative to the armature
 * @param bone the bone to move
 * @param newPosition the new position to move to
 */
void gf2d_armature_bone_move_to(Bone *bone,GFC_Vector2D newPosition);

/**
 * @brief move the bone (and its children)
 * @param bone the bone to move
 * @param offset how much to move by
 */
void gf2d_armature_bone_move(Bone *bone,GFC_Vector2D offset);

/**
 * @brief move the tip of the bone
 * @param bone the bone to move
 * @param offset how much to move by
 */
void gf2d_armature_bone_tip_move(Bone *bone,GFC_Vector2D offset);

/**
 * @brief scale the bone (and its children)
 * @param bone the bone to scale
 * @param scale the factor to scale the bone by
 * @param center scale relative to this position
 */
void gf2d_armature_bone_scale_children(Bone *bone,GFC_Vector2D scale, GFC_Vector2D center);

/**
 * @brief scale the entire armature by the provided scale
 * @param armature the armature to scale
 * @param scale how much to scale each dimension by
 * @param center where to scale relative to
 */
void gf2d_armature_scale(Armature2D *armature,GFC_Vector2D scale,GFC_Vector2D center);

/**
 * @brief get the location of the bone tip in armature space
 * @param bone the bone to get
 * @return a zero gfc_vector on error, or the tip location in armature space
 */
GFC_Vector2D gf2d_armature_get_bone_tip(Bone *bone);

/**
 * @brief get the tip of the posebone
 */
GFC_Vector2D gf2d_armature_get_bonepose_tip(BonePose *posebone,GFC_Vector2D scale, float rotation);

/**
 * @brief create a new bone and its relative bone poses
 * @param armature the armature to add to
 * @param parent (if not null) set this as the parent bone
 * @return NULL on error or a pointer to the newly created bone
 */
Bone *gf2d_armature_add_bone(Armature2D* armature,Bone *parent);

/**
 * @brief delete a bone, its children and poses from the armature
 * @param armature the armature from which the bone belongs
 * @param bone the bone in question to delete
 */
void gf2d_armature_delete_bone(Armature2D *armature, Bone *bone);

/**
 * @brief make a copy of a bone and all its children
 * @param armature the armature to add bones to
 * @param bone the bone to copy
 * @return NULL on error, or a pointer to the new bone otherwise
 */
Bone *gf2d_armature_duplicate_bone(Armature2D *armature, Bone *bone);

/**
 * @brief check if a bone is named with a .l or .left and switch it to .r or .right or vice versa
 * @param newname the new bone name will be written here (dst)
 * @param oldname the name to check.
 * @note if the oldname does not end with the required suffix, nothing is done
 * @returns 1 if the flip happened (and newname was changed) or 0 if nothing was done
 */
int gf2d_armature_bone_name_lr_flip(char *newname,const char *oldname);

/**
 * @brief allocate memory for a new empty pose
 * @return NULL on memory error or a valid pointer to a new BonePose
 */
BonePose *gf2d_armature_bone_pose_new();

/**
 * @brief make a new copy of an existing pose for an armature
 * @param armature the armature to duplicate it for
 * @param pose the pose to duplicate
 * @return NULL on error or the newly duplicated pose
 */
Pose *gf2d_armature_pose_duplicate(Armature2D *armature,Pose *pose);

/**
 * @brief free a pose, note this does not remove it from an armature.
 * @param pose the pose to free.
 */
void gf2d_armature_pose_free(Pose *pose);

/**
 * @brief copy the pose from src to dst
 * @note src and dst must have the same bone counts for this to work
 * @param dst the pose to be copied TO
 * @param src the pose to be copied FROM
 */
void gf2d_armature_pose_copy(Pose *dst,Pose *src);

/**
 * @brief create the needed pose and bones for a given armature
 * @param armature the armature to create a pose for
 * @return NULL on error, or a BLANK pose for an armature
 * @note the pose is NOT added TO the armature.
 */
Pose *gf2d_armature_pose_create_for(Armature2D *armature);

/**
 * @brief make a new copy of an existing pose for an armature and insert at the index
 * @param armature the armature to duplicate it for
 * @param pose the pose to duplicate
 * @param index the location to insert the new frame
 * @return NULL on error or the newly duplicated pose
 */
Pose *gf2d_armature_pose_duplicate_at_index(Armature2D *armature,Pose *pose,Uint32 index);

/**
 * @brief get the number of poses on an armature
 * @param armature the armature to query
 * @return the number of pose frames
 */
Uint32 gf2d_armature_get_pose_count(Armature2D *armature);

/**
 * @brief add a new blank pose to the armature
 * @param armature the armature to add a pose to
 * @return NULL on error or the created pose.  
 * @note this creates all the pose bones too
 */
Pose *gf2d_armature_pose_add(Armature2D *armature);

/**
 * @brief add a new blank pose to the armature at the index specified
 * @param armature the armature to add a pose to
 * @param index where to insert.  MUST be <= current count
 * @return NULL on error or the created pose.  
 * @note this creates all the pose bones too
 */
Pose *gf2d_armature_pose_add_at_index(Armature2D *armature,Uint32 index);

/**
 * @brief delete a single pose from the armature by index
 * @param armature the armature to modify
 * @param index the index of the pose to delete
 */
void gf2d_armature_delete_pose_by_index(Armature2D *armature,Uint32 index);

/**
 * @brief allocate and initialize a new pose
 * @return NULL on error or a blank pose otherwise
 */
Pose *gf2d_armature_pose_new();

/**
 * @brief get an armature's pose by its index
 * @param armature the armature to query
 * @param index the index to get
 * @return NULL on bad index or error, a valid Pose otherwise
 */
Pose *gf2d_armature_pose_get_by_index(Armature2D *armature,Uint32 index);

#endif
