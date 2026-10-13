#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"

int player_think(Entity* self);
int player_update(Entity* self);
int player_free(Entity* self);


Entity* player_new(GFC_Vector3D position) {
	Entity* self;
	self = entity_new();
	if (!self) {
		slog("failed to create a new player entity");
		return NULL;
	}

	self->model = model_load("models/dino/dino.model");
	self->position = position;
	self->think = player_think;
	return self;
}

int player_think(Entity* self)
{
	if (!self)
	{
		return 0;
	}
	self->rotation.z = sin(SDL_GetTicks() / 10);
	return 1;
}

int player_free(Entity* self) {
	return 1;
}