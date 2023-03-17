#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_color.h"
#include "gfc_primitives.h"
#include "gfc_shape.h"

#include "gf2d_body.h"
#include "gf2d_actor.h"
#include "gf3d_model.h"

typedef struct Entity_S
{
    Uint8       _inuse;         /**<keeps track of memory usage*/
    TextLine    name;
    ModelMat    mat;            /**<orientation matrix for the model for 3D*/
    Actor      *actor;          /**<actor for 2D entities*/
    Action     *action;         /**<which action is current*/
    float       frame;          /**<current animation frame*/
    int         actionReturn;   /**<from the last frame this entity updated*/
    
    Shape       shape;          /**<2d shape for collisions in 2D space*/
    Body        body;           /**<instance for collisions in 2D space*/
    
    Color       color;          /**<default color for the model*/
    Color       detailColor;    /**<detail color for the model*/
    Color       selectedColor;  /**<Color for highlighting*/
    Uint8       hidden;         /**<if true, not drawn*/
    Uint8       selected;
    
    Box         bounds;         // for collisions
    int         team;           //same team dont clip
    int         clips;          // if false, skip collisions

    void       (*think)(struct Entity_S *self); /**<pointer to the think function*/
    void       (*update)(struct Entity_S *self); /**<pointer to the update function*/
    void       (*draw)(struct Entity_S *self); /**<pointer to an optional extra draw funciton*/
    void       (*takeDamage)(struct Entity_S *self, float damage, struct Entity_S *inflictor); /**<pointer to the think function*/
    void       (*onDeath)(struct Entity_S *self); /**<pointer to an funciton to call when the entity dies*/
    void       (*free)(struct Entity_S *self); /**<pointer to the custom free function, necessar when there is custom data*/
        
    float       roll;           //kept separate 
    float       rotation;       /**<for 2D actor rotation*/
    Vector2D    flip;           /**<for 2d actor drawing*/
    Vector3D    velocity;
    Vector3D    acceleration;
    Vector3D    targetPosition;
    
    float       speed;// how fast it moves
    Bool        targetComplete;
    int         counter;//generic counting variable
    float       cooldown;
            
    Uint32      health;         /**<entity dies when it reaches zero*/
    float       damage;
    // WHATEVER ELSE WE MIGHT NEED FOR ENTITIES
    struct Entity_S *parent;    /**<entity that spawned this one*/
    struct Entity_S *target;    /**<entity to target for weapons / ai*/
    struct Entity_S *killer;    /**<entity that killed this one*/
    
    void *data;   /**<IF an entity needs to keep track of extra data, we can do it here*/
}Entity;

/**
 * @brief initializes the entity subsystem
 * @param maxEntities the limit on number of entities that can exist at the same time
 */
void gf3d_entity_system_init(Uint32 maxEntities);

/**
 * @brief provide a pointer to a new empty entity
 * @return NULL on error or a valid entity pointer otherwise
 */
Entity *gf3d_entity_new();

/**
 * @brief free a previously created entity from memory
 * @param self the entity in question
 */
void gf3d_entity_free(Entity *self);


/**
 * @brief Draw an entity in the current frame
 * @param self the entity in question
 */
void gf3d_entity_draw(Entity *self);

/**
 * @brief draw ALL active entities to the 3D pipeline
 */
void gf3d_entity_draw_all();

/**
 * @brief draw ALL active entities to the 2D pipeline
 */
void gf3d_entity_draw_all_2d();

/**
 * @brief update the position of the entity based on its velocity in 3D space
 * @param self the entity to update
 */
void gf3d_entity_update_position_3d(Entity *self);

/**
 * @brief rotate an entity to match the given direction
 * @param self the entity to rotate
 * @param dir the direction to rotate it by
 */
void gf3d_entity_rotate_to_dir_3d(Entity *self,Vector3D dir);

/**
 * @brief Call an entity's think function if it exists
 * @param self the entity in question
 */
void gf3d_entity_think(Entity *self);

/**
 * @brief run the think functions for ALL active entities
 */
void gf3d_entity_think_all();

/**
 * @brief run the update functions for ALL active entities
 */
void gf3d_entity_update_all();

/**
 * @brief get the entity's position
 * @param self the entity to check
 * @return a vector with their position
 */
Vector3D gf3d_entity_get_position(Entity *self);

/**
 * @brief get an entity by its name.  Names are not guaranteed to be unique, so be careful
 * @param name the name to search for
 * @return NULL if not found, or the first entity with the given name
 */
Entity *gf3d_entity_get_by_name(const char *name);



void gf3d_entity_rotate_to_dir(Entity *self,Vector2D dir);

#endif
