#include "simple_logger.h"

#include "gfc_matrix.h"

#include "player.h"

void player_think(Entity *self);
void player_update(Entity *self);
int player_draw(Entity* self);
void player_free(Entity *self);
void player_collider(Entity* self, Entity* other, GFC_Vector3D collision);

// Flag for collision
Bool collide = false;
/*
* @brief Type of collision being detected
* @param [0]-Objects
* @param [1]-Enemy Collision
* @param [2]-Enemy Hurtbox
* @param [3]-Enviormental
* @param [4]-Floor
*/
int typeCollision[] = { 0,0,0,0,0 };
/*
* @brief Entities that are touching this entity
* @param [0]-Objects
* @param [1]-Enemy Collision
* @param [2]-Enemy Hurtbox
* @param [3]-Enviormental
* @param [4]-Floor
*/
Entity entCollision[] = { 0,0,0,0,0 };

Entity* player_new(GFC_TextLine name, Model* model, GFC_Vector3D spawnPosition)
{
	Entity *self;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn player entity.");
		return NULL;
	}

	gfc_line_sprintf(self->tag, "Player");
	gfc_line_sprintf(self->name, name);
	self->position = spawnPosition;
	self->rotation = gfc_vector3d(0, 0, 0);
	self->scale = gfc_vector3d(1, 1, 1);
	self->model = model;
	self->velocity = gfc_vector3d(0, 0, 0);
	self->direction = gfc_vector3d(0, 0, 0);
	self->cameraMode = 0;
	self->radius = 0;
	//self->collision = gfc_box(self->position.x, self->position.y, self->position.z, 4, 4, 4);

	float xScale = 6.0f;
	float yScale = 6.0f;
	float zScale = 11.0f;

	self->collisionX = gfc_new_primitive(3, self->position.x-(xScale/2.0f), self->position.y-(yScale/2.0f), self->position.z-(zScale/2)-1, xScale, yScale, zScale, 0.0f, gfc_vector3d(0, 0, 0), gfc_vector3d(0, 0, 0), gfc_vector3d(0, 0, 0));

	self->think = player_think;
	self->update = player_update;
	self->draw = player_draw;
	self->free = player_free;
	self->touch = player_collider;

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

void player_collider(Entity* self, Entity* other, GFC_Vector3D collision) {
	if (!self)return;

	GFC_Vector3D dir = self->direction;
	if (strcmp(other->tag, "Object") == 0) {
		slog("Touching object");
		collide = true;
		typeCollision[0] = 1;
		entCollision[0] = *other;
	}	
}

void player_to_object(Entity* self, Entity* object) {
	//slog("Touching Object");
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
	
	if (keys[SDL_SCANCODE_SPACE]) dir.z += 1;

	// Add collision phycics here
	if (collide) {
		// Objects
		if (typeCollision[0] == 1) {
			
			typeCollision[0] = 0;
			
		}
		collide = false;
	}

	gfc_vector3d_normalize(&dir);
	gfc_vector3d_scale(self->velocity,dir,0.1);
}

void player_update(Entity *self)
{
	if (!self)return;

	gfc_vector3d_add(self->position, self->position, self->velocity);
	gfc_vector3d_add(self->collisionX.s.b, self->collisionX.s.b, self->velocity);

	//gfc_primitive_offset(self->collisionX, self->velocity);
	

	//int collided = ent_collid_action(self);

	//if (collided != 0) {
	//	slog("Colliding with object.");
	//}

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

