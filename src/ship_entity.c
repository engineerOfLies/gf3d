#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_list.h"

#include "gf3d_draw.h"

#include "config_def.h"
#include "ship.h"
#include "ship_entity.h"


void ship_entity_update(Entity *self);
void ship_entity_draw(Entity *self);
void ship_entity_think(Entity *self);
void ship_entity_free(Entity *self);

Entity *ship_entity_new(Vector3D position,Ship *data,Color detailColor)
{
    const char *str;
    SJson *def;
    Entity *ent = NULL;    
    if (!data)
    {
        slog("ship_entity: null ship data provided");
        return NULL;
    }
    def = config_def_get_by_name("ships",data->name);
    if (!def)return NULL;
    ent = gf3d_entity_new();
    if (!ent)return NULL;
    
    str = sj_object_get_value_as_string(def,"model");
    if (str)
    {
        ent->model = gf3d_model_load(str);
        if (!ent->model)
        {
            slog("SHIP MODEL DID NOT LOAD");
        }
    }
    else
    {
        slog("no model specified for ship %s",data->name);
    }

    gfc_line_cpy(ent->name,data->displayName);
    sj_value_as_vector3d(sj_object_get_value(def,"scale"),&ent->mat.scale);
    sj_value_as_vector3d(sj_object_get_value(def,"rotation"),&ent->mat.rotation);
    vector3d_copy(ent->mat.position,position);
    
    ent->selectedColor = gfc_color(0.9,0.7,0.1,1);
    ent->color = gfc_color(1,1,1,1);
    gfc_color_copy(ent->detailColor,detailColor);
    slog("color: %f,%f,%f,%f",ent->detailColor.r,ent->detailColor.g,ent->detailColor.b,ent->detailColor.a);
    ent->think = ship_entity_think;
    ent->draw = ship_entity_draw;
    ent->update = ship_entity_update;
    ent->free = ship_entity_free;
    ent->data = data;
    vector3d_copy(ent->mat.position,position);
    return ent;
}

void ship_entity_free(Entity *self)
{
    // the ship entity doesn't own the Ship data, ship data owns this
}

void ship_entity_update(Entity *self)
{
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
}

void ship_entity_draw(Entity *self)
{
    Ship *data;
    if ((!self)||(!self->data))return;
    data = self->data;
    if ((strcmp(data->location,"docked")==0)||
        (strcmp(data->location,"interstellar")==0))
    {
        self->hidden = 1;
        return;// inside the station
    }
    self->hidden = 0;
}

void ship_entity_think(Entity *self)
{
    if (!self)return;
    // do maintenance
}

/*eol@eof*/
