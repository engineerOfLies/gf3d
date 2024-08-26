#include <stdlib.h>
#include "simple_logger.h"
#include "gf2d_collision.h"
#include "gf2d_dynamic_body.h"

GFC_Shape gf2d_dynamic_body_to_shape(DynamicBody *a)
{
    GFC_Shape aS = {0};
    if ((!a)||(!a->body)||(!a->body->shape))return aS;
    gfc_shape_copy(&aS,*a->body->shape);
    gfc_shape_move(&aS,a->position);
    return aS;
}

GFC_Vector2D gf2d_dynamic_body_bounce(DynamicBody *dba,GFC_Vector2D normal)
{
    GFC_Vector2D nv = {0};
    gfc_vector2d_reflect(&nv, normal,dba->velocity);
    return nv;
}

Collision *gf2d_dynamic_body_bounds_collision_check(DynamicBody *dba,GFC_Rect bounds,float timeStep)
{
    Collision *collision = NULL;
    GFC_Rect dbBounds;
    if (!dba)return NULL;
    if (!dba->body)
    {
        slog("cannot do collision check, body missing from DynamicBody");
        return NULL;
    }
    dbBounds = gfc_shape_get_bounds(gf2d_dynamic_body_to_shape(dba));
    if ((dbBounds.x > bounds.x)&&(dbBounds.x + dbBounds.w < bounds.x + bounds.w)&&
        (dbBounds.y > bounds.y)&&(dbBounds.y + dbBounds.h < bounds.y + bounds.h))
    {
        // No collision with the level bounds
        return NULL;
    }
    collision = gf2d_collision_new();
    if (!collision)return NULL;
    collision->body = NULL;
    collision->timeStep = timeStep;
    //TODO: collision->pointOfContact;
    if (dbBounds.x <= bounds.x)collision->normal.x = 1;
    if (dbBounds.y <= bounds.y)collision->normal.y = 1;
    if (dbBounds.x + dbBounds.w >= bounds.x + bounds.w)collision->normal.x = -1;
    if (dbBounds.y + dbBounds.h >= bounds.y + bounds.h)collision->normal.y = -1;
    gfc_vector2d_normalize(&collision->normal);
    memset(&collision->shape,0,sizeof(GFC_Shape));
    collision->bounds = 1;
    dba->blocked = 1;
    return collision;
}

Collision *gf2d_dynamic_body_shape_collision_check(DynamicBody *dba,GFC_Shape shape,float timeStep)
{
    Collision *collision = NULL;
    if (!dba)return NULL;
    if (!dba->body)
    {
        slog("cannot do collision check, body or shape shape missing");
        return NULL;
    }
    if (!gfc_shape_overlap(gf2d_dynamic_body_to_shape(dba),shape))
    {
        return NULL;
    }
    collision = gf2d_collision_new();
    if (!collision)return NULL;
    collision->body = NULL;
    collision->timeStep = timeStep;
    //TODO: collision->pointOfContact;
    collision->normal = gfc_shape_get_normal_for_shape(shape, gf2d_dynamic_body_to_shape(dba));
    collision->shape = shape;
    dba->blocked = 1;
    return collision;
}

Collision *gf2d_dynamic_body_collision_check(DynamicBody *dba,DynamicBody *dbb,float timeStep)
{
    Collision *collision = NULL;
    if ((!dba)||(!dbb))return NULL;
    if ((!dba->body)||(!dbb->body))
    {
        slog("cannot do collision check, body missing from one or more DynamicBody");
        return NULL;
    }
    if ((dba->body->ignore)&&(dba->body->ignore == dbb->body))return NULL;
    if ((dbb->body->ignore)&&(dbb->body->ignore == dba->body))return NULL;
    if (dbb->body->cliplayer & PLATFORM_LAYER)
    {
        if (dba->body->velocity.y < 0)return NULL;// platforms will only be checked for from game
        if (gfc_shape_overlap(gf2d_body_to_shape(dba->body),gf2d_body_to_shape(dbb->body)))
        {
            // if we were already overlapping, skip it
            return NULL;
        }
    }
    if ((dba->body->team)&&(dbb->body->team)&&(dba->body->team == dbb->body->team))
    {
        return NULL;
    }
    if (!dba->body->cliplayer)
    {
        return NULL;
    }
    if (!gfc_shape_overlap(gf2d_dynamic_body_to_shape(dba),gf2d_dynamic_body_to_shape(dbb)))
    {
        return NULL;
    }
    collision = gf2d_collision_new();
    if (!collision)return NULL;
    collision->body = dbb->body;
    collision->timeStep = timeStep;
    //TODO: collision->pointOfContact;
    collision->normal = gfc_shape_get_normal_for_shape(gf2d_dynamic_body_to_shape(dbb),gf2d_dynamic_body_to_shape(dba));
    collision->shape = dbb->shape;
    if (dba->body->cliplayer & dbb->body->cliplayer)
    {
        dba->blocked = 1;
    }
    return collision;
}

DynamicBody *gf2d_dynamic_body_new()
{
    DynamicBody *db;
    db = (DynamicBody *)malloc(sizeof(DynamicBody));
    if (!db)
    {
        slog("failed to allocation data for a new dynamic body");
        return NULL;
    }
    memset(db,0,sizeof(DynamicBody));
    db->collisionList = gfc_list_new();
    db->bucketList = gfc_list_new();
    return db;
}

void gf2d_dynamic_body_clear_collisions(DynamicBody *db)
{
    if (!db)return;
    gf2d_collision_list_clear(db->collisionList);
}

void gf2d_dynamic_body_free(DynamicBody *db)
{
    if (!db)return;
    //cleanup collionGFC_List
    gf2d_collision_list_free(db->collisionList);
    gfc_list_delete(db->bucketList);
    free(db);
}

void gf2d_dynamic_body_update(DynamicBody *db,float factor)
{
    if (!db)return;
    if (!db->body)return;
    if ((db->body->position.x != db->position.x)||(db->body->position.y != db->position.y))
    {
        gfc_vector2d_copy(db->body->position,db->position);
        //BUCKETS
        db->shape = *db->body->shape;
        gfc_shape_move(&db->shape,db->position);
    }
    gfc_vector2d_scale(db->body->velocity,db->velocity,factor);
}

void gf2d_dynamic_body_reset(DynamicBody *db,float factor)
{
    if (!db)return;
    db->blocked = 0;
    gf2d_dynamic_body_clear_collisions(db);
    // may need to redo buckets if the positions don't match
    gfc_vector2d_copy(db->position,db->body->position);
    gfc_vector2d_scale(db->velocity,db->body->velocity,factor);
    db->speed = gfc_vector2d_magnitude(db->velocity);
}

void gf2d_dynamic_body_resolve_overlap(DynamicBody *db,float backoff)
{
    int i,count;
    GFC_Rect r;
    Collision *collision;
    GFC_Vector2D adjust = {0};
    if (!db)return;
    count = gfc_list_get_count(db->collisionList);
    for (i = 0; i < count; i++)
    {
        collision = (Collision*)gfc_list_get_nth(db->collisionList,i);
        if (!collision)continue;
        if (!gfc_shape_overlap(gf2d_dynamic_body_to_shape(db),collision->shape))continue;
        r = gfc_shape_get_bounds(collision->shape);
        if (db->position.x > (r.x + r.w))adjust.x += 1;
        else if (db->position.x < r.x)adjust.x -= 1;
        if (db->position.y > (r.y + r.h))adjust.y += 1;
        else if (db->position.y < r.y)adjust.y -= 1;
    }
    if (adjust.x < 0)db->position.x -= backoff;
    if (adjust.x > 0)db->position.x += backoff;
    if (adjust.y < 0)db->position.y -= backoff;
    if (adjust.y > 0)db->position.y += backoff;
}

/*eol@eof*/
