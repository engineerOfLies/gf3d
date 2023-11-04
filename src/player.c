#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"

#define GRAVITY -0.00000000000000000000981f

static int thirdPersonMode = 1;
void player_think(Entity *self);
void player_update(Entity *self, float deltaTime);

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
    ent->size = gf3d_get_model_size_from_obj("models/dino/dino.obj");
    ent->boundingBox.min = get_Bounding_Box_Min(ent->size, ent->position);
    ent->boundingBox.max = get_Bounding_Box_Max(ent->size, ent->position);
    ent->velocity = Vector3D_Zero();
    ent->acceleration = Vector3D_Zero();
    ent->acceleration.z = GRAVITY;
    ent->rotation.x = -GFC_PI;
    ent->rotation.z = -GFC_HALF_PI*0.125;
    ent->hidden = 0;
    return ent;
}


void player_think(Entity* self)
{
    Vector3D forward = { 0 };
    Vector3D right = { 0 };
    Vector2D w, mouse;
    int mx, my;
    SDL_GetRelativeMouseState(&mx, &my);
    const Uint8* keys;
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
        vector3d_add(self->position, self->position, forward);
    }
    if (keys[SDL_SCANCODE_S])
    {
        vector3d_add(self->position, self->position, -forward);
    }
    if (keys[SDL_SCANCODE_D])
    {
        vector3d_add(self->position, self->position, right);
    }
    if (keys[SDL_SCANCODE_A])
    {
        vector3d_add(self->position, self->position, -right);
    }
    if (keys[SDL_SCANCODE_SPACE])self->position.z += 1;
    if (keys[SDL_SCANCODE_Z])self->position.z -= 1;

    // Camera rotation
    if (mouse.x != 0)self->rotation.z -= (mouse.x * 0.001);
    if (mouse.y != 0)
    {
        // Adjust the camera's pitch based on mouse movement
        Vector3D cameraRotation;
        gf3d_camera_get_rotation(&cameraRotation);
        cameraRotation.x += (mouse.y * 0.001);
        gf3d_camera_set_rotation(cameraRotation);
    }

    if (keys[SDL_SCANCODE_F3])
    {
        thirdPersonMode = !thirdPersonMode;
        self->hidden = !self->hidden;
    }
}

void player_update(Entity* self, float deltaTime)
{
    if (!self) return;

    self->boundingBox.min = get_Bounding_Box_Min(self->size, self->position);
    self->boundingBox.max = get_Bounding_Box_Max(self->size, self->position);

    if (!self->grounded)
    {
        self->velocity.z = GRAVITY;
        self->position.z += self->velocity.z * deltaTime;

    }
    else
    {
        self->velocity.z = 0;
    }

    self->position.x += self->velocity.x * deltaTime;
    self->position.y += self->velocity.y * deltaTime;
    
    
    //slog("Current position %f, %f, %f", self->position.x, self->position.y, self->position.z);

    Vector3D cameraOffset = { 0, 100, -50 }; 
    Vector3D cameraPosition;
    cameraPosition.x = self->position.x + cameraOffset.x;
    cameraPosition.y = self->position.y + cameraOffset.y;
    cameraPosition.z = self->position.z + cameraOffset.z;

    Vector3D cameraRotation = { GFC_HALF_PI / 4, 0, self->rotation.z };

    gf3d_camera_set_position(cameraPosition);
    gf3d_camera_set_rotation(cameraRotation);
}

/*eol@eof*/
