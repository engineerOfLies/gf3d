#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_color.h"
#include "gfc_primitives.h"

#include "gf3d_model.h"

typedef struct World World;
typedef enum
{
    ES_idle = 0,
    ES_hunt,
    ES_dead,
    ES_attack
}EntityState;


typedef struct Entity_S
{
    Uint8       _inuse;     /**<keeps track of memory usage*/
    Matrix4     modelMat;   /**<orientation matrix for the model*/
    Color       color;      /**<default color for the model*/
    Model      *model;      /**<pointer to the entity model to draw  (optional)*/
    Uint8       hidden;     /**<if true, not drawn*/
    Uint8       selected;
    Color       selectedColor;      /**<Color for highlighting*/
    
    Box         bounds; // for collisions
    int         team;  //same team dont clip
    int         clips;  // if false, skip collisions

    void       (*think)(struct Entity_S *self); /**<pointer to the think function*/
    void       (*update)(struct Entity_S *self); /**<pointer to the update function*/
    void       (*draw)(struct Entity_S *self); /**<pointer to an optional extra draw funciton*/
    void       (*damage)(struct Entity_S *self, float damage, struct Entity_S *inflictor); /**<pointer to the think function*/
    void       (*onDeath)(struct Entity_S *self); /**<pointer to an funciton to call when the entity dies*/
    
    EntityState state;
    
    Vector3D    position;  
    Vector3D    velocity;
    Vector3D    acceleration;
    Vector3D    size; // size of the entity for the bounding box

        
    Vector3D    scale;
    Vector3D    rotation;
    
    Uint32      health;     /**<entity dies when it reaches zero*/
    // WHATEVER ELSE WE MIGHT NEED FOR ENTITIES
    struct Entity_S *target;    /**<entity to target for weapons / ai*/
    
    void *customData;   /**<IF an entity needs to keep track of extra data, we can do it here*/
    struct {
        Vector3D min;
        Vector3D max;
    }boundingBox;
}Entity;

/**
 * @brief initializes the entity subsystem
 * @param maxEntities the limit on number of entities that can exist at the same time
 */
void entity_system_init(Uint32 maxEntities);

/**
 * @brief provide a pointer to a new empty entity
 * @return NULL on error or a valid entity pointer otherwise
 */
Entity *entity_new();

/**
 * @brief free a previously created entity from memory
 * @param self the entity in question
 */
void entity_free(Entity *self);


/**
 * @brief Draw an entity in the current frame
 * @param self the entity in question
 */
void entity_draw(Entity *self);

/**
 * @brief draw ALL active entities
 */
void entity_draw_all();

/**
 * @brief Call an entity's think function if it exists
 * @param self the entity in question
 */
void entity_think(Entity *self, float deltaTime);

/**
 * @brief run the think functions for ALL active entities
 */
void entity_think_all(float deltaTime);

/**
 * @brief run the update functions for ALL active entities
 */
void entity_update_all(float deltaTime);

/**
 * @brief Calculates the Max of the Bounding Box
 * @param size of the entity
 * @param positioin of the entity
 */
Vector3D get_Bounding_Box_Max(Vector3D size, Vector3D position);

/**
 * @brief Calculates the min of the Bounding Box
 * @param size of the entity
 * @param position of the entity
 */
Vector3D get_Bounding_Box_Min(Vector3D size, Vector3D position);

/**
 * @brief Detects if 2 entities are colliding with each other
 * @param Entity a
 * @param Entity b
 */
int bounding_box_collision(Entity* a, Entity* b);

/**
 * @brief Detects if and entity is colliding with the world
 * @param Entity a
 * @param World b
 */
int world_bounding_box_collision(Entity* a, World* b);
#endif
