#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"
#include "world.h"

//static int thirdPersonMode = 0;
void player_think(Entity *self);
void player_update(Entity *self);
Vector3D camera_rotation;

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
    camera_rotation.x = GFC_PI;
    camera_rotation.z = GFC_HALF_PI;
    ent->rotation.x = camera_rotation.x ;
    ent->rotation.z = camera_rotation.z;
    ent->rotation.y = GFC_PI;

    //ent->hidden = 1;
    return ent;
}


void player_think(Entity *self)
{
    Vector3D forward = {0};
    Vector3D right = {0};
    Vector2D w,mouse;
    //Vector3D cameraPos, move;
    int mx,my;
    SDL_GetRelativeMouseState(&mx,&my);
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    mouse.x = mx;
    mouse.y = my;
    w = vector2d_from_angle(self->rotation.z);
    forward.x = w.x;
    forward.y = w.y;
    w = vector2d_from_angle(self->rotation.z - GFC_HALF_PI);
    right.x = w.x;
    right.y = w.y;
    if (keys[SDL_SCANCODE_W])
    {
        vector3d_add(self->position,self->position,forward);

//         move.z = 0;
//         vector3d_set_magnitude(&move, 100);
//         move.z = 100;
//         vector3d_add(cameraPos,self->position,move);
//         gf3d_camera_look_at(cameraPos, self->position, vector3d(0,0,1));
    }
    if (keys[SDL_SCANCODE_S])
    {
        vector3d_add(self->position,self->position,-forward);
    }
    if (keys[SDL_SCANCODE_D])
    {
        vector3d_add(self->position,self->position,right);
    }
    if (keys[SDL_SCANCODE_A])
    {
        vector3d_add(self->position,self->position,-right);
    }
    if (keys[SDL_SCANCODE_SPACE])self->position.z += 1;
    if (keys[SDL_SCANCODE_Z])self->position.z -= 1;



     if (keys[SDL_SCANCODE_UP])self->rotation.x -= 0.0050;
     if (keys[SDL_SCANCODE_DOWN])self->rotation.x += 0.0050;
    //if (keys[SDL_SCANCODE_RIGHT])self->rotation.z -= 0.0050;
    //if (keys[SDL_SCANCODE_LEFT])self->rotation.z += 0.0050;

    if (mouse.x != 0)camera_rotation.z -= (mouse.x * 0.001);
    if (mouse.y != 0)camera_rotation.x += (mouse.y * 0.001);
    self->rotation.z = camera_rotation.z;

//     if (keys[SDL_SCANCODE_F3])
//     {
//         thirdPersonMode = !thirdPersonMode;
//         self->hidden = !self->hidden;
//     }
}

void player_update(Entity *self)
{
    //Vector3D forward = {0};
    Vector3D position;
    Vector3D rotation;
    float height;
    //Vector2D w;
    Vector3D torso_position;
    Vector3D camera_position;

    if (!self)return;

    vector3d_copy(position,self->position);
    vector3d_copy(rotation,camera_rotation);
    vector3d_copy(torso_position, position);
    camera_position = torso_position;
    slog("rotation Value: \n x:%f \n y:%f \n z:%f", rotation.x, rotation.y, rotation.z);
    if(camera_rotation.x >= 4.5){
        camera_rotation.x = 4.5;
    }
    if(camera_rotation.x <= 1.2){
        camera_rotation.x = 1.2;
    }

    height = world_get_collision_height(self->position);
    if(self->position.z > height){
        self->velocity.z -= 0.0000098;
    }
    vector3d_add(self->position,self->position,self->velocity);
    if(self->position.z < height)
    {
        self->position.z = height;
        if(self->velocity.z < 0)self->velocity.z = 0;
    }
    //vector3d_copy(torso_position, self->position);
    //vector3d_copy(camera_position, torso_position);
    //camera_position = torso_position;
//     if (thirdPersonMode)
//     {
//         position.z += 100;
//         rotation.x += M_PI*0.125;
//         w = vector2d_from_angle(self->rotation.z);
//         forward.x = w.x * 100;
//         forward.y = w.y * 100;
//         vector3d_add(position,position,-forward);
//     }
     gf3d_camera_set_position(camera_position);
     gf3d_camera_set_rotation(rotation);
}

/*eol@eof*/
