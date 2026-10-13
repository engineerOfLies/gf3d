#include "simple_logger.h"

#include "entity.h"

extern int __DEBUG;

typedef struct
{
	Uint32 entity_count;
	Entity* entity_list;
}EntitySystem;

static EntitySystem entity_system = { 0 };

void entity_system_close();

Entity* entity_initialize(Uint32 entity_system_index);

void entity_update(Entity* self);

void entity_think(Entity* self);

void entity_system_init(Uint32 max_entities)
{
	if (entity_system.entity_count != 0) {
		slog("cannot initialize entity system, already initialized");
		return;
	}
	if (!max_entities)
	{
		slog("cannot initialize entity system for Zero entities");
		return;
	}
	entity_system.entity_list = gfc_allocate_array(sizeof(Entity), max_entities);
	if (!entity_system.entity_list) {
		slog("failed to allocate global entity list for %i entities", max_entities);
		return;
	}
	entity_system.entity_count = max_entities;

	if (__DEBUG)slog("entity system initialized");
	atexit(entity_system_close);
}

void entity_system_close()
{
	entity_system_free_all();
	if (entity_system.entity_list) {
		free(entity_system.entity_list);
		entity_system.entity_list = NULL;
	}
	memset(&entity_system, 0, sizeof(EntitySystem));
	if (__DEBUG)slog("entity system closed");
}

Entity* entity_new()
{
	int i;
	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			return entity_initialize(i);
		}
	}
	slog("entity_new: no free slots for new entities");
	return NULL;
}

Entity* entity_initialize(Uint32 entity_system_index)
{
	//boiler plate initilization goes here
	entity_system.entity_list[entity_system_index]._inuse = 1;
	entity_system.entity_list[entity_system_index].scale = gfc_vector3d(1, 1, 1);
	entity_system.entity_list[entity_system_index].color_mod = GFC_COLOR_WHITE;

	return &entity_system.entity_list[entity_system_index];
}

void entity_load(const char* filename);

void entity_free(Entity* self)
{
	if (!self)
	{
		slog("cannot free an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot free an entity that is not in use");
		return;
	}

	if (self->model)
	{
		model_free(self->model);
	}
	if (self->free)
	{
		if (self->free(self));
	}
	memset(self, 0, sizeof(Entity));
}

void entity_update(Entity* self)
{
	if (!self)
	{
		slog("cannot update an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot update an entity that is not in use");
		return;
	}
	if (self->update)
	{
		if (self->update(self))
		{
			return;
		}
	}
	//generic updating goes here
	gfc_vector3d_add(self->position, self->position, self->velocity);
}

void entity_think(Entity* self)
{
	if (!self)
	{
		slog("cannot think an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot think an entity that is not in use");
		return;
	}
	if (self->think)
	{
		if (self->think(self))
		{
			return;
		}
	}
	//generic thinking goes here
}

void entity_queue_render(Entity* self)
{
	if (!self)
	{
		slog("cannot update an entity that doesnt exist");
		return;
	}
	if (!self->_inuse)
	{
		slog("cannot update an entity that is not in use");
		return;
	}
	if (self->draw)
	{
		if (self->draw(self)) //if we want to draw using our own draw, return 1
		{
			return;
		}
	}

	gfc_matrix4_from_vectors(self->mat, self->position, self->rotation, self->scale);
	model_queue_render(self->model, self->mat, self->color_mod);
}

void entity_system_free_all() 
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot free entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (entity_system.entity_list[i]._inuse)
		{
			entity_free(&entity_system.entity_list[i]);
		}
	}
}

void entity_system_update_all()
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot update entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			continue;
		}
		entity_update(&entity_system.entity_list[i]);
	}
}

void entity_system_think_all()
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot think entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			continue;
		}
		entity_think(&entity_system.entity_list[i]);
	}
}

void entity_system_queue_render_all()
{
	int i;
	if (!entity_system.entity_list)
	{
		slog("cannot render entities from nonexistent entity_system.entity_list");
		return;
	}

	for (i = 0; i < entity_system.entity_count; i++)
	{
		if (!entity_system.entity_list[i]._inuse)
		{
			continue;
		}
		entity_queue_render(&entity_system.entity_list[i]);
	}
}