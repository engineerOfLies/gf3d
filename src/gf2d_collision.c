#include <stdlib.h>
#include "simple_logger.h"

#include "gf2d_dynamic_body.h"
#include "gf2d_draw.h"

#include "gf2d_collision.h"

Collision *gf2d_collision_new()
{
    Collision *collision = NULL;
    collision = (Collision *)malloc(sizeof(Collision));
    if (!collision)
    {
        slog("failed to allocate data for a collision object");
        return NULL;
    }
    memset(collision,0,sizeof(Collision));
    return collision;
}

void gf2d_collision_free(Collision *collision)
{
    if (!collision)return;
    free(collision);
}

void gf2d_collision_list_clear(GFC_List *list)
{
    int i, count;
    Collision *collision;
    if (!list)return;
    count = gfc_list_get_count(list);
    for (i = 0; i < count;i++)
    {
        collision = (Collision*)gfc_list_get_nth(list,i);
        if (!collision)continue;
        gf2d_collision_free(collision);
    }
    gfc_list_clear(list);
}

void gf2d_collision_list_free(GFC_List *list)
{
    int i, count;
    Collision *collision;
    if (!list)return;
    count = gfc_list_get_count(list);
    for (i = 0; i < count;i++)
    {
        collision = (Collision*)gfc_list_get_nth(list,i);
        if (!collision)continue;
        gf2d_collision_free(collision);
    }
    gfc_list_delete(list);
}

Collision *gf2d_collision_space_static_shape_clip(GFC_Shape a, GFC_Shape s)
{
    Collision *collision;
    GFC_Vector2D poc,normal;
    if (!gfc_shape_overlap_poc(a, s, &poc, &normal))
    {
        return NULL;
    }
    collision = gf2d_collision_new();
    collision->collided = 1;
    collision->blocked = 1;
    gfc_vector2d_copy(collision->pointOfContact,poc);
    gfc_vector2d_copy(collision->normal,normal);
    collision->shape = s;
    collision->body = NULL;
    collision->bounds = 0;
    collision->timeStep = 0; 
    return collision;
}

Collision *gf2d_collision_space_dynamic_body_clip(GFC_Shape a, DynamicBody *d)
{
    GFC_Shape s;
    Collision *collision;
    GFC_Vector2D poc,normal;
    if (!d)return NULL;
    s = gf2d_dynamic_body_to_shape(d);
    if (!gfc_shape_overlap_poc(a, s, &poc, &normal))
    {
        return NULL;
    }
    collision = gf2d_collision_new();
    collision->collided = 1;
    collision->blocked = 1;
    gfc_vector2d_copy(collision->pointOfContact,poc);
    gfc_vector2d_copy(collision->normal,normal);
    collision->shape = *d->body->shape;
    collision->body = d->body;
    collision->bounds = 0;
    collision->timeStep = 0; 
    return collision;
}

GFC_List *gf2d_collision_dynamic_body_check(Space *space, GFC_Shape shape, CollisionFilter filter, GFC_List *collisionList)
{
    int i,c;
    DynamicBody *db;
    SpaceBucket *bucket;
    Collision *collision;
    if (!space)return NULL;
    if (!collisionList)
    {
        collisionList = gfc_list_new();
    }
    if (space->usesBuckets)
    {
        bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(shape),NULL);
        while(bucket != NULL)
        {
            c = gfc_list_get_count(bucket->dynamicBodies);
            for (i = 0; i < c;i++)
            {
                db = (DynamicBody*)gfc_list_get_nth(bucket->dynamicBodies,i);
                if (!db)continue;
                if (!db->body)
                {
                    continue;
                }
                if (db->body == filter.ignore)continue;
                if (filter.team != 0)
                {
                    if (db->body->team == filter.team)continue;// skip same team
                }
                if (!(filter.cliplayer & db->body->cliplayer))continue;
                // check for layer compatibility
                collision = gf2d_collision_space_dynamic_body_clip(shape, db);
                if (collision == NULL)continue;
                gfc_list_append(collisionList,(void*)collision);
            }
            bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(shape),bucket);
        }
    }
    else
    {
        c = gfc_list_get_count(space->dynamicBodyList);
        for (i = 0; i < c;i++)
        {
            db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
            if (!db)continue;
            if (db->body == filter.ignore)continue;
            if (filter.team != 0)
            {
                if (db->body->team == filter.team)continue;// skip same team
            }
            if (!(filter.cliplayer & db->body->cliplayer))continue;
            // check for layer compatibility
            collision = gf2d_collision_space_dynamic_body_clip(shape, db);
            if (collision == NULL)continue;
            gfc_list_append(collisionList,(void*)collision);
        }
    }
    return collisionList;
}

GFC_List *gf2d_collision_check_space_shape(Space *space, GFC_Shape shape,CollisionFilter filter)
{
    GFC_List *collisionList = NULL;
    collisionList = gfc_list_new();
    if (filter.worldclip)
    {
        collisionList = gf2d_space_static_shape_check(space, shape, collisionList);
        //check if the shape clips the level bounds
/*        collision = gf2d_dynamic_body_bounds_collision_check(db,space->bounds,t);
        if (collision != NULL)
        {
            db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
        }*/
    }
    if (filter.cliplayer)
    {
        collisionList = gf2d_collision_dynamic_body_check(space, shape, filter, collisionList);
    }
    if (gfc_list_get_count(collisionList) == 0)
    {
        gfc_list_delete(collisionList);
        return NULL;
    }
    return collisionList;
}

Collision gf2d_collision_trace_space(Space *space, GFC_Vector2D start, GFC_Vector2D end ,CollisionFilter filter)
{
    Collision out = {0};
    Collision *collision = NULL;
    Collision *best = NULL;
    double     bestDistance = -1;
    double     distance;
    double     length;
    int count,i;
    GFC_List *collisionList;
    collisionList = gf2d_collision_check_space_shape(space, gfc_shape_from_edge(gfc_edge_from_vectors(start,end)),filter);
    if (!collisionList)
    {
        return out;
    }
    count = gfc_list_get_count(collisionList);
    for (i =0; i < count;i++)
    {
        collision = (Collision*)gfc_list_get_nth(collisionList,i);
        if (!collision)continue;
        if (!best)
        {
            best = collision;
            bestDistance = gfc_vector2d_magnitude_between(start,collision->pointOfContact);
            continue;
        }
        distance = gfc_vector2d_magnitude_between(start,collision->pointOfContact);
        if (distance < bestDistance)
        {
            best = collision;
            bestDistance = distance;
        }
    }
    if (best != NULL)
    {
        length = gfc_vector2d_magnitude_between(start,end);
        if (!length)
        {
            best->timeStep = 0;
        }
        else
        {
            best->timeStep = bestDistance / length;
        }
        memcpy(&out,best,sizeof(Collision));
    }
    gf2d_collision_list_free(collisionList);
    return out;
}


/*eol@eof*/
