#ifndef __SPACE_H__
#define __SPACE_H__

#include "gfc_shape.h"
#include "gf2d_body.h"

typedef struct
{
    GFC_Vector2D coordinate;            /**<where in the hash this bucket is*/
    GFC_List   *dynamicBodies;            /**<list of clipping dynamic bodies*/
    GFC_List   *staticShapes;             /**<list of clipping static shapes*/
}SpaceBucket;

typedef struct
{
    GFC_List       *dynamicBodyList;    /**<list of bodies in the space*/
    GFC_List       *staticShapes;       /**<list of shapes that will collide that do not move*/
    int             usesBuckets;        /**<if true, we will optimize with buckets*/
    GFC_List       *buckets;            /**<for spacial hash, a list of body lists*/
    SpaceBucket    *voidBucket;        /**<catch all bucket for if bodies exit the playable space*/
    GFC_Vector2D    bucketSize;         /**<how large the buckets are individually*/
    GFC_Vector2D    bucketCount;        /**<how many buckets per row,column*/
    int             precision;          /**<number of backoff attempts before giving up*/
    GFC_Rect        bounds;             /**<absolute bounds of the space*/
    float           timeStep;           /**<how much each iteration of the simulation progresses time by*/
    GFC_Vector2D    gravity;            /**<global gravity pull direction*/
    float           dampening;          /**<rate of movement degrade  ambient frictions*/
    float           slop;               /**<how much to correct for body overlap*/
    Uint32          idpool;
}Space;

/**
 * @brief create a new space
 * @return NULL on error or a new empty space
 */
Space *gf2d_space_new();

/**
 * @brief create a new space and set some defaults
 * @param precision number of retry attempts when movement collides
 * @param bounds the absolute bounds of the space
 * @param timeStep this should be fraction that can add up to 1.  ie: 0.1 or 0.01, etc
 * @param gravity the direction that gravity pulls
 * @param dampening the rate of all movemement decay
 * @param slop how much to correct for body overlap
 * @param useBuckets if true, use the spacial hash system
 * @param bucketSize how large the bucket should be (width and heigh)
 */
Space *gf2d_space_new_full(
    int         precision,
    GFC_Rect        bounds,
    float       timeStep,
    GFC_Vector2D    gravity,
    float       dampening,
    float       slop,
    int         useBuckets,
    GFC_Vector2D    bucketSize);

/**
 * @brief create a space based on specs from a config file
 */
Space *gf2d_space_load(SJson *json);

/**
 * @brief cleans up a space
 */
void gf2d_space_free(Space *space);

/**
 * @brief visualize the space and its contents
 * @param space the space to draw
 * @param offset a positional offset applied when drawing.
 */
void gf2d_space_draw(Space *space,GFC_Vector2D offset);

/**
 * @brief add a body to the space simulation
 * @param space the space to add a body to
 * @param body the body to add to the space
 * @note the space will not free the body, but do not until it has been removed from the space
 */
void gf2d_space_add_body(Space *space,Body *body);

/**
 * @brief removes a body from the space
 * @note this should not be done DURING a space update
 * @param space the space to remove the body from
 * @param body the body to remove
 */
void gf2d_space_remove_body(Space *space,Body *body);

/**
 * @brief add a statuc shape to the space
 * @note the shape parameters need to be in absolute space, not relative to any body
 * @param space the space to add to
 * @param shape the shape to add.
 */
void gf2d_space_add_static_shape(Space *space,GFC_Shape shape);

/**
 * @brief update the bodies in the physics space for one time slice
 * @param space the space to be updated
 */
void gf2d_space_update(Space *space);

/**
 * @brief attempt to fix any bodies that are overlapping static shapes or clipping the space bounds
 * @param space the space to update
 * @param tries the number of tries to get everything clear before giving up
 */
void gf2d_space_fix_overlaps(Space *space,Uint8 tries);

/**
 * @brief given the point, get the bucket coordnates that it falls into
 * @param space the space the query
 * @param point the spot to check
 * @return (-1,-1) if out of range, or error.  The bucket index otherwise
 */
GFC_Vector2D gf2d_space_bucket_coordinates_by_point(Space *space,GFC_Vector2D point);

/**
 * @brief get the corresponding bucket index given a position
 * @param space the space the query
 * @param point the spot to check
 * @return -1 if out of range, or error.  The bucket index otherwise
 */
int gf2d_space_bucket_index_get_by_point(Space *space,GFC_Vector2D point);

/**
 * @brief get the corresponding bucket given a position
 * @param space the space the query
 * @param point the spot to check
 * @return Null on error, the voidBucket if its out of range, The bucket otherwise
 */
SpaceBucket *gf2d_space_bucket_get_by_point(Space *space,GFC_Vector2D point);

/**
 * @brief check against the static shapes of a space to see if there any collisions
 * @param space the space to check
 * @param shape the shape to check with
 * @param collisionGFC_List the list of collisions to append to (if NULL, a new list is created)
 * @return NULL on missing space, or a list of collisions otherwise
 */
GFC_List *gf2d_space_static_shape_check(Space *space, GFC_Shape shape, GFC_List *collisionList);

/**
 * @brief iterate through an area of the spacial hash based on which buckets clip the given bounds
 * @param space the space to check
 * @param bounds the query rectangle (bounds of your collision shape)
 * @param last provide the return of the last call to this function.  Provide NULL to initiate a search
 * @return NULL when search is complete, or the next clipped bucket.
 */
SpaceBucket *gf2d_space_bucket_foreach_clipped(Space *space,GFC_Rect bounds,SpaceBucket *last);


#endif
