#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_lights.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"

#include "gf2d_mouse.h"

#include "player.h"

static int thirdPersonMode = 0;
void player_think(Entity *self);
void player_update(Entity *self);

void player_draw(Entity *self)
{
}

Entity *player_new(Vector3D position,Vector3D rotation)
{
    Entity *ent = NULL;
    
    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no player for you!");
        return NULL;
    }
    
    ent->think = player_think;
    ent->update = player_update;
    gf3d_model_mat_set_position(&ent->mat,position);
    gf3d_model_mat_set_rotation(&ent->mat,rotation);

    ent->hidden = 0;
    ent->draw = player_draw;
    return ent;
}


void player_think(Entity *self)
{
    Vector3D forward = {0};
    Vector3D right = {0};
    Vector2D w;
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    w = vector2d_from_angle(self->mat.rotation.z);
    forward.x = w.x;
    forward.y = w.y;
    w = vector2d_from_angle(self->mat.rotation.z - GFC_HALF_PI);
    right.x = w.x;
    right.y = w.y;
    
    vector3d_scale(forward,forward,0.1);
    vector3d_scale(right,right,0.1);
    if (keys[SDL_SCANCODE_W])
    {   
        gf3d_model_mat_move(&self->mat,forward);
    }
    if (keys[SDL_SCANCODE_S])
    {
        gf3d_model_mat_move(&self->mat,vector3d(-forward.x,-forward.y,-forward.z));
    }
    if (keys[SDL_SCANCODE_D])
    {
        gf3d_model_mat_move(&self->mat,right);
    }
    if (keys[SDL_SCANCODE_A])    
    {
        gf3d_model_mat_move(&self->mat,vector3d(-right.x,-right.y,-right.z));
    }
    if (keys[SDL_SCANCODE_SPACE])self->mat.position.z += 0.1;
    if (keys[SDL_SCANCODE_Z])self->mat.position.z -= 0.1;
    
    if (keys[SDL_SCANCODE_UP])self->mat.rotation.x -= 0.0050;
    if (keys[SDL_SCANCODE_DOWN])self->mat.rotation.x += 0.0050;
    if (keys[SDL_SCANCODE_RIGHT])self->mat.rotation.z -= 0.0050;
    if (keys[SDL_SCANCODE_LEFT])self->mat.rotation.z += 0.0050;
    
    if (keys[SDL_SCANCODE_F3])
    {
        thirdPersonMode = !thirdPersonMode;
        self->hidden = !self->hidden;
    }
    
    if (keys[SDL_SCANCODE_O])
    {
        slog("position: %f,%f,%f",self->mat.position.x,self->mat.position.y,self->mat.position.z);
        slog("rotation: %f,%f,%f",self->mat.rotation.x,self->mat.rotation.y,self->mat.rotation.z);
    }
}

void player_update(Entity *self)
{
    Vector3D forward = {0};
    Vector3D position;
    Vector3D rotation;
    Vector2D w;
    
    if (!self)return;
    
    vector3d_copy(position,self->mat.position);
    vector3d_copy(rotation,self->mat.rotation);
    gf3d_camera_set_position(position);
    gf3d_camera_set_rotation(rotation);
}

/*eol@eof*/
