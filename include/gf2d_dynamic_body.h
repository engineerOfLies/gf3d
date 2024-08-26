#ifndef __GF2D_DYNAMIC_BODY_H__
#define __GF2D_DYNAMIC_BODY_H__

#include "gf2d_body.h"
#include "gf2d_collision.h"

typedef struct
{
    Uint32          id;
    Body           *body;
    GFC_Shape       shape;          /**<shape with offset from the body*/
    Uint8           blocked;        /**<true once this body has been blocked and no longer moves*/
    GFC_Vector2D    position;       /**<temp position during update*/
    GFC_Vector2D    oldPosition;    /**<previous position during update*/
    GFC_Vector2D    velocity;       /**<scaled velocity based on space step*/
    double          speed;          /**<scalar speed of velocity of the body*/
    GFC_List       *collisionList;  /**<list of collisions accrued during space update*/
    GFC_List       *bucketList;     /**<list of space buckets we sit in*/
}DynamicBody;

DynamicBody *gf2d_dynamic_body_new();

/**
 * @brief get the shape (adjusted for current position) from the dynamic body
 * @param a the dynamic body to get the shape for
 * @returns a shape
 */
GFC_Shape gf2d_dynamic_body_to_shape(DynamicBody *a);

void gf2d_dynamic_body_free(DynamicBody *db);

/**
 * @brief reset a dynamic body before a space update
 * @param db the dynamic body to reset
 * @param factor the time set factor to update by
 */
void gf2d_dynamic_body_reset(DynamicBody *db,float factor);

/**
 * @brief after a space update, this writes back to the body the new data and calls update functions
 * @param db the dynamic body that has been updated
 * @param factor the time set factor to update by
 */
void gf2d_dynamic_body_update(DynamicBody *db,float factor);

/**
 * @brief check if two dynamic bodies are overlapping at the current time step
 * @note the collision object returned must be freed
 * @param dba the first dynamic body, the one moving
 * @param dbb the other dynamic body, the one not moving
 * @param timeStep the time segment for the current update round
 * @return NULL on error or no collision, a pointer to the collision data otherwise
 */
Collision *gf2d_dynamic_body_collision_check(DynamicBody *dba,DynamicBody *dbb,float timeStep);

/**
 * @brief check if a dynamic body is clipping an arbitrary shape
 * @note the collision object returned must be freed
 * @param dba the first dynamic body, the one moving
 * @param shape the arbitrary shape to check against
 * @param timeStep the time segment for the current update round
 * @return NULL on error or no collision, a pointer to the collision data otherwise
 */
Collision *gf2d_dynamic_body_shape_collision_check(DynamicBody *dba,GFC_Shape shape,float timeStep);

/**
 * @brief check if a dynamic body is clipping the bounds of a space
 * @note the collision object returned must be freed
 * @param dba the first dynamic body, the one moving
 * @param bounds the outer boundary of the physics space
 * @param timeStep the time segment for the current update round
 * @return NULL on error or no collision, a pointer to the collision data otherwise
 */
Collision *gf2d_dynamic_body_bounds_collision_check(DynamicBody *dba,GFC_Rect bounds,float timeStep);

/**
 * @brief calculate a bounce gfc_vector for the given dynamic body based on the provided normal
 * @param dba the body to bounce
 * @param normal the direction of reflection
 * @return an empty gfc_vector on error or a new gfc_vector based on the bounce
 */
GFC_Vector2D gf2d_dynamic_body_bounce(DynamicBody *dba,GFC_Vector2D normal);

/**
 * @brief removes collisions from the dynamic body
 * @param db the dynamic body to clear
 */
void gf2d_dynamic_body_clear_collisions(DynamicBody *db);

/**
 * @brief attempt to back a dynamic body away from static shapes that it is overlapping
 * @note this relies on the collisioGFC_List being previously populated
 * @param db the dynamic body to adjust
 * @param backoff use this as a factor to move the body off any static shapes the body is clipping  too large will look wrong, too small will not work  0 becomes a no-op
 */
void gf2d_dynamic_body_resolve_overlap(DynamicBody *db,float backoff);

#endif
