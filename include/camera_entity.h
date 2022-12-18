#ifndef __CAMERA_ENTITY_H__
#define __CAMERA_ENTITY_H__

#include "entity.h"

/**
 * @brief Create a new camera entity
 * @param position where to spawn the entity at
 * @return NULL on error, or an entity pointer on success
 */
Entity *camera_entity_new(Vector3D position,Vector3D rotation);

/**
 * @brief enable/disable free look mode
 * @param enable if true, free look will be on
 */
void camera_entity_enable_free_look(Uint8 enable);

/**
 * @brief switch free look mode on and off
 */
void camera_entity_toggle_free_look();

/**
 * @brief set this position as the target to look at during auto pan mode
 * @param target the point to look at
 */
void camera_entity_set_look_target(Vector3D target);

/**
 * @brief get the current look target location
 * @return the target position
 */
Vector3D camera_entity_get_look_target();

#endif
