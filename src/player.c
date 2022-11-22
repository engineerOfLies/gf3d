#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_lights.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"

#include "gf2d_mouse.h"

#include "player.h"

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
    
    gf3d_camera_set_position(position);
    gf3d_camera_set_rotation(rotation);
    return ent;
}


void player_think(Entity *self)
{
    float moveSpeed = 0.1;
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    
    if (keys[SDL_SCANCODE_W])
    {
        gf3d_camera_walk_forward(moveSpeed);
    }
    if (keys[SDL_SCANCODE_S])
    {
        gf3d_camera_walk_forward(-moveSpeed);
    }
    if (keys[SDL_SCANCODE_D])
    {
        gf3d_camera_walk_right(moveSpeed);
    }
    if (keys[SDL_SCANCODE_A])    
    {
        gf3d_camera_walk_right(-moveSpeed);
    }
    if (keys[SDL_SCANCODE_SPACE])gf3d_camera_move_up(moveSpeed);
    if (keys[SDL_SCANCODE_Z])gf3d_camera_move_up(-moveSpeed);
    
    if (keys[SDL_SCANCODE_UP])gf3d_camera_pitch(-0.005);
    if (keys[SDL_SCANCODE_DOWN])gf3d_camera_pitch(0.005);
    if (keys[SDL_SCANCODE_RIGHT])gf3d_camera_yaw(-0.005);
    if (keys[SDL_SCANCODE_LEFT])gf3d_camera_yaw(0.005);
    
    if (keys[SDL_SCANCODE_O])
    {
        slog("position: %f,%f,%f",self->mat.position.x,self->mat.position.y,self->mat.position.z);
        slog("rotation: %f,%f,%f",self->mat.rotation.x,self->mat.rotation.y,self->mat.rotation.z);
    }
}

void player_update(Entity *self)
{
    
    if (!self)return;
}

/*eol@eof*/
