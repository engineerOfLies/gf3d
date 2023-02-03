#ifndef __CAMERA_ENTITY_H__
#define __CAMERA_ENTITY_H__

#include "gf3d_entity.h"

typedef enum
{
    CTT_None,
    CTT_Position,
    CTT_Entity
}CameraTargetType;


/**
 * @brief Create a new camera entity
 * @param position where to spawn the entity at
 * @return NULL on error, or an entity pointer on success
 */
Entity *gf3d_camera_entity_new(Vector3D position,Vector3D rotation);

/**
 * @brief enable/disable free look mode
 * @param enable if true, free look will be on
 */
void gf3d_camera_entity_enable_free_look(Uint8 enable);

/**
 * @brief switch free look mode on and off
 */
void gf3d_camera_entity_toggle_free_look();

/**
 * @brief check if the camera is in free look mode or not
 * @return true is freeLook is one, false otherwise
 */
Bool gf3d_camera_entity_free_look_enabled();

/**
 * @brief set the position of the camera
 * @param position where to put it
 */
void gf3d_camera_entity_set_position(Vector3D position);

/**
 * @brief get the position of the camera
 * @return where it is
 */
Vector3D gf3d_camera_entity_get_position();

/**
 * @brief set this position as the target to look at
 * @param target the point to look at
 */
void gf3d_camera_entity_set_look_target(Vector3D target);

/**
 * @brief get the current look target location
 * @return the target position
 */
Vector3D gf3d_camera_entity_get_look_target();

/**
 * @brief get the current look target entity
 * @return the target entity or NULL if not set
 */
Entity *gf3d_camera_entity_get_look_target_entity();

/**
 * @brief set the entity as the target to look at 
 * @param target the entuty to look at, or NULL to clear it
 */
void gf3d_camera_entity_set_look_target_entity(Entity *target);

/**
 * @brief set the camera to a specific target mode
 * @param mode the CameraTargetType to set it to
 */
void gf3d_camera_entity_set_look_mode(CameraTargetType mode);

/**
 * @brief set the camera to auto pan or not
 * @param enable if true turn on pan, if false, turn it off
 */
void gf3d_camera_entity_set_auto_pan(Bool enable);

#endif
