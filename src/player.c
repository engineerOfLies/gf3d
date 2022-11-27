#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_lights.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"

#include "gf2d_mouse.h"

#include "station.h"
#include "player.h"

static Entity *player_entity = NULL;

void player_draw(Entity *self);
void player_free(Entity *self);
void player_think(Entity *self);
void player_update(Entity *self);

PlayerData *player_data_parse(SJson *json)
{
    int i,c;
    const char *str;
    Resource *resource;
    SJson *list,*item;
    PlayerData *data;
    if (!json)return NULL;
    data = gfc_allocate_array(sizeof(PlayerData),1);
    if (!data)return NULL;
    data->resources = gfc_list_new();
    sj_get_float_value(sj_object_get_value(json,"credits"),&data->credits);
    sj_get_integer_value(sj_object_get_value(json,"population"),(int *)&data->population);
    sj_get_integer_value(sj_object_get_value(json,"staff"),(int *)&data->staff);
    sj_get_float_value(sj_object_get_value(json,"wages"),&data->wages);
    sj_get_float_value(sj_object_get_value(json,"taxRate"),&data->taxRate);
    sj_get_float_value(sj_object_get_value(json,"salesTaxRate"),&data->salesTaxRate);
    list = sj_object_get_value(json,"resources");
    c = sj_array_get_count(list);
    for (i = 0;i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        resource = gfc_allocate_array(sizeof(Resource),1);
        if (!resource)continue;
        sj_get_integer_value(sj_object_get_value(item,"amount"),(int *)&resource->amount);
        sj_get_float_value(sj_object_get_value(item,"value"),&resource->value);
        str = sj_get_string_value(sj_object_get_value(item,"name"));
        if (str)gfc_line_cpy(resource->name,str);
        data->resources = gfc_list_append(data->resources,resource);
    }
    data->station = station_new(vector3d(0,0,0),"saves/default.save");
    return data;
}

Entity *player_new(const char *file)
{
    SJson *json;
    Entity *ent = NULL;
    
    if (!file)
    {
        slog("no player file provided");
        return NULL;
    }
    json = sj_load(file);
    if (!json)return NULL;

    if (!player_entity)
    {
        ent = entity_new();
        if (!ent)
        {
            slog("UGH OHHHH, no player for you!");
            return NULL;
        }
        player_entity = ent;
    }
    else
    {
        ent = player_entity;
        if (ent->data)
        {
            player_free(ent);
        }
        ent->data = NULL;
    }
    ent->data = player_data_parse(json);
    sj_free(json);
    
    
    ent->think = player_think;
    ent->update = player_update;
    ent->draw = player_draw;
    ent->free = player_free;
    
    return ent;
}

void player_free(Entity *self)
{
    int i,c;
    Resource *resource;
    PlayerData *data;
    if ((!self)||(!self->data))return;
    data = self->data;
    c = gfc_list_get_count(data->resources);
    for (i = 0; i < c; i++)
    {
        resource = gfc_list_get_nth(data->resources,i);
        if (!resource)continue;
        free(resource);
    }
    gfc_list_delete(data->resources);
    entity_free(data->station);
    free(data);
}

void player_draw(Entity *self)
{
}

void player_think(Entity *self)
{
}

void player_update(Entity *self)
{
    // this will be where I run the economy upkeep for the player.
}

PlayerData *player_get_data()
{
    if (!player_entity)return NULL;
    return player_entity->data;
}

/*eol@eof*/
