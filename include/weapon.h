#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "entity.h"


/**
 * @brief Create a new weapon entity
 * @param position where to spawn the weapon at
 * @return NULL on error, or an weapon entity pointer on success
 */
Entity *weapon_new(void);

#endif


