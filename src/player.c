#include "simple_logger.h"

#include "gfc_matrix.h"

#include "player.h"

void player_think(Entity *self);
void player_update(Entity *self);
int player_draw(Entity* self);
void player_free(Entity *self);

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

	self->think = player_think;
	self->update = player_update;
	self->draw = player_draw;
	self->free = player_free;

	slog("Player succefully spawned.");
	return self;
}

void player_think(Entity *self)
{
	if (!self)return;

	GFC_Vector3D dir = { 0,0,0 };
	gfc_vector3d_normalize(&dir);
	gfc_vector3d_scale(self->velocity,dir,3);
}

void player_update(Entity *self)
{
	if (!self)return;

	gfc_vector3d_add(self->position, self->position, self->velocity);
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