#include "simple_logger.h"

#include "gfc_matrix.h"

#include "player.h"

void player_think(Entity *self);
void player_update(Entity *self);
int player_draw(Entity* self);
void player_free(Entity *self);
//void update_Camera_Mode(Entity *self, int camera);
//int get_Camera_Mode(Entity *self);

Entity* player_new(GFC_TextLine* name, Model* model, GFC_Vector3D spawnPosition)
{
	Entity *self;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn player entity.");
		return NULL;
	}

	self->name = name;
	self->position = spawnPosition;
	self->rotation = gfc_vector3d(0, 0, 0);
	self->scale = gfc_vector3d(1, 1, 1);
	self->model = model;
	self->velocity = gfc_vector3d(0, 0, 0);
	self->direction = gfc_vector3d(0, 0, 0);
	self->cameraMode = 0;
	self->radius = 0;

	self->think = player_think;
	self->update = player_update;
	self->draw = player_draw;
	self->free = player_free;

	slog("Player succefully spawned.");
	return self;
}

void update_Camera_Mode(Entity *self, int camera)
{
	self->cameraMode = camera;
}

int get_Camera_Mode(Entity *self) 
{
	return self->cameraMode;
}

void player_think(Entity *self)
{
	if (!self)return;

	GFC_Vector3D dir = self->direction;
	const Uint8* keys;

	keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
	
	// Player Control mode
	if (self->cameraMode == 0) {
		if (keys[SDL_SCANCODE_W]) dir.y = -1; 	// Press W
		if (keys[SDL_SCANCODE_S]){ dir.y = 1;  }	// Press S
		if (keys[SDL_SCANCODE_D]){ dir.x = -1;  }	// Press D
		if (keys[SDL_SCANCODE_A]){ dir.x = 1; }	// Press A
	}
	// Target Lock mode
	else if (self->cameraMode == 4) 
	{
		if (keys[SDL_SCANCODE_W]) dir.y = -1;			// Press W
		if (keys[SDL_SCANCODE_S]) dir.y = 1;				// Press S
		if (keys[SDL_SCANCODE_D]) dir.x = -1;			// Press D
		if (keys[SDL_SCANCODE_A]) dir.x = 1;				// Press A
	}
	else
	{
		if (keys[SDL_SCANCODE_W]) dir.y = -1;			// Press W
		if (keys[SDL_SCANCODE_S]) dir.y = 1;				// Press S
		if (keys[SDL_SCANCODE_D]) dir.x = -1;			// Press D
		if (keys[SDL_SCANCODE_A]) dir.x = 1;

		
	}
	
	
	gfc_vector3d_normalize(&dir);
	gfc_vector3d_scale(self->velocity,dir,3);
}

void player_update(Entity *self)
{
	if (!self)return;

	gfc_vector3d_add(self->position, self->position, self->velocity);


	/*
	float direction;
	//gfc_vector3d_normalize(&self->velocity);

	if (self->velocity.y == 0 && self->velocity.x == 0) direction = 0;
	else {
		// Forward and back
		if (self->velocity.x) direction = (atan2(self->velocity.y, self->velocity.x));  
		else if (self->velocity.y > 0)  direction = GFC_HALF_PI;	//Left
		else { direction = -GFC_HALF_PI; }							//Right

		if (direction < 0) { direction += GFC_2PI; }
	}

	//slog("%f", direction);

	//Forward = 0
	//Right = One and a half PI
	//Left = Half PI
	//Down = PI

	// Forward
	if (direction == 0.0f) self->rotation.z = 0.0f;
	else if (direction == GFC_PI) self->rotation.z = GFC_PI;
	else if (direction == GFC_HALF_PI) self->rotation.z = GFC_HALF_PI;
	else if (direction == -GFC_HALF_PI) self->rotation.z = -GFC_HALF_PI;*/
}

int player_draw(Entity* self)
{
	if (!self)return;
	//slog("Player is drawing");
	return;
}

void player_free(Entity *self)
{
	if (!self)return;
}

