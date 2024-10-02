#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gf3d_model.h"

typedef struct Entity_S
{
    Uint8           _inuse;     //flag for memory management
    GFC_TextLine    name;       //name of entity
    GFC_Vector3D    position;   //where I am in space
    GFC_Vector3D    rotation;   //how I should rotate
    GFC_Vector3D    scale;      //stretching
    Model          *model;      // my graphics
    //behavior
    void (*think)   (struct Entity_S *self);//called every frame for the entity to decide things
    void (*update)  (struct Entity_S *self);//called every frame for the entity to update its state
    int  (*draw)    (struct Entity_S *self);//if return -1, skip generic draw
    void (*free)    (struct Entity_S *self);//called when the entity is cleaned up to clean up custom data
    void           *data;//entity custom data - for everything beyond the basics
}Entity;

/**
 * @brief initialize the entity manager subsystem
 * @param maxEnts how many entities can exist at the same time
 */
void entity_system_init(Uint32 maxEnts);

/**
 * @brief draw all active entities
 */
void entity_draw_all();

/**
 * @brief have all entities decide on action
 */
void entity_think_all();

/**
 * @brief have all entities execute update actions
 */
void entity_update_all();

/**
 * @brief allocated a blank entity for use
 * @return NULL on failure (out of memory) or a pointer to the initialized entity
 */
Entity *entity_new();

/**
 * @brief performs a generic draw call for the entity
 */
void entity_draw_generic(Entity * self);

/**
 * @brief return the memory of a previously allocated entity back to the pool.
 * @param self the entity to free
 */
void entity_free(Entity *self);


#endif
