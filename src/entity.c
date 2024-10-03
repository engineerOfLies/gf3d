#include "simple_logger.h"

#include "gfc_matrix.h"

#include "entity.h"

typedef struct
{
	Entity* entity_list;
	Uint32		entity_max;
}EntityManager;

// Used for sanity check
static EntityManager _entity_manager = { 0 };	/**Initialize a LOCAL GLOBAL entity manager*/

void entity_system_close()
{
	entity_clear_all(NULL);
	if (_entity_manager.entity_list)free(_entity_manager.entity_list);
	memset(&_entity_manager, 0, sizeof(EntityManager));
}

void entity_system_init(Uint32 max_ents)
{
	if (!_entity_manager.entity_list)
	{
		slog("cannot have two instances of an entity manager, one is already active");
		return;
	}
	if (!max_ents)
	{
		slog("cannot allocate 0 entities!");
	}
	_entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), max_ents);
	if (!_entity_manager.entity_list) {
		slog("failed to allocated global entity list");
		return;
	}
	_entity_manager.entity_max = max_ents;
	atexit(entity_system_close);
}

void entity_clear_all(Entity* ignore)
{
	int i;
	for (i = 0; i < _entity_manager.entity_max; i++)
	{
		if (&_entity_manager.entity_list[i] == ignore)continue;	//Skip this iteration if the entity is suposed to be ignored.
		if (!_entity_manager.entity_list[i]._inuse)continue;	//Skip this iteration of the loop
		entity_free(&_entity_manager.entity_list[i]);
	}
}

Entity* entity_new()
{
	int i;
	for (i = 0; i < _entity_manager.entity_list; i++)
	{
		if (_entity_manager.entity_list[i]._inuse)continue;		//Skip any active entites
		memset(&_entity_manager.entity_list[i], 0, sizeof(Entity));
		_entity_manager.entity_list[i]._inuse = 1;
		//setting the default color
		//setting the default scale
		return &_entity_manager.entity_list[i];
	}
	slog("no more available entities");
	return NULL;
}

void entity_free(Entity* self)
{
	if (!self)return; //can't do work on nothing
	gf3d_model_free(self->model);
	// Anything else we allocate for our entity would get cleaned up here
	if (self->free)self->free(self->data);
}

void entity_think(Entity* self)
{
	if (!self) return;
	// Check if entity can think and make it do it's think function
	if (self->think)self->think(self);
}

void entity_system_think()
{
	int i;
	for (i = 0; i < _entity_manager.entity_list; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse)continue;		//Skip any inactive entites
		entity_think(&_entity_manager.entity_list[i]);
	}
}

void entity_update(Entity* self)
{
	if (!self) return;
	// Check if entity can think and make it do it's think function
	if (self->update)self->update(self);
}

void entity_system_update()
{
	int i;
	for (i = 0; i < _entity_manager.entity_list; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse)continue;		//Skip any inactive entites
		entity_update(&_entity_manager.entity_list[i]);
	}
}

void entity_draw(Entity* self)
{
	GFC_Matrix4 matrix;
	if (!self)return;

	if (self->draw)
	{
		if (self->draw(self) == -1)return;
	}

	gfc_matrix4_from_vectors(
		matrix,
		self->position,
		self->rotation,
		self->scale
	);

	gf3d_model_draw(
		self->model,
		matrix,
		GFC_COLOR_WHITE,
		0
	);
}

void entity_system_draw()
{
	int i;
	for (i = 0; i < _entity_manager.entity_list; i++)
	{
		if (!_entity_manager.entity_list[i]._inuse)continue;		//Skip any inactive entites
		entity_draw(&_entity_manager.entity_list[i]);
	}
}
