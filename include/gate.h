#ifndef __GATE_H__
#define __GATE_H__

#include "gf3d_entity.h"

/**
 * @brief Create a new entity
 * @param position where to spawn the entity at
 * @return NULL on error, or an entity pointer on success
 */
Entity *gate_new(Vector3D position);


#endif
