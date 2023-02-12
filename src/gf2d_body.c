#include <stdlib.h>
#include "simple_logger.h"

#include "gf2d_draw.h"
#include "gf2d_body.h"


Uint8 gf2d_body_shape_collide(Body *a,Shape *s,Vector2D *poc, Vector2D *normal);

void gf2d_body_clear(Body *body)
{
    if (!body)return;
    memset(body,0,sizeof(Body));
}

void gf2d_body_push(Body *body,Vector2D direction,float force)
{
    if (!body)return;
    if (body->mass != 0)
    {
        force = force / body->mass;
    }
    vector2d_set_magnitude(&direction,force);
    vector2d_add(body->velocity,body->velocity,direction);
}

void gf2d_body_draw(Body *body,Vector2D offset)
{
    Color color;
    Shape shape;
    Vector2D center;
    if (!body)return;
    color = gfc_color8(0,255,255,255);
    // draw center point
    vector2d_add(center,body->position,offset);
    gf2d_draw_circle(center,2,color);
        
    color = gfc_color8(255,0,255,255);
    gfc_shape_copy(&shape,*body->shape);
    gfc_shape_move(&shape,body->position);
    gf2d_draw_shape(shape,color,offset);
}

void gf2d_body_set(
    Body       *body,
    const char *name,
    Uint8       worldclip,
    Uint32      cliplayer,
    Uint32      touchlayer,
    Uint32      team,
    Vector2D    position,
    Vector2D    velocity,
    float       mass,
    float       gravity,
    float       elasticity,
    Shape      *shape,
    void       *data,
    int         (*touch)(struct Body_S *self, List *collision))
{
    if (!body)return;
    body->cliplayer = cliplayer;
    body->touchlayer = touchlayer;
    body->team = team;
    body->worldclip = worldclip;
    vector2d_copy(body->position,position);
    vector2d_copy(body->velocity,velocity);
    body->mass = mass;
    body->gravity = gravity;
    body->elasticity = elasticity;
    body->shape = shape;
    body->data = data;
    body->touch = touch;
    gfc_word_cpy(body->name,name);
}

Shape gf2d_body_to_shape(Body *a)
{
    Shape aS = {0};
    if (!a)return aS;
    gfc_shape_copy(&aS,*a->shape);
    gfc_shape_move(&aS,a->position);
    return aS;
}

Uint8 gf2d_body_body_collide(Body *a,Body *b)
{
    if ((!a)||(!b))
    {
        slog("missing body in collision check");
        return 0;
    }
    return gfc_shape_overlap(gf2d_body_to_shape(a),gf2d_body_to_shape(b));
}

/*eol@eof*/
