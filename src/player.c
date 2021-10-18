#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"


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
    
//    ent->model = gf3d_model_load("dino");
    ent->think = player_think;
    ent->update = player_update;
    vector3d_copy(ent->position,position);
    ent->rotation.x = -M_PI;
    return ent;
}


void player_think(Entity *self)
{
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    if (keys[SDL_SCANCODE_W])self->position.y += 0.10;
    if (keys[SDL_SCANCODE_S])self->position.y -= 0.10;
    if (keys[SDL_SCANCODE_A])self->position.x -= 0.10;
    if (keys[SDL_SCANCODE_D])self->position.x += 0.10;
    if (keys[SDL_SCANCODE_SPACE])self->position.z += 0.10;
    if (keys[SDL_SCANCODE_Z])self->position.z -= 0.10;
    
    if (keys[SDL_SCANCODE_UP])self->rotation.x += 0.0010;
    if (keys[SDL_SCANCODE_DOWN])self->rotation.x -= 0.0010;
    if (keys[SDL_SCANCODE_LEFT])self->rotation.z += 0.0010;
    if (keys[SDL_SCANCODE_RIGHT])self->rotation.z -= 0.0010;

}

void player_update(Entity *self)
{
    if (!self)return;
    gf3d_camera_set_position(self->position);
    gf3d_camera_set_rotation(self->rotation);
}

/*eol@eof*/
