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
    camera_entity_enable_free_look(!camera_entity->hidden);
}

void camera_entity_enable_free_look(Uint8 enable)
{
    if (!camera_entity)return;
    if (!enable)
    {
        gf3d_camera_set_position(camera_entity->acceleration);
    }
    else
    {
        vector3d_copy(camera_entity->acceleration,gf3d_camera_get_position());
    }
    camera_entity->hidden = enable;
}


void camera_entity_think(Entity *self)
{
    float moveSpeed = 1;
    Vector3D position,rotation;
    const Uint8 * keys;

    if (self->hidden)
    {
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
        
        if (keys[SDL_SCANCODE_R])
        {
            rotation = gf3d_camera_get_angles();
            slog("rotation: %f,%f,%f",rotation.x,rotation.y,rotation.z);
        }
        if (keys[SDL_SCANCODE_P])
        {
            position = gf3d_camera_get_position();
            slog("position: %f,%f,%f",position.x,position.y,position.z);
        }
        return;
    }
    gf3d_camera_walk_right(moveSpeed/10);
    gf3d_camera_look_at(vector3d(0,0,0),NULL);
}

/*eol@eof*/
