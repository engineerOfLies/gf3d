#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "projectile.h"
#include "player.h"

static int thirdPersonMode = 0;
void player_think(Entity *self);
void player_update(Entity *self);

Entity *player_new(Vector3D position)
{
    Entity *ent = NULL;
    
    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no player for you!");
        return NULL;
    }
    
    ent->model = gf3d_model_load("models/dino.model");
    ent->think = player_think;
    ent->update = player_update;
    vector3d_copy(ent->position,position);
    
    
    
    ent->hidden = 1;
    return ent;
}


void player_think(Entity *self)
{
    Vector3D forward, move = {0};
    Vector2D mouse;
    int mx,my;
    Uint32 buttons;
    buttons = SDL_GetRelativeMouseState(&mx,&my);
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    mouse.x = mx;
    mouse.y = my;
    
    move = vector3d_get_from_angles(self->rotation);
    
    if (keys[SDL_SCANCODE_W])
    {
        slog("move: %f,%f,%f",move.x,move.y,move.z);
       vector3d_add(self->position,self->position,move);
//         self->position.y += 1;
    }
    if (keys[SDL_SCANCODE_S])
    {
       vector3d_add(self->position,self->position,-move);
//         self->position.y -= 1;
    }
    move = vector3d_get_from_angles(self->rotation);
    move.z = 0;
    vector3d_rotate_about_z(&move,-GFC_HALF_PI);
    vector3d_normalize(&move);
    if (keys[SDL_SCANCODE_D])
    {
        vector3d_add(self->position,self->position,move);
        slog("move:%f,%f,%f",move.x,move.y,move.z);
//         self->position.x += 1;
    }
    if (keys[SDL_SCANCODE_A])
    {
        vector3d_add(self->position,self->position,-move);
//         self->position.x -= 1;
    }

    if (keys[SDL_SCANCODE_SPACE])
    {
//         vector3d_add(self->position,self->position,move);
        self->position.z += 1;
    }
    if (keys[SDL_SCANCODE_Z])
    {
//         vector3d_sub(self->position,self->position,move);
        self->position.z -= 1;
    }
        
    if (keys[SDL_SCANCODE_UP])self->rotation.x += 0.00750;
    if (keys[SDL_SCANCODE_DOWN])self->rotation.x -= 0.00750;
    if (keys[SDL_SCANCODE_RIGHT])self->rotation.z -= 0.00750;
    if (keys[SDL_SCANCODE_LEFT])self->rotation.z += 0.00750;
    if (keys[SDL_SCANCODE_KP_RIGHTBRACE])self->rotation.y -= 0.00750;
    if (keys[SDL_SCANCODE_KP_LEFTBRACE])self->rotation.y += 0.00750;
    
    if (self->rotation.x >= (GFC_HALF_PI - GFC_EPSILON))self->rotation.x = GFC_HALF_PI - GFC_EPSILON;
    if (self->rotation.x <= -(GFC_HALF_PI - GFC_EPSILON))self->rotation.x = -(GFC_HALF_PI - GFC_EPSILON);
    
    
//     if (mouse.x != 0)self->rotation.z -= (mouse.x * 0.002);
//     if (mouse.y != 0)self->rotation.x += (mouse.y * 0.002);
    
    if (self->cooldown <= 0)
    {
        if (buttons)
        {
            forward = vector3d_get_from_angles(self->rotation);
//             vector3d_set(forward,0,-1,0);
//             vector3d_rotate_about_x(&forward, -self->rotation.x);
//             vector3d_rotate_about_z(&forward, self->rotation.z);

            projectile_new(self,"models/soul_reaver.model",self->position, forward, 1,1,0);
            self->cooldown = 100;
        }
    }
    else self->cooldown--;
    
    if (keys[SDL_SCANCODE_F3])
    {
        thirdPersonMode = !thirdPersonMode;
        self->hidden = !self->hidden;
    }
}

void player_update(Entity *self)
{
    Vector3D forward;
    Vector3D thrust;
    
    if (!self)return;
    
    //forward movement based on rotations
    forward = vector3d_get_from_angles(self->rotation);    
    vector3d_scale(thrust,forward,self->thrust);
    vector3d_add(self->position,self->position,thrust);

    
    gf3d_camera_set_position(self->position);
    gf3d_camera_set_rotation(self->rotation);
}

/*eol@eof*/
