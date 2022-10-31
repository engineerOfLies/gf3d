#ifndef __STATION_H__
#define __STATION_H__

#include "entity.h"

/**
 * @brief Create a new station entity
 * @param position where to spawn the aguman at
 * @return NULL on error, or an agumon entity pointer on success
 */
Entity *station_new(Vector3D position);


#endif
