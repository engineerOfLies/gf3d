#ifndef __GATE_H__
#define __GATE_H__

#include "gf3d_entity.h"

/**
 * @brief Create a new entity
 * @param def config information for the entity
 * @return NULL on error, or an entity pointer on success
 */
Entity *gate_new(SJson *def);


#endif
