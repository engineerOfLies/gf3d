#include <stdlib.h>
#include <string.h>

#include "simple_logger.h"

#include "entity.h"
#include "world.h"

typedef struct
{
    Entity *entity_list;
    Uint32  entity_count;
    
}EntityManager;

static EntityManager entity_manager = {0};

void entity_system_close()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        entity_free(&entity_manager.entity_list[i]);        
    }
    free(entity_manager.entity_list);
    memset(&entity_manager,0,sizeof(EntityManager));
    slog("entity_system closed");
}

void entity_system_init(Uint32 maxEntities)
{
    entity_manager.entity_list = gfc_allocate_array(sizeof(Entity),maxEntities);
    if (entity_manager.entity_list == NULL)
    {
        slog("failed to allocate entity list, cannot allocate ZERO entities");
        return;
    }
    entity_manager.entity_count = maxEntities;

    atexit(entity_system_close);
    slog("entity_system initialized");
}

Entity *entity_new()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet, so we can!
        {
            entity_manager.entity_list[i]._inuse = 1;
            gfc_matrix_identity(entity_manager.entity_list[i].modelMat);
            entity_manager.entity_list[i].scale.x = 1;
            entity_manager.entity_list[i].scale.y = 1;
            entity_manager.entity_list[i].scale.z = 1;
            
            entity_manager.entity_list[i].color = gfc_color(1,1,1,1);
            entity_manager.entity_list[i].selectedColor = gfc_color(1,1,1,1);
            
            return &entity_manager.entity_list[i];
        }
    }
    slog("entity_new: no free space in the entity list");
    return NULL;
}

void entity_free(Entity *self)
{
    if (!self)return;
    //MUST DESTROY
    gf3d_model_free(self->model);
    memset(self,0,sizeof(Entity));
}


void entity_draw(Entity *self)
{
    if (!self)return;
    if (self->hidden)return;
    gf3d_model_draw(self->model,self->modelMat,gfc_color_to_vector4f(self->color),vector4d(1,1,1,1));
    if (self->selected)
    {
        gf3d_model_draw_highlight(
            self->model,
            self->modelMat,
            gfc_color_to_vector4f(self->selectedColor));
    }
}

void entity_draw_all()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet
        {
            continue;// skip this iteration of the loop
        }
        entity_draw(&entity_manager.entity_list[i]);
    }
}

void entity_think(Entity *self, float deltaTime)
{
    if (!self)return;
    if (self->think)self->think(self, deltaTime);
}

void entity_think_all(float deltaTime)
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet
        {
            continue;// skip this iteration of the loop
        }
        entity_think(&entity_manager.entity_list[i], deltaTime);
    }
}


void entity_update(Entity *self, float deltaTime)
{
    Vector3D scaledVelocity;
    Vector3D scaledAcceleration;
    if (!self)return;
    // HANDLE ALL COMMON UPDATE STUFF
    
    vector3d_scale(scaledVelocity, self->velocity, deltaTime);
    vector3d_scale(scaledAcceleration, self->acceleration, deltaTime);
    vector3d_add(self->position, self->position, scaledVelocity);
    vector3d_add(self->velocity, self->velocity, scaledAcceleration);
    
    gfc_matrix_identity(self->modelMat);
    
    gfc_matrix_scale(self->modelMat,self->scale);
    gfc_matrix_rotate_by_vector(self->modelMat,self->modelMat,self->rotation);
    gfc_matrix_translate(self->modelMat,self->position);
    
    if (self->update) self->update(self, deltaTime);
}

void entity_update_all(float deltaTime)
{
    int i, j;
    World* world = get_world();
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet
        {
            continue;// skip this iteration of the loop
        }
        entity_update(&entity_manager.entity_list[i], deltaTime);
        // scans through every entity and checks collision
        for (j = 0; j < entity_manager.entity_count; j++)
        {
            if (i == j || !entity_manager.entity_list[j]._inuse)
            {
                // skip if trying to compare the same entity or if the next entity in the list doesn't exist
                continue;
            }

            if (bounding_box_collision(&entity_manager.entity_list[i], &entity_manager.entity_list[j]))
            {
                // Just stops the entities from moving if they collide
                entity_manager.entity_list[i].velocity.x = 0;
                entity_manager.entity_list[i].velocity.y = 0;
                entity_manager.entity_list[i].velocity.z = 0;
            }
                // scans entities and compares with world bounding box for collision with world 
                if (world_bounding_box_collision(&entity_manager.entity_list[i], world))
                {
                    // Entities will stop falling when colliding with the world
                    entity_manager.entity_list[i].velocity.z = 0;
                }
        }
    }
}

Vector3D get_Bounding_Box_Max(Vector3D size, Vector3D position)
{
    Vector3D max;
    max.x = position.x + size.x / 2;
    max.y = position.y + size.y / 2;
    max.z = position.z + size.z / 2;
    return max;
}

Vector3D get_Bounding_Box_Min(Vector3D size, Vector3D position)
{
    Vector3D min;
    min.x = position.x - size.x / 2;
    min.y = position.y - size.y / 2;
    min.z = position.z - size.z / 2;
    return min;
}

int bounding_box_collision(Entity* a, Entity* b)
{
    if (a->boundingBox.max.x < b->boundingBox.min.x || a->boundingBox.min.x > b->boundingBox.max.x)
    {
        return 0;
    }
    if (a->boundingBox.max.y < b->boundingBox.min.y || a->boundingBox.min.y > b->boundingBox.max.y)
    {
        return 0;
    }
    if (a->boundingBox.max.z < b->boundingBox.min.z || a->boundingBox.min.z > b->boundingBox.max.z)
    {
        return 0;
    }
    return 1;
}

int world_bounding_box_collision(Entity* a, World* b)
{

    if (a->boundingBox.max.x < b->worldBoundingBox.min.x || a->boundingBox.min.x > b->worldBoundingBox.max.x) 
    {
        return 0;
    }
    if (a->boundingBox.max.y < b->worldBoundingBox.min.y || a->boundingBox.min.y > b->worldBoundingBox.max.y) 
    {
        return 0;
    }
    if (a->boundingBox.max.z < b->worldBoundingBox.min.z || a->boundingBox.min.z > b->worldBoundingBox.max.z)
    {
        return 0;
    }
    // If we reach here, there is overlap along all three axes, so there is a collision
    return 1;
}

/*eol@eof*/
