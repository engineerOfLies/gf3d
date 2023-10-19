#include "simple_logger.h"
#include "projectile.h"


void projectile_think(Entity *self);
void projectile_update(Entity *self);


Entity *projectile_new_from_config(Entity *parent,Vector3D position, Vector3D dir, const char *configFile)
{
    SJson *config;
    float speed = 0;
    float damage = 0;
    int damageType = 0;
    const char * modelFile = NULL;
    
    config = sj_load(configFile);
    if (!config)
    {
        slog("failed to load projectile config file %s",configFile);
        return NULL;
    }
    // PARSE THE FILE
    sj_object_get_value_as_float(config,"speed",&speed);
    sj_object_get_value_as_float(config,"damage",&damage);
    sj_object_get_value_as_int(config,"damageType",&damageType);
    modelFile = sj_object_get_value_as_string(config,"model");
    sj_free(config);// clean up

    return projectile_new(parent,modelFile,position, dir, speed,damage,damageType);
}

Entity *projectile_new(Entity *parent,const char * modelFile,Vector3D position, Vector3D dir, float speed,float damage,int damageType)
{
    Entity *self;
    
    self = entity_new();
    if (!self)
    {
        slog("failed to make a new projectile!");
        return NULL;
    }
    
    self->parent = parent;
    vector3d_copy(self->position,position);
    vector3d_normalize(&dir);
    vector3d_scale(self->velocity,dir,speed);
    self->think = projectile_think;
    self->update = projectile_update;
    self->damage = damage;
    self->health = 1000;
    self->model = gf3d_model_load(modelFile);

    
    return self;
}

void projectile_think(Entity *self)
{
    Entity *other;
    if (!self)return;
    // do the dumb things I gotta do
    other = entity_get_collision_entity(self);
    if (other)
    {
        //this is where I die :.(
        if (other->onDamage)
        {
            other->onDamage(other, self->damage, self->parent?self->parent:self);
        }
        entity_free(self);
    }
}

void projectile_update(Entity *self)
{
    if (!self)return;
    self->health--;
    if (self->health <= 0)entity_free(self);
    //run physics, update position, check for collision, whatever else I gotta do
}


/*eol@eof*/
