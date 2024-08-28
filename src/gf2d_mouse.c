#include <SDL.h>

#include "simple_logger.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"

#include "gf2d_actor.h"
#include "gf2d_mouse.h"


typedef struct
{
    Uint32 buttons;         /**<buttons mask*/
    GFC_Vector2D position;  /**<position of mouse*/
}MouseState;

typedef struct
{
    MouseState  mouse[2];   /**<mouse state for the current and last frames*/
    Actor      *actor;      /**<mouse actor*/
    GFC_Action *action;
    float       frame;
    Uint8       hidden;     /**<if true, don't show mouse or use its inputs*/
}Mouse;

static Mouse _mouse = {0};

int gf2d_mouse_hidden()
{
    return (_mouse.hidden > 0);
}

void gf2d_mouse_hide()
{
    _mouse.hidden++;
}

void gf2d_mouse_show()
{
    _mouse.hidden--;
}

void gf2d_mouse_set_action(const char *action)
{
    _mouse.action = gf2d_actor_get_action(_mouse.actor, action,&_mouse.frame);
}

void gf2d_mouse_load(const char *actorFile)
{
    if (_mouse.actor)gf2d_actor_free(_mouse.actor);
    _mouse.actor = gf2d_actor_load(actorFile);
    if (!_mouse.actor)slog("failed to load mouse actor");
    gf2d_mouse_set_action("default");
}

void gf2d_mouse_update()
{
    int x,y;
    gfc_action_next_frame(_mouse.action,&_mouse.frame);
    memcpy(&_mouse.mouse[1],&_mouse.mouse[0],sizeof(MouseState));
    _mouse.mouse[0].buttons = SDL_GetMouseState(&x,&y);
    gfc_vector2d_set(_mouse.mouse[0].position,x,y);
}

void gf2d_mouse_draw()
{
    if (_mouse.hidden > 0)return;
    gf2d_actor_draw(
        _mouse.actor,
        _mouse.frame,
        _mouse.mouse[0].position,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);
}

int gf2d_mouse_moved()
{
    if ((_mouse.mouse[0].position.x != _mouse.mouse[1].position.x) ||
        (_mouse.mouse[0].position.y != _mouse.mouse[1].position.y) ||
        (_mouse.mouse[0].buttons != _mouse.mouse[1].buttons))
    {
        return 1;
    }
    return 0;
}

int gf2d_mouse_button_pressed(int button)
{
    int mask;
    if (_mouse.hidden)return 0;
    mask = 1 << button;
    if ((_mouse.mouse[0].buttons & mask) &&
        !(_mouse.mouse[1].buttons & mask))
    {
        return 1;
    }
    return 0;
}

int gf2d_mouse_button_held(int button)
{
    int mask;
    if (_mouse.hidden)return 0;
    mask = 1 << button;
    if ((_mouse.mouse[0].buttons & mask) &&
        (_mouse.mouse[1].buttons & mask))
    {
        return 1;
    }
    return 0;
}

int gf2d_mouse_button_released(int button)
{
    int mask;
    if (_mouse.hidden)return 0;
    mask = 1 << button;
    if (!(_mouse.mouse[0].buttons & mask) &&
        (_mouse.mouse[1].buttons & mask))
    {
        return 1;
    }
    return 0;
}

int gf2d_mouse_button_state(int button)
{
    int mask;
    mask = 1 << button;
    return (_mouse.mouse[0].buttons & mask);
}

float gf2d_mouse_get_angle_to(GFC_Vector2D point)
{
    GFC_Vector2D delta;
    gfc_vector2d_sub(delta,_mouse.mouse[0].position,point);
    return gfc_vector2d_angle(delta);
}

GFC_Edge3D gf2d_mouse_get_cast_ray()
{
    GFC_Vector2D mouse,res,w;
    GFC_Vector3D position;
    GFC_Vector3D a = {0,5000,0},b;
    
    GFC_Vector3D forward = {0};
    GFC_Vector3D rightNear,right = {0};
    GFC_Vector3D upNear,up = {0};

    mouse = gf2d_mouse_get_position();
    
    res = gf3d_vgraphics_get_resolution();
    position = gf3d_camera_get_position();
    
    w.x = (mouse.x/res.x *2) -1;//amount right to move
    w.y = ((mouse.y/res.y *2) -1) * -1;//amount up to move
    
    gfc_vector2d_normalize(&res);
    gfc_vector3d_angle_vectors(gf3d_camera_get_angles(), &forward, &right, &up);

    gfc_vector3d_scale(rightNear,right,-1 * w.x * (res.x * 0.15));
    gfc_vector3d_scale(upNear,up,-1 * w.y * (res.y * 0.15));
     a.x = position.x + rightNear.x+ upNear.x;
     a.y = position.y + rightNear.y+ upNear.y;
     a.z = position.z + rightNear.z+ upNear.z;
    gfc_vector3d_scale(forward,forward,5000);
    gfc_vector3d_scale(right,right,-5000 * w.x);
    gfc_vector3d_scale(up,up,-5000 * w.y);
     b.x = forward.x + rightNear.x + upNear.x;
     b.y = forward.y + rightNear.y + upNear.y;
     b.z = forward.z + rightNear.z + upNear.z;
    return gfc_edge3d_from_vectors(a,b);
}

GFC_Vector2D gf2d_mouse_get_position()
{
    return _mouse.mouse[0].position;
}

GFC_Vector2D gf2d_mouse_get_movement()
{
    GFC_Vector2D dif;
    gfc_vector2d_sub(dif,_mouse.mouse[0].position,_mouse.mouse[1].position);
    return dif;
}

int gf2d_mouse_in_rect(GFC_Rect r)
{
    return gfc_point_in_rect(_mouse.mouse[0].position,r);
}

/*eol@eof*/
