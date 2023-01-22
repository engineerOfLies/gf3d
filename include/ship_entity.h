#ifndef __SHIP_ENTITY_H__
#define __SHIP_ENTITY_H__

#include "gfc_color.h"
#include "gf3d_entity.h"
#include "ship.h"

Entity *ship_entity_new(Vector3D position,Ship *data,Color detailColor);

/**
 * @brief set the ship to move towards the given location at its speed;
 * @param ent the ship entity to move
 * @param pathIndex the part of the flightpath to move towards
 */
void ship_entity_move_to(Entity *ent,Uint32 pathIndex);

#endif
