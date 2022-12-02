#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_lights.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"

#include "gf2d_mouse.h"

#include "resources.h"
#include "station.h"
#include "player.h"

static Entity *player_entity = NULL;

void player_draw(Entity *self);
void player_free(Entity *self);
void player_think(Entity *self);
void player_update(Entity *self);

SJson *player_data_save(PlayerData *data)
{
    SJson *json;
    if (!data)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
 
    sj_object_insert(json,"wages",sj_new_float(data->wages));
    sj_object_insert(json,"taxRate",sj_new_float(data->taxRate));
    sj_object_insert(json,"salesTaxRate",sj_new_float(data->salesTaxRate));
    sj_object_insert(json,"resources",resources_list_save(data->resources));
    return json;
}

void player_save(const char *filename)
{
    SJson *json;
    PlayerData *data;
    if (!filename)return;
    if (!player_entity)return;
    data = player_entity->data;
    if (!data)return;
    json = sj_object_new();
    
    sj_object_insert(json,"player",player_data_save(data));
    if (data->station)
    {
        sj_object_insert(json,"station",station_save_data(data->station->data));
    }

    sj_save(json,filename);
    sj_free(json);
}

PlayerData *player_data_parse(SJson *json)
{
    SJson *res;
    PlayerData *data;
    if (!json)return NULL;
    data = gfc_allocate_array(sizeof(PlayerData),1);
    if (!data)return NULL;
    data->resources = gfc_list_new();
    sj_get_float_value(sj_object_get_value(json,"wages"),&data->wages);
    sj_get_float_value(sj_object_get_value(json,"taxRate"),&data->taxRate);
    sj_get_float_value(sj_object_get_value(json,"salesTaxRate"),&data->salesTaxRate);
    res = sj_object_get_value(json,"resources");
    if (res)
    {
        data->resources = resources_list_parse(res);
    }
    else 
    {
        slog("no player resources");
    }
    return data;
}

Entity *player_new(const char *file)
{
    SJson *json;
    Entity *ent = NULL;
    PlayerData *data;
    
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
    ent->data = player_data_parse(sj_object_get_value(json,"player"));
    if (!ent->data)
    {
        slog("failed to parse player data");
        player_free(ent);
        return NULL;
    }
    data = ent->data;
    data->station = station_new(vector3d(0,0,0),sj_object_get_value(json,"station"));

    sj_free(json);
    
    player_save("saves/testsave.save");
    
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
