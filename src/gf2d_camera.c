#include "simple_logger.h"

#include "gf2d_camera.h"

typedef struct
{
    Rect view;
    Rect bounds;
}Camera;
static Camera _camera = {0};

void gf2d_camera_set_dimensions(Sint32 x,Sint32 y,Uint32 w,Uint32 h)
{
    gfc_rect_set(_camera.view,x,y,w,h);
}

Rect gf2d_camera_get_bounds()
{
    return _camera.bounds;
}

Rect gf2d_camera_get_dimensions()
{
    return _camera.view;
}

Vector2D gf2d_camera_get_position()
{
    return vector2d(_camera.view.x,_camera.view.y);
}

Vector2D gf2d_camera_get_size()
{
    return vector2d(_camera.view.w,_camera.view.h);
}

Vector2D gf2d_camera_get_offset()
{
    return vector2d(-_camera.view.x,-_camera.view.y);
}

void gf2d_camera_set_bounds(Sint32 x,Sint32 y,Uint32 w,Uint32 h)
{
    gfc_rect_set(_camera.bounds,x,y,w,h);
}

void gf2d_camera_bind()
{
    if (_camera.view.w > _camera.bounds.w)
    {
        _camera.view.x = -(_camera.view.w - _camera.bounds.w)/2;
    }
    else
    {
        if (_camera.view.x < _camera.bounds.x)_camera.view.x = _camera.bounds.x;
        if (_camera.view.x + _camera.view.w > _camera.bounds.x + _camera.bounds.w)_camera.view.x = _camera.bounds.x + _camera.bounds.w - _camera.view.w;
    }
    if (_camera.view.h > _camera.bounds.h)
    {
        _camera.view.y = -(_camera.view.h - _camera.bounds.h)/2;
    }
    else
    {
        if (_camera.view.y < _camera.bounds.y)_camera.view.y = _camera.bounds.y;
        if (_camera.view.y + _camera.view.h > _camera.bounds.y + _camera.bounds.h)_camera.view.y = _camera.bounds.y + _camera.bounds.h - _camera.view.h;
    }
}

void gf2d_camera_move(Vector2D v)
{
    vector2d_add(_camera.view,v,_camera.view);
}

void gf2d_camera_set_focus(Vector2D position)
{
    gf2d_camera_set_position(vector2d(position.x - (_camera.view.w/2),position.y - (_camera.view.h/2)));
}

void gf2d_camera_set_position(Vector2D position)
{
    vector2d_copy(_camera.view,position);
}

void gf2d_camera_set_position_absolute(Vector2D position)
{
    vector2d_copy(_camera.view,position);
}

void gf2d_camera_center_on(Vector2D position)
{
    Vector2D res;
    res = gf2d_camera_get_size();
    vector2d_scale(res,res,-0.5);
    vector2d_add(_camera.view,position,res);
}
/*eol@eof*/
