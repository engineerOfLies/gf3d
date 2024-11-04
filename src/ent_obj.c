#include "simple_logger.h"

#include "gfc_matrix.h"

#include "ent_obj.h"

void obj_think(Entity* self);
void obj_update(Entity* self);
int	 obj_draw(Entity* self);
void obj_free(Entity* self);
void obj_collider(Entity* self, Entity* other);

Entity* obj_new(GFC_TextLine name, Model* model, GFC_Vector3D spawnPosition)
{
	Entity* self;

	self = entity_new();
	if (!self)
	{
		slog("Failed to spawn object entity.");
		return NULL;
	}

	gfc_line_sprintf(self->tag, "Object");
	gfc_line_sprintf(self->name, name);
	//self->name = name;
	self->position = spawnPosition;
	self->rotation = gfc_vector3d(0, 0, 0);
	self->scale = gfc_vector3d(2, 2, 2);
	self->model = model;
	self->velocity = gfc_vector3d(0, 0, 0);
	self->direction = gfc_vector3d(0, 0, 0);
	self->radius = 0;

	float xScale = 4.0f;
	float yScale = 4.0f;
	float zScale = 4.0f;

	self->collisionX = gfc_primitive_offset(gfc_new_primitive(3, self->position.x, self->position.y, self->position.z, xScale, yScale, zScale, 0.0f, gfc_vector3d(0, 0, 0), gfc_vector3d(0, 0, 0), gfc_vector3d(0, 0, 0)), gfc_vector3d(1,-1,-1));


	self->think = obj_think;
	self->update = obj_update;
	self->draw = obj_draw;
	self->free = obj_free;
	self->touch = obj_collider;

	slog("%s succefully spawned.", self->name);
	return self;
}

void obj_collider(Entity* self, Entity* other) {
	//slog("Object collision with %s", other->name);
}

void obj_think(Entity* self)
{
	if (!self)return;

	GFC_Vector3D dir = self->direction;

	gfc_vector3d_normalize(&dir);
	gfc_vector3d_scale(self->velocity, dir, 1);
}

void obj_update(Entity* self)
{
	if (!self)return;

	gfc_vector3d_add(self->position, self->position, self->velocity);
	//gfc_vector3d_add(self->collision, self->position, self->velocity);
	//self->collision = gfc_box(self->position.x, self->position.y, self->position.z, 3, 3, 3);
}

int obj_draw(Entity* self)
{
	if (!self)return;
	//slog("Object is drawing");
	return;
}

void obj_free(Entity* self)
{
	if (!self)return;
}

