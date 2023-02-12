#include <stdlib.h>
#include "simple_logger.h"

#include "gf2d_draw.h"
#include "gf2d_collision.h"
#include "gf2d_dynamic_body.h"

#include "gf2d_space.h"

extern int __DEBUG;

Uint8 gf2d_body_shape_collide(Body *a,Shape *s,Vector2D *poc, Vector2D *normal);
void gf2d_space_remove_body_from_buckets(Space *space, DynamicBody *db);

SpaceBucket *gf2d_space_bucket_new()
{
    SpaceBucket *bucket;
    bucket = gfc_allocate_array(sizeof(SpaceBucket),1);
    if (!bucket)return NULL;
    bucket->dynamicBodies = gfc_list_new();
    bucket->staticShapes = gfc_list_new();
    return bucket;
}

void gf2d_space_bucket_free(SpaceBucket *bucket)
{
    if (!bucket)return;
    // bucket don't own any of their data.  So just free the list
    gfc_list_delete(bucket->staticShapes);
    gfc_list_delete(bucket->dynamicBodies);
    free(bucket);
}

void gf2d_space_free(Space *space)
{
    SpaceBucket *bucket;
    int i,c;
    if (!space)return;
    
    //static shapes ARE owned by the space, so are deleted when the space goes away
    gfc_list_foreach(space->staticShapes,free);
    gfc_list_foreach(space->dynamicBodyList,(gfc_work_func*)gf2d_dynamic_body_free);
    gfc_list_delete(space->staticShapes);
    gf2d_space_bucket_free(space->voidBucket);
    c = gfc_list_get_count(space->buckets);
    for (i = 0; i < c; i++)
    {
        bucket = gfc_list_get_nth(space->buckets,i);
        if (!bucket)continue;
        gf2d_space_bucket_free(bucket);
    }
    gfc_list_delete(space->buckets);
    free(space);
}

void gf2d_space_buckets_clear(Space *space)
{
    SpaceBucket *bucket;
    int i,c;
    if ((!space)||(!space->usesBuckets))return;
    c = gfc_list_get_count(space->buckets);
    for (i = 0; i < c; i++)
    {
        bucket = gfc_list_get_nth(space->buckets,i);
        if (!bucket)continue;
        gfc_list_clear(bucket->dynamicBodies);
        gfc_list_clear(bucket->staticShapes);
    }
}

Vector2D gf2d_space_bucket_coordinates_by_point(Space *space,Vector2D point)
{
    Vector2D coordinate = {-1,-1};
    Vector2D spacePoint;
    if ((!space)||(!space->usesBuckets))return coordinate;
    if ((!space->bucketSize.x)||(!space->bucketSize.y))
    {
        slog("ERROR: zero dimention in space bucket size");
        return coordinate;
    }
    vector2d_sub(spacePoint,point,space->bounds);// account for an offset from the level bounds...
    coordinate.x = (int) floor(spacePoint.x/space->bucketSize.x);
    coordinate.y = (int) floor(spacePoint.y/space->bucketSize.y);
    return coordinate;
}

int gf2d_space_bucket_coordinate_to_index(Space *space,Vector2D coordinate)
{
    if (!space)return -1;
    if ((coordinate.x < 0)||(coordinate.y < 0)
       ||(coordinate.x >= space->bucketCount.x)||(coordinate.y >= space->bucketCount.y))return -1;
    return (coordinate.y * space->bucketCount.x)+coordinate.x;
}

int gf2d_space_bucket_index_get_by_point(Space *space,Vector2D point)
{
    Vector2D coordinate;
    coordinate = gf2d_space_bucket_coordinates_by_point(space,point);
    if (coordinate.x == -1)return -1;
    return gf2d_space_bucket_coordinate_to_index(space,coordinate);
}

SpaceBucket *gf2d_space_bucket_get_by_coordinate(Space *space,Vector2D coordinate)
{
    int index;
    if (!space)return NULL;
    index = gf2d_space_bucket_coordinate_to_index(space,coordinate);
    if (index == -1)return space->voidBucket;
    return gfc_list_get_nth(space->buckets,index);
}

SpaceBucket *gf2d_space_bucket_get_by_point(Space *space,Vector2D point)
{
    int index;
    index = gf2d_space_bucket_index_get_by_point(space,point);
    if (index == -1)return space->voidBucket;
    return gfc_list_get_nth(space->buckets,index);
}

SpaceBucket *gf2d_space_bucket_foreach_clipped(Space *space,Rect bounds,SpaceBucket *last)
{
    int i,j;
    Vector2D start,end,begin;
    SpaceBucket *next = NULL;
    if (!space)return NULL;
    start = gf2d_space_bucket_coordinates_by_point(space,vector2d(bounds.x,bounds.y));
    end = gf2d_space_bucket_coordinates_by_point(space,vector2d(bounds.x+bounds.w,bounds.y+bounds.h));
    if (!last)
    {
        next = gf2d_space_bucket_get_by_point(space,vector2d(bounds.x,bounds.y));
        if (!next)return NULL;
        if (next == space->voidBucket)
        {
            //not checking voidBucket here, so find first NON void bucket
            for (j = start.y;j <= end.y;j++)
            {
                for (i = start.x;i <= end.x;i++)
                {
                    next = gf2d_space_bucket_get_by_coordinate(space,vector2d(i,j));
                    if (next != space->voidBucket)
                    {
                        return next;
                    }
                }
            }
            return NULL;// never hit anything valid for this coordinate
        }
        return next;
    }
    vector2d_copy(begin,last->coordinate);
    i = begin.x + 1;
    for (j = begin.y;j <= end.y;j++)
    {
        for (;i <= end.x;i++)
        {
            next = gf2d_space_bucket_get_by_coordinate(space,vector2d(i,j));
            if (next == last)continue;
            if (next != space->voidBucket)
            {
                return next;
            }
        }
        i = start.x;
    }
    return NULL;
}

void gf2d_space_bucket_add_dynamic_body(SpaceBucket *bucket,DynamicBody *db)
{
    int index;
    if ((!bucket)||(!db))return;
    index = gfc_list_get_item_index(bucket->dynamicBodies,db);
    if (index != -1)
    {
        slog("static shape already in the bucket");
        return;
    }
    bucket->dynamicBodies = gfc_list_append(bucket->dynamicBodies,db);
    index = gfc_list_get_item_index(db->bucketList,bucket);
    if (index == -1)//not already in the bucket
    {
        db->bucketList = gfc_list_append(db->bucketList,bucket);
    }
}

void gf2d_space_bucket_add_shape(SpaceBucket *bucket, Shape *shape)
{
    int index;
    if ((!bucket)||(!shape))return;
    index = gfc_list_get_item_index(bucket->staticShapes,shape);
    if (index != -1)
    {
//        slog("static shape already in the bucket");
//        gfc_shape_slog(*shape);
        return;
    }
    bucket->staticShapes = gfc_list_append(bucket->staticShapes,shape);
}

void gf2d_space_bucket_index_remove_shape(SpaceBucket *bucket, Shape *shape)
{
    int index;
    if ((!bucket)||(!shape))return;
    index = gfc_list_get_item_index(bucket->staticShapes,shape);
    if (index != -1)
    {
        //already here
        return;
    }
    gfc_list_delete_data(bucket->staticShapes,shape);
}

void gf2d_space_buckets_remove_shape(Space *space,Shape *shape)
{
    Rect bounds;
    Vector2D start,end;
    int i,j;
    if ((!space)||(!shape))return;
    bounds = gfc_shape_get_bounds(*shape);
    start = gf2d_space_bucket_coordinates_by_point(space,vector2d(bounds.x,bounds.y));
    end = gf2d_space_bucket_coordinates_by_point(space,vector2d(bounds.x+bounds.w,bounds.y+bounds.h));
    for (j = start.y;j <= end.y;j++)
    {
        for (i = start.x;i < end.x;i++)
        {
            gf2d_space_bucket_index_remove_shape(
                gf2d_space_bucket_get_by_point(space,vector2d(i,j)),
                shape);
        }
    }
}

List *gf2d_space_static_shape_check(Space *space, Shape shape, List *collisionList)
{
    int i,c;
    Collision *collision;
    SpaceBucket *bucket;
    Shape *staticShape;
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
            c = gfc_list_get_count(bucket->staticShapes);
            for (i = 0; i < c;i++)
            {
                staticShape = (Shape*)gfc_list_get_nth(bucket->staticShapes,i);
                if (!staticShape)continue;
                // check for layer compatibility
                collision = gf2d_collision_space_static_shape_clip(shape, *staticShape);
                if (collision == NULL)continue;
                collisionList = gfc_list_append(collisionList,(void*)collision);
            }
            bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(shape),bucket);
        }
    }
    else
    {
        c = gfc_list_get_count(space->staticShapes);
        for (i = 0; i < c;i++)
        {
            staticShape = (Shape*)gfc_list_get_nth(space->staticShapes,i);
            if (!staticShape)continue;
            // check for layer compatibility
            collision = gf2d_collision_space_static_shape_clip(shape, *staticShape);
            if (collision == NULL)continue;
            collisionList = gfc_list_append(collisionList,(void*)collision);
        }
    }
    return collisionList;
}

void gf2d_space_buckets_add_shape(Space *space,Shape *shape)
{
    Rect bounds;
    Vector2D start,end;
    int i,j;
    if ((!space)||(!shape))return;
    bounds = gfc_shape_get_bounds(*shape);
    start = gf2d_space_bucket_coordinates_by_point(space,vector2d(bounds.x,bounds.y));
    end = gf2d_space_bucket_coordinates_by_point(space,vector2d(bounds.x+bounds.w,bounds.y+bounds.h));
    for (j = start.y;j <= end.y;j++)
    {
        for (i = start.x;i <= end.x;i++)
        {
            gf2d_space_bucket_add_shape(
                gf2d_space_bucket_get_by_coordinate(space,vector2d(i,j)),
                shape);
        }
    }
}

Space *gf2d_space_new_full(
    int         precision,
    Rect        bounds,
    float       timeStep,
    Vector2D    gravity,
    float       dampening,
    float       slop,
    int         useBuckets,
    Vector2D    bucketSize)
{
    Space *space;
    SpaceBucket *bucket;
    int i;
    space = gf2d_space_new();
    if (!space)return NULL;
    gfc_rect_copy(space->bounds,bounds);
    vector2d_copy(space->gravity,gravity);
    space->timeStep = timeStep;
    space->precision = precision;
    space->dampening = dampening;
    space->slop = slop;
    space->usesBuckets = useBuckets;
    vector2d_copy(space->bucketSize,bucketSize);
    if ((useBuckets)&&(bucketSize.x)&&(bucketSize.y))
    {
        space->voidBucket = gf2d_space_bucket_new();
        if (space->voidBucket)space->voidBucket->coordinate.x = space->voidBucket->coordinate.y = -1; 
        space->buckets = gfc_list_new();
        space->bucketCount.x = ceil(bounds.w / bucketSize.x);
        space->bucketCount.y = ceil(bounds.h / bucketSize.y);
        if (__DEBUG)
        {
            slog("space has %i buckets per line, and %i lines",(int)space->bucketCount.x,(int)space->bucketCount.y);
        }
        for (i= 0; i < space->bucketCount.y*space->bucketCount.x;i++)
        {
            bucket = gf2d_space_bucket_new();
            if (!bucket)continue;
            bucket->coordinate.x = i % (int)space->bucketCount.x;
            bucket->coordinate.y = i / (int)space->bucketCount.x;
            space->buckets = gfc_list_append(space->buckets,bucket);
        }
    }
    return space;
}

Space *gf2d_space_new()
{
    Space *space;
    space = (Space *)malloc(sizeof(Space));
    if (!space)
    {
        slog("failed to allocate space for Space");
        return NULL;
    }
    memset(space,0,sizeof(Space));
    space->dynamicBodyList = gfc_list_new();
    space->staticShapes = gfc_list_new();
    return space;
}

void gf2d_space_add_static_shape(Space *space,Shape shape)
{
    Shape *newShape;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    newShape = (Shape*)malloc(sizeof(shape));
    if (!newShape)
    {
        slog("failed to allocate new space for the shape");
        return;
    }
    memcpy(newShape,&shape,sizeof(Shape));
    space->staticShapes = gfc_list_append(space->staticShapes,(void *)newShape);
    if (space->usesBuckets)gf2d_space_buckets_add_shape(space,newShape);
}

void gf2d_space_remove_body(Space *space,Body *body)
{
    int i,count;
    DynamicBody *db = NULL;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    if (!body)
    {
        slog("no body provided");
        return;
    }
    if (space->dynamicBodyList)
    {
        count = gfc_list_get_count(space->dynamicBodyList);
        for (i = 0; i < count;i++)
        {
            db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
            if (!db)continue;
            if (db->body != body)continue;
            if (space->usesBuckets)
            {
                gf2d_space_remove_body_from_buckets(space, db);
            }
            gf2d_dynamic_body_free(db);
            gfc_list_delete_nth(space->dynamicBodyList,i);
            break;
        }
    }
}

void gf2d_space_remove_body_from_buckets(Space *space, DynamicBody *db)
{
    int i,c;
    SpaceBucket *bucket;
    if ((!space)||(!db))return;
    c = gfc_list_get_count(db->bucketList);
    for (i =0;i < c; i++)
    {
        bucket = gfc_list_get_nth(db->bucketList,i);
        if (!bucket)continue;
        gfc_list_delete_data(bucket->dynamicBodies,db);
    }
    gfc_list_clear(db->bucketList);
}

void gf2d_space_add_body_to_buckets(Space *space,DynamicBody *db)
{
    Shape bodyShape;
    SpaceBucket *bucket;
    if ((!space)||(!db)||(!db->body)||(!db->body->shape))return;
    bodyShape = gf2d_dynamic_body_to_shape(db);
    bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(bodyShape),NULL);
    while(bucket != NULL)
    {
        //for each clipping bucket
        gf2d_space_bucket_add_dynamic_body(bucket,db);
        bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(bodyShape),bucket);
    }
}

void gf2d_space_add_body(Space *space,Body *body)
{
    DynamicBody *db = NULL;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    if (!body)
    {
        slog("no body provided");
        return;
    }
    db = gf2d_dynamic_body_new();
    if (!db)return;
    db->body = body;
    db->id = space->idpool++;
    space->dynamicBodyList = gfc_list_append(space->dynamicBodyList,(void *)db);
    if (space->usesBuckets)gf2d_space_add_body_to_buckets(space,db);
}

void gf2d_bucket_draw(Space *space,SpaceBucket *bucket, Vector2D offset)
{
    DynamicBody *db = NULL;
    int i,count;
    if ((!space)||(!bucket))return;
    count = gfc_list_get_count(bucket->staticShapes);
    for (i = 0; i < count;i++)
    {
        gf2d_draw_shape(*(Shape *)gfc_list_get_nth(bucket->staticShapes,i),gfc_color8(0,255,0,255),offset);
    }
    gf2d_draw_shape(
        gfc_shape_rect(
            bucket->coordinate.x * space->bucketSize.x,
            bucket->coordinate.y * space->bucketSize.y,
            space->bucketSize.x,
            space->bucketSize.y),
        gfc_color8(0,255,128,255),
        offset);
    count = gfc_list_get_count(bucket->dynamicBodies);
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(bucket->dynamicBodies,i);
        if (!db)continue;
        gf2d_body_draw(db->body,offset);
    }

}

void gf2d_space_draw(Space *space,Vector2D offset)
{
    int i,count;
    Rect r;
    SpaceBucket *bucket;
    DynamicBody *db = NULL;
    if (!space)
    {
        slog("no space provided");
        return;
    }
    r = space->bounds;
    vector2d_add(r,r,offset);    
    gf2d_draw_rect(r,gfc_color8(255,0,0,255));
    count = gfc_list_get_count(space->dynamicBodyList);
    if (space->usesBuckets)
    {
        count = gfc_list_get_count(space->buckets);
        for (i = 0; i < count;i++)
        {
            bucket = gfc_list_get_nth(space->buckets,i);
            if (!bucket)continue;
            gf2d_bucket_draw(space,bucket, offset);
        }
    }
    else
    {
        for (i = 0; i < count;i++)
        {
            db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
            if (!db)continue;
            gf2d_body_draw(db->body,offset);
        }
        count = gfc_list_get_count(space->staticShapes);
        for (i = 0; i < count;i++)
        {
            gf2d_draw_shape(*(Shape *)gfc_list_get_nth(space->staticShapes,i),gfc_color8(0,255,0,255),offset);
        }
    }
}
void gf2d_space_dynamic_bodies_world_clip(Space *space,DynamicBody *db, float t)
{
    int i,c;
    Shape *shape;
    Shape bodyShape;
    SpaceBucket *bucket = NULL;
    Collision *collision;
    if ((!space)||(!db)||(!db->body))return;
    if (!db->body->worldclip)return;
    if (space->usesBuckets)
    {
        bodyShape = gf2d_dynamic_body_to_shape(db);
        bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(bodyShape),NULL);
        while(bucket != NULL)
        {
            c = gfc_list_get_count(bucket->staticShapes);
            for (i = 0; i < c;i++)
            {
                shape = (Shape*)gfc_list_get_nth(bucket->staticShapes,i);
                if (!shape)continue;
                // check for layer compatibility
                collision = gf2d_dynamic_body_shape_collision_check(db,*shape,t);
                if (collision == NULL)continue;
                db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
            }
            bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(bodyShape),bucket);
        }
    }
    else
    {
        c = gfc_list_get_count(space->staticShapes);
        for (i = 0; i < c;i++)
        {
            shape = (Shape*)gfc_list_get_nth(space->staticShapes,i);
            if (!shape)continue;
            // check for layer compatibility
            collision = gf2d_dynamic_body_shape_collision_check(db,*shape,t);
            if (collision == NULL)continue;
            db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
        }
    }
    //check if the dynamic body is leaving the level bounds
    collision = gf2d_dynamic_body_bounds_collision_check(db,space->bounds,t);
    if (collision != NULL)
    {
        db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
    }
}

void gf2d_space_dynamic_bodies_body_step(Space *space,DynamicBody *db,float t)
{
    Shape bodyShape;
    DynamicBody *other;
    SpaceBucket *bucket;
    Collision *collision;
    int i,c;
    if (space->usesBuckets)
    {
        bodyShape = gf2d_dynamic_body_to_shape(db);
        bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(bodyShape),NULL);
        while(bucket != NULL)
        {
            c = gfc_list_get_count(bucket->dynamicBodies);
            for (i = 0; i < c;i++)
            {
                other = (DynamicBody*)gfc_list_get_nth(bucket->dynamicBodies,i);
                if (!other)continue;
                if (other == db)continue;   // skip checking outself
                // check for layer compatibility
                collision = gf2d_dynamic_body_collision_check(db,other,t);
                if (collision == NULL)continue;
                db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
            }
            bucket = gf2d_space_bucket_foreach_clipped(space,gfc_shape_get_bounds(bodyShape),bucket);
        }
    }
    else
    {
        c = gfc_list_get_count(space->dynamicBodyList);
        for (i = 0; i < c;i++)
        {
            other = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
            if (!other)continue;
            if (other == db)continue;   // skip checking outself
            // check for layer compatibility
            collision = gf2d_dynamic_body_collision_check(db,other,t);
            if (collision == NULL)continue;
            db->collisionList = gfc_list_append(db->collisionList,(void*)collision);
        }
    }
}

void gf2d_space_dynamic_bodies_step(Space *space,DynamicBody *db, float t)
{
    Collision *collision;
    Vector2D reflected,total;
    int normalCount;
    int i,count;
    if ((!space)||(!db))return;
    if ((!db->velocity.x)&&(!db->velocity.y))return;// skip entities that are not moving
    // save our place in case of collision
    vector2d_copy(db->oldPosition,db->position);
    vector2d_add(db->position,db->position,db->velocity);
    
    gf2d_dynamic_body_clear_collisions(db);    
    // check against dynamic bodies
    
    gf2d_space_dynamic_bodies_body_step(space,db,t);

    gf2d_space_dynamic_bodies_world_clip(space,db, t);
    if (db->blocked)
    {
        db->blocked = 0;
        gf2d_dynamic_body_resolve_overlap(db,space->slop);
        if (db->body->elasticity > 0)
        {
            count = gfc_list_get_count(db->collisionList);
            vector2d_clear(total);
            normalCount = 0;
            for (i = 0; i < count; i++)
            {
                collision = (Collision*)gfc_list_get_nth(db->collisionList,i);
                if (!collision)continue;
                vector2d_add(db->position,db->position,collision->normal);
                reflected = gf2d_dynamic_body_bounce(db,collision->normal);
                if (vector2d_magnitude_squared(reflected) != 0)
                {
                    vector2d_add(total,total,reflected);
                    normalCount++;
                }
            }
            if (normalCount)
            {
                vector2d_scale(total,total,(1.0/normalCount)*space->slop);
                db->velocity = total;
                vector2d_set_magnitude(&db->velocity,db->speed);
            }
        }
    }
}

void gf2d_space_buckets_update_dynamic_bodies(Space *space)
{
    DynamicBody *db;
    int i,c;
    c = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < c;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        //if ((db->position.x == db->body->position.x)&&(db->position.y == db->body->position.y))continue;
        //skip anything that hasn't moved
        gf2d_space_remove_body_from_buckets(space, db);
        gf2d_space_add_body_to_buckets(space,db);
    }
}

void gf2d_space_step(Space *space,float t)
{
    DynamicBody *db = NULL;
    int i,count;
    if (!space)return;
    if (space->usesBuckets)
    {
        gf2d_space_buckets_update_dynamic_bodies(space);
    }
    count = gfc_list_get_count(space->dynamicBodyList);
    
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        gf2d_space_dynamic_bodies_step(space,db, t);
    }
}

void gf2d_space_reset_bodies(Space *space)
{
    int i,count;
    if (!space)return;
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        gf2d_dynamic_body_reset((DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i),space->timeStep);
    }
}

void gf2d_space_update_bodies(Space *space,float loops)
{
    DynamicBody *db = NULL;
    int i,count;
    if (!space)return;
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        gf2d_dynamic_body_update(db,loops);
    }
}

void gf2d_space_update(Space *space)
{
    float s;
    float loops = 0;
    Uint32 then;
    if (!space)return;
    gf2d_space_fix_overlaps(space,8);
    gf2d_space_reset_bodies(space);
    // reset all body tracking
    
    if (__DEBUG)then = SDL_GetTicks();
    for (s = 0; s <= 1; s += space->timeStep)
    {
        gf2d_space_step(space,s);
        loops = loops + 1;
    }
    if (__DEBUG)slog("collision took %i milliseconds",SDL_GetTicks() - then);
    gf2d_space_update_bodies(space,loops);
}

Uint8 gf2d_space_resolve_overlap(Space *space)
{
    DynamicBody *db = NULL;
    int i,count;
    int clipped = 0;
    if (!space)return 1;
    gf2d_space_reset_bodies(space);
    // for each dynamic body, get list of staic shapes that are clipped
    count = gfc_list_get_count(space->dynamicBodyList);
    for (i = 0; i < count;i++)
    {
        db = (DynamicBody*)gfc_list_get_nth(space->dynamicBodyList,i);
        if (!db)continue;
        gf2d_space_dynamic_bodies_world_clip(space,db, 0);
        if (gfc_list_get_count(db->collisionList))
        {
            gf2d_dynamic_body_resolve_overlap(db,space->slop);
        }
    }
    return clipped;
}

void gf2d_space_fix_overlaps(Space *space,Uint8 tries)
{
    int i = 0;
    int done = 0;
    for (i = 0; (i < tries) & (done != 1);i++)
    {
        done = gf2d_space_resolve_overlap(space);
    }
}

/*eol@eof*/
