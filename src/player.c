#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"

#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_lights.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"

#include "gf2d_mouse.h"
#include "gf2d_message_buffer.h"

#include "resources.h"
#include "station.h"
#include "event_manager.h"
#include "player.h"

static Entity *player_entity = NULL;

void player_draw(Entity *self);
void player_free(Entity *self);
void player_think(Entity *self);
void player_update(Entity *self);

SJson *player_get_id_pool()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->idPools;
}

SJson *player_get_history()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->history;
}

SJson *player_data_save(PlayerData *data)
{
    SJson *json;
    if (!data)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
 
    sj_object_insert(json,"filename",sj_new_str(data->filename));
    sj_object_insert(json,"name",sj_new_str(data->name));
    sj_object_insert(json,"assistantName",sj_new_str(data->assistantName));
    sj_object_insert(json,"wages",sj_new_float(data->wages));
    sj_object_insert(json,"taxRate",sj_new_float(data->taxRate));
    sj_object_insert(json,"salesTaxRate",sj_new_float(data->salesTaxRate));
    sj_object_insert(json,"staff",sj_new_int(data->staff));
    sj_object_insert(json,"population",sj_new_int(data->population));
    sj_object_insert(json,"hour",sj_new_uint32(data->hour));
    sj_object_insert(json,"day",sj_new_uint32(data->day));
    sj_object_insert(json,"resources",resources_list_save(data->resources));
    sj_object_insert(json,"yesterday",resources_list_save(data->yesterday));
    sj_object_insert(json,"lastMonth",resources_list_save(data->lastMonth));
    sj_object_insert(json,"lastYear",resources_list_save(data->lastYear));
    sj_object_insert(json,"stockpile",resources_list_save(data->stockpile));
    sj_object_insert(json,"salePrice",resources_list_save(data->salePrice));
    sj_object_insert(json,"allowSale",resources_list_save(data->allowSale));
    sj_object_insert(json,"history",sj_copy(data->history));
    sj_object_insert(json,"idPools",sj_copy(data->idPools));
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
    
    gfc_line_cpy(data->filename,filename);
    
    sj_object_insert(json,"player",player_data_save(data));
    if (data->station)
    {
        sj_object_insert(json,"station",station_save_data(data->station->data));
    }
    sj_object_insert(json,"missions",missions_save_to_config());
    sj_object_insert(json,"planet",planet_save_to_config(data->planet));

    sj_save(json,filename);
    sj_free(json);
}

PlayerData *player_data_parse(SJson *json)
{
    const char *str;
    SJson *res;
    PlayerData *data;
    if (!json)return NULL;
    data = gfc_allocate_array(sizeof(PlayerData),1);
    if (!data)return NULL;
    str = sj_object_get_value_as_string(json,"name");
    if (str)gfc_line_cpy(data->name,str);
    
    str = sj_object_get_value_as_string(json,"filename");
    if (str)gfc_line_cpy(data->filename,str);

    str = sj_object_get_value_as_string(json,"assistantName");
    if (str)gfc_line_cpy(data->assistantName,str);

    sj_object_get_value_as_uint32(json,"hour",&data->day);
    sj_object_get_value_as_uint32(json,"day",&data->day);
    sj_get_integer_value(sj_object_get_value(json,"staff"),&data->staff);
    sj_get_integer_value(sj_object_get_value(json,"population"),&data->population);
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
        data->resources = gfc_list_new();
    }
    res = sj_object_get_value(json,"yesterday");
    if (res)
    {
        data->yesterday = resources_list_parse(res);
    }
    else
    {
        data->yesterday = resources_list_duplicate(data->resources);
    }
    res = sj_object_get_value(json,"lastMonth");
    if (res)
    {
        data->lastMonth = resources_list_parse(res);
    }
    else
    {
        data->lastMonth = resources_list_duplicate(data->resources);
    }
    res = sj_object_get_value(json,"lastYear");
    if (res)
    {
        data->lastYear = resources_list_parse(res);
    }
    else
    {
        data->lastYear = resources_list_duplicate(data->resources);
    }
    res = sj_object_get_value(json,"stockpile");
    if (res)
    {
        data->stockpile = resources_list_parse(res);
    }
    else
    {
        data->allowSale = gfc_list_new();
    }
    res = sj_object_get_value(json,"salePrice");
    if (res)
    {
        data->salePrice = resources_list_parse(res);
    }
    else
    {
        data->allowSale = gfc_list_new();
    }
    res = sj_object_get_value(json,"allowSale");
    if (res)
    {
        data->allowSale = resources_list_parse(res);
    }
    else
    {
        data->allowSale = gfc_list_new();
    }

    res = sj_object_get_value(json,"history");
    if (!res)
    {
        data->history = sj_object_new();
    }
    else
    {
        data->history = sj_copy(res);
    }
    res = sj_object_get_value(json,"idPools");
    if (!res)
    {
        data->idPools = sj_object_new();
    }
    else
    {
        data->idPools = sj_copy(res);
    }

    return data;
}

Entity *player_new(const char *file)
{
    SJson *json,*res;
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
        entity_free(player_entity);
        ent = entity_new();
        if (!ent)
        {
            slog("UGH OHHHH, no player for you!");
            return NULL;
        }
        player_entity = ent;
    }
    data = player_data_parse(sj_object_get_value(json,"player"));
    if (!data)
    {
        slog("failed to parse player data");
        entity_free(ent);
        return NULL;
    }
    ent->data = data;
    //NOTE missions MUST be loaded before the planet or the station
    missions_load_from_config(sj_object_get_value(json,"missions"));
    data->world = world_load("config/world.json");
    data->station = station_new(vector3d(0,0,0),sj_object_get_value(json,"station"));
    res = sj_object_get_value(json,"planet");
    if (res)data->planet = planet_load_from_config(res);
    if (!data->planet)
    {
        data->planet = planet_new();
        planet_generate_resource_map(data->planet);
    }

    
    sj_free(json);
    
    ent->think = player_think;
    ent->update = player_update;
    ent->draw = player_draw;
    ent->free = player_free;
    
    return ent;
}

void player_free(Entity *self)
{
    PlayerData *data;
    if ((!self)||(!self->data))return;
    data = self->data;
    resources_list_free(data->resources);
    entity_free(data->station);
    world_delete(data->world);
    sj_free(data->history);
    free(data);
    player_entity = NULL;
    self->data= NULL;
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

Uint32 player_get_day()
{
    PlayerData *data;
    if (!player_entity)return 0;
    data = player_entity->data;
    if (!data)return 0;
    return data->day;
}

Uint32 player_get_hour()
{
    PlayerData *data;
    if (!player_entity)return 0;
    data = player_entity->data;
    if (!data)return 0;
    return data->hour;
}

void player_upkeep(PlayerData *player)
{
    float amount;
    int totalStaff;
    float wages,taxes;
    int food;
    StationData *station;
    if (!player)return;
    station = player_get_station_data();
    if (!station)return;
    totalStaff = player->staff + station->staffAssigned;
    wages = totalStaff * player->wages;
    resources_list_withdraw(player->resources,"credits",wages);
    message_printf("Paid out %.2fCr to station staff",wages);
    taxes = player->population * player->taxRate;
    player->resources = resources_list_give(player->resources,"credits",taxes);
    message_printf("Collected %.2fCr in taxes from the people living on the station.",taxes);
    food = (player->population/12);
    
    amount = resources_list_get_amount(player->resources,"food");
    if (amount < food)
    {
        message_new("There is not enough food to feed the people!");
        food = amount;
    }

    resources_list_withdraw(player->resources,"food",food);
    message_printf("People consumed %i tons of food this month",food);
}

void player_hour_advance()
{
    PlayerData *data;
    if (!player_entity)return;
    data = player_entity->data;
    if (!data)return;
    data->hour++;
    if (data->hour >= 24)
    {
        data->hour = 0;
        data->day++;
        resources_list_free(data->yesterday);
        data->yesterday = resources_list_duplicate(data->resources);
        if ((data->day % 7)== 0)//every 7 days is a week
        {
            player_upkeep(data);
        }
        if ((data->day % 30)== 0)//every 30 days is a month
        {
            message_new("New Month");
            resources_list_free(data->lastMonth);
            data->lastMonth = resources_list_duplicate(data->resources);
        }
        if ((data->day % 360)== 0)//every 360 days is a year
        {
            message_new("HAPPY NEW YEAR!");
            resources_list_free(data->lastYear);
            data->lastYear = resources_list_duplicate(data->resources);
        }
        station_upkeep(player_get_station_data());
        planet_facilities_update(player_get_planet());
        mission_update_all();
    }
    event_manager_update();
}


int player_get_facility_count()
{
    int count = 0;
    int i,c;
    StationData *station;
    StationSection *section;
    PlayerData *data;
    if (!player_entity)return 0;
    data = player_entity->data;
    if (data->planet)
    {
        count = gfc_list_get_count(data->planet->facilities);
    }
    station = player_get_station_data();
    if (station)
    {
        c = gfc_list_get_count(station->sections);
        for (i = 0; i < c; i++)
        {
            section = gfc_list_get_nth(station->sections,i);
            if (!section)continue;
            count += gfc_list_get_count(section->facilities);
        }
    }
    return count;
}

StationFacility *player_get_facility_nth(Uint32 index)
{
    int i,c;
    StationData *station;
    StationSection *section;
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (data->planet)
    {
        if (index < gfc_list_get_count(data->planet->facilities))
        {
            return gfc_list_get_nth(data->planet->facilities,index);
        }
        index -= gfc_list_get_count(data->planet->facilities);//remove the planety facilities from the seach
    }
    station = player_get_station_data();
    if (station)
    {
        c = gfc_list_get_count(station->sections);
        for (i = 0; i < c; i++)
        {
            section = gfc_list_get_nth(station->sections,i);
            if (!section)continue;
            if (index < gfc_list_get_count(section->facilities))
            {
                return gfc_list_get_nth(section->facilities,index);
            }
            index -= gfc_list_get_count(section->facilities);//remove the planety facilities from the seach
        }
    }
    return NULL;
}

int player_has_working_dock()
{
    int i,c;
    StationFacility *facility;
    c = player_get_facility_count();
    for (i =0; i < c; i++)
    {
        facility = player_get_facility_nth(i);
        if (!facility)continue;
        if ((strcmp(facility->facilityType,"small_docking_bay")==0)||
            (strcmp(facility->facilityType,"medium_docking_bay")==0)||
            (strcmp(facility->facilityType,"large_docking_bay")==0))
        {
            if ((!facility->inactive)&&(!facility->disabled))
            {
                return 1;
            }
        }
    }    
    return 0;
}

StationFacility *player_get_facility_by_name(const char *name)
{
    int i,c;
    StationData *station;
    StationSection *section;
    StationFacility *facility;
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!name)return NULL;
    if (data->planet)
    {
        facility = station_facility_get_by_name(data->planet->facilities, name);
        if (facility)return facility;
    }
    station = player_get_station_data();
    if (station)
    {
        c = gfc_list_get_count(station->sections);
        for (i = 0; i < c; i++)
        {
            section = gfc_list_get_nth(station->sections,i);
            if (!section)continue;
            facility = station_facility_get_by_name(section->facilities, name);
            if (facility)return facility;
        }
    }
    return NULL;
}

StationFacility *player_get_facility_by_name_id(const char *name,Uint32 id)
{
    int i,c;
    StationData *station;
    StationSection *section;
    StationFacility *facility;
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!name)return NULL;
    if (data->planet)
    {
        facility = station_facility_get_by_name_id(data->planet->facilities, name,id);
        if (facility)return facility;
    }
    station = player_get_station_data();
    if (station)
    {
        c = gfc_list_get_count(station->sections);
        for (i = 0; i < c; i++)
        {
            section = gfc_list_get_nth(station->sections,i);
            if (!section)continue;
            facility = station_facility_get_by_name_id(section->facilities, name,id);
            if (facility)return facility;
        }
    }
    return NULL;
}

PlanetData *player_get_planet()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->planet;
}

List * player_get_resources()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->resources;
}

List * player_get_stockpile()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->stockpile;
}

List * player_get_sale_price()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->salePrice;
}

List * player_get_allow_sale()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->allowSale;
}


World *player_get_world()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    return data->world;
}

PlayerData *player_get_data()
{
    if (!player_entity)return NULL;
    return player_entity->data;
}

StationData *player_get_station_data()
{
    PlayerData *data;
    if (!player_entity)return NULL;
    data = player_entity->data;
    if (!data)return NULL;
    if (!data->station)return NULL;
    return data->station->data;
}

void player_return_staff(Uint32 staff)
{
    PlayerData *data;
    if (!player_entity)return;
    data = player_entity->data;
    if (!data)return;
    data->staff += staff;
}

int player_get_new_id(const char *name)
{
    SJson *idPools,*idPool;
    int id = -1;
    if (!name)return -1;
    idPools = player_get_id_pool();
    if (!idPools)return -1;
    
    idPool = sj_object_get_value(idPools,name);
    if (!idPool)
    {
        sj_object_insert(idPools,name,sj_new_int(1));
        return 1;
    }
    sj_get_integer_value(idPool,&id);
    id++;
    sj_object_delete_key(idPools,name);
    sj_object_insert(idPools,name,sj_new_int(id));
    return id;
}

/*eol@eof*/
