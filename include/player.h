#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
* @brief spawn a player
* @return NULL on error, or a pointer of the player otherwise
*/
Entity *player_new(GFC_TextLine name, Model* model, GFC_Vector3D spawnPosition);
#endif