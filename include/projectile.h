#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "entity.h"

/**
 * @brief spawn a new projectile into the system using a config file
 * @param parent who is firing it
 * @param position where to begin
 * @param dir where to go
 * @param configFile where to get the rest of the config
 * @return NULL on failure, a pointer to the projectile entity otherwise
*/
Entity *projectile_new_from_config(Entity *parent,Vector3D position, Vector3D dir, const char *configFile);

/**
 * @brief spawn a new projectile into the system
 * @param parent who is firing it
 * @param position where to begin
 * @param dir where to go
 * @param speed how fast we go
 * @param damage how much damage we do
 * @param damageType fire, water, leaf, etc
 * @return NULL on failure, a pointer to the projectile entity otherwise
*/
Entity *projectile_new(Entity *parent,const char * modelFile,Vector3D position, Vector3D dir, float speed,float damage,int damageType);


#endif
