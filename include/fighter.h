#ifndef __FIGHTER_H__
#define __FIGHTER_H__

#include "gf3d_entity.h"

/**
 * @brief Create a new fighter entity
 * @param position where to spawn the entity at
 * @return NULL on error, or an entity pointer on success
 */
Entity *fighter_new(Vector3D position);


#endif
