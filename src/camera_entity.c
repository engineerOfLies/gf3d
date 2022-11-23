#include "simple_logger.h"
#include "gfc_types.h"
#include "gfc_input.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_lights.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"

#include "gf2d_mouse.h"

#include "camera_entity.h"


static Entity *camera_entity = NULL;

void camera_entity_think(Entity *self);

Entity *camera_entity_new(Vector3D position,Vector3D rotation)
{
    Entity *ent = NULL;

    gf3d_camera_set_position(position);
    gf3d_camera_set_rotation(rotation);
    
    if (camera_entity)
    {
        return camera_entity;
    }
    
    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no camera_entity for you!");
        return NULL;
    }
    
    ent->think = camera_entity_think;
    ent->hidden = 0;
    camera_entity = ent;
    return ent;
}

void camera_entity_toggle_free_look()
{
    camera_entity->hidden = !camera_entity->hidden;
}

void camera_entity_enable_free_look(Uint8 enable)
{
    if (!camera_entity)return;
    camera_entity->hidden = enable;
}


void camera_entity_think(Entity *self)
{
    float moveSpeed = 0.1;
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    if (!self->hidden)return;
    
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

/*eol@eof*/
