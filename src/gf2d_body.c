#include <stdlib.h>
#include "simple_logger.h"

#include "gfc_config.h"

#include "gf3d_cliplayers.h"

#include "gf2d_draw.h"
#include "gf2d_body.h"


Uint8 gf2d_body_shape_collide(Body *a,GFC_Shape *s,GFC_Vector2D *poc, GFC_Vector2D *normal);

void gf2d_body_clear(Body *body)
{
    if (!body)return;
    memset(body,0,sizeof(Body));
}

void gf2d_body_push(Body *body,GFC_Vector2D direction,float force)
{
    if (!body)return;
    if (body->mass != 0)
    {
        force = force / body->mass;
    }
    gfc_vector2d_set_magnitude(&direction,force);
    gfc_vector2d_add(body->velocity,body->velocity,direction);
}

void gf2d_body_draw(Body *body,GFC_Vector2D offset)
{
    GFC_Color color;
    GFC_Shape shape;
    GFC_Vector2D center;
    if (!body)return;
    color = gfc_color8(0,255,255,255);
    // draw center point
    gfc_vector2d_add(center,body->position,offset);
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
    GFC_Vector2D    position,
    GFC_Vector2D    velocity,
    float       mass,
    float       gravity,
    float       elasticity,
    GFC_Shape      *shape,
    void       *data,
    int         (*touch)(struct Body_S *self, GFC_List *collision))
{
    if (!body)return;
    body->cliplayer = cliplayer;
    body->touchlayer = touchlayer;
    body->team = team;
    body->worldclip = worldclip;
    gfc_vector2d_copy(body->position,position);
    gfc_vector2d_copy(body->velocity,velocity);
    body->mass = mass;
    body->gravity = gravity;
    body->elasticity = elasticity;
    body->shape = shape;
    body->data = data;
    body->touch = touch;
    gfc_word_cpy(body->name,name);
}

GFC_Shape gf2d_body_to_shape(Body *a)
{
    GFC_Shape aS = {0};
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

void gf2d_body_from_config(Body *body, SJson *config)
{
    const char *str;    
    if ((!body)||(!config)) return;
    str = sj_object_get_value_as_string(config,"name");
    if (str)gfc_line_cpy(body->name,str);
    sj_object_get_value_as_float(config,"gravity",&body->gravity);
    sj_object_get_value_as_uint8(config,"worldclip",&body->worldclip);
    sj_object_get_value_as_uint32(config,"team",&body->team);
    sj_value_as_vector2d(sj_object_get_value(config,"position"),&body->position);
    sj_value_as_vector2d(sj_object_get_value(config,"velocity"),&body->velocity);
    sj_object_get_value_as_float(config,"mass",&body->mass);
    sj_object_get_value_as_float(config,"elasticity",&body->elasticity);
    body->cliplayer = gf3d_cliplayers_from_config(sj_object_get_value(config,"cliplayer"));
    body->touchlayer = gf3d_cliplayers_from_config(sj_object_get_value(config,"touchlayer"));
}

/*eol@eof*/
