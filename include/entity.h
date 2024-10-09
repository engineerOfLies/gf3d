#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gf3d_model.h"
#include "gfc_text.h"
#include "gfc_vector.h"

/**
* @purpose Make and manage entity
*/

typedef struct Entity_S
{
	Uint8				_inuse;		/**(private) Flag for keeping track of memory useage*/
	GFC_TextLine*		name;		/**Name of the entity*/
	GFC_Vector3D		position;	/**Where I am in space*/
	GFC_Vector3D		rotation;	/**How I should rotate*/
	GFC_Vector3D		scale;		/**Stretching*/
	GFC_Vector3D		velocity;	/**How much I should move in space*/
	Model* model;		/**My graphics*/
	//GFC_Matrix4 matrix;	/**My matrix*/
	// behavior
	void (*think)(struct Entity_S* self);		/**Function to call to make decisions*/
	void (*update)(struct Entity_S* self);		/**Function to call to execute think's decisions*/
	int  (*draw)(struct Entity_S* self);		/**Function to call to draw it's model*/
	void (*free)(struct Entity_S* self);		/**clean up any custom data*/
	void* data;									/**For ad hoc addition data for the entity*/
}Entity;

/**
*  @brief cleans up all entities
*/
void entity_system_close();

/**
*  @brief initialize the entity manager system and queues up cleanup on exit
*  @param max_ents the maximum number of entites that can exist at the same time.
*/
void entity_system_init(Uint32 max_ents);

/**
*  @brief clean up all active entites
*  @param ignore do not clean up this entity
*/
void entity_clear_all(Entity* ignore);

/**
* @brief get a blank entity to be used
* @return NULL on no more room or error, a blank entity otherwise
**/
Entity* entity_new();

/*
* @brief clean up an entity and free its spot for future use.
* @param self the entity to free
*/
void entity_free(Entity* self);

/**
* @brief run the think functions for all active entities
*/
void entity_system_think();

/**
* @brief run the think functions for all active entities
*/
void entity_system_update();

/**
* @brief draw all active entities
*/
void entity_system_draw();

/*
* @brief Use to draw anything
*/
void entity_draw(Entity *self);

/*
* @brief Draw all entities in the entity manager
* @return All entities drawn
*/
//void entity_draw_all();

#endif