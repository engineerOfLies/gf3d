#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"
#include "world.h"

//static int thirdPersonMode = 0;
void player_think(Entity *self);
void player_update(Entity *self);
//void player_free(Entity *self);
Vector3D camera_rotation;


Entity *player_new(Vector3D position)
{
    Entity *ent = NULL;

    //rlStatuses *statuses = NULL;

    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no player for you!");
        return NULL;
    }

    ent->model = gf3d_model_load("models/dino.model");
    ent->think = player_think;
    ent->update = player_update;
    //ent->free = player_free;
    vector3d_copy(ent->position,position);
    camera_rotation.x = GFC_PI;
    camera_rotation.z = GFC_HALF_PI;
    ent->rotation.x = camera_rotation.x ;
    ent->rotation.z = camera_rotation.z;
    ent->rotation.y = GFC_PI;
    ent->health = 100;
    ent->isPlayer = 1;

    // statuses = gfc_allocate_array(sizeof(rlStatuses),1);
    //
    ent->hydration = 100;
    ent->saturation = 100;
    ent->defication = 0;
    ent->sanityation = 100;
    ent->calefaction = 97.8;
    //
    // ent->customData = statuses;

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
    forward.x = ((w.x)*0.0625);
    forward.y = ((w.y)*0.0625);
    w = vector2d_from_angle(self->rotation.z - GFC_HALF_PI);
    right.x = ((w.x)*0.0625);
    right.y = ((w.y)*0.0625);
    if (keys[SDL_SCANCODE_W])
    {
        vector3d_add(self->position,self->position,forward);
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
    if (keys[SDL_SCANCODE_SPACE]){
        if(self->velocity.z == 0){
            self->velocity.z = 0.25;
        }
    }

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


    if(camera_rotation.x >= 4.5){
        camera_rotation.x = 4.5;
    }
    if(camera_rotation.x <= 1.2){
        camera_rotation.x = 1.2;
    }

    height = world_get_collision_height(self->position);
    if(self->position.z > height){
        self->velocity.z -= 0.00098;
    }
    vector3d_add(self->position,self->position,self->velocity);
    if(self->position.z < height)
    {
        self->position.z = height;
        if(self->velocity.z < 0)self->velocity.z = 0;
    }
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

    if(SDL_GetTicks64()%5000 == 0)
    {
        self->saturation -= 1;
        self->hydration -= 2;
        self->defication += 2;
    }

        if(SDL_GetTicks64()%10000 == 0)
    {
        self->calefaction -= 0.25;
    }
}


// void player_free(Entity *self){
//
//     rlStatuses *statuses;
//
//     if (!self)return;
//     statuses = self->customData;
//     free(statuses);
// }



/*eol@eof*/
