#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "entity.h"

/**
* @brief spawn an object with physcics.
* @return NULL on error, or a pointer of the player otherwise
*/
Entity* obj_new(GFC_TextLine name, Model* model, GFC_Vector3D spawnPosition);
#endif