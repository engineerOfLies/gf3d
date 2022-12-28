#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "gf2d_message_buffer.h"

#include "gf3d_draw.h"

#include "config_def.h"
#include "station_def.h"
#include "resources.h"
#include "player.h"
#include "station.h"
#include "station_facility.h"


void station_facility_draw(StationFacility *facility)
{
    PlanetData *planet;
    if (!facility)return;
    if (!facility->mat.model)return;
    if (strcmp(facility->facilityType,"planetary")==0)
    {
        planet = player_get_planet();
        if (!planet)return;
        facility->mat.position = planet_position_to_position(planet->radius + 2, facility->position);
        vector3d_add(facility->mat.position,facility->mat.position,planet->mat.position);
        facility->mat.rotation = planet_position_to_rotation(facility->position);
        gf3d_model_mat_set_matrix(&facility->mat);
    }
    gf3d_model_draw(facility->mat.model,0,facility->mat.mat,vector4d(1,1,1,1),vector4d(1,1,1,1));
}

void station_facility_draw_highlight(StationFacility *facility)
{
    PlanetData *planet;
    if (!facility)return;
    if (!facility->mat.model)return;
    if (strcmp(facility->facilityType,"planetary")==0)
    {
        planet = player_get_planet();
        if (!planet)return;
        facility->mat.position = planet_position_to_position(planet->radius + 2, facility->position);
        vector3d_add(facility->mat.position,facility->mat.position,planet->mat.position);
        facility->mat.rotation = planet_position_to_rotation(facility->position);
        gf3d_model_mat_set_matrix(&facility->mat);
    }
    gf3d_model_draw_highlight(facility->mat.model,0,facility->mat.mat,vector4d(1,.7,0.1,1));
}



int station_facility_types_valid(SJson *array,const char *check)
{
    int i,c;
    const char *str;
    SJson *item;
    if (!check)return 0;
    c = sj_array_get_count(array);
    for (i = 0; i < c;i++)
    {
        item = sj_array_get_nth(array,i);
        if (!item)continue;
        str = sj_get_string_value(item);
        if (strcmp(str,check)==0)return 1;// yes this is valid
    }
    return 0;
}

int station_facility_is_unique(StationFacility *facility)
{
    Bool unique;
    SJson *def;
    if (!facility)return 0;
    def = config_def_get_by_name("facilities",facility->name);
    if (!def)return 0;
    if ((sj_object_get_value_as_bool(def,"unique",&unique))&&(unique))return 1;
    return 0;
}


StationFacility *station_facility_get_by_position(List *list,Vector2D position)
{
    int i,c;
    StationFacility *facility;
    if (!list)
    {
        slog("no list provided");
        return NULL;
    }
    c = gfc_list_get_count(list);
    for (i = 0; i < c; i++)
    {
        facility = gfc_list_get_nth(list,i);
        if (!facility)continue;
        if (((int)facility->position.x == (int)position.x)&&((int)facility->position.y == (int)position.y))
            return facility;
    }
    return NULL;
}

int station_facility_types_in_list(List *list, const char *facility_type)
{
    int i,c;
    const char *str;
    if ((!list)||(!facility_type))return 0;
    c = gfc_list_get_count(list);
    for (i = 0; i < c; i++)
    {
        str = gfc_list_get_nth(list,i);
        if (!str)continue;
        if (strcmp(str,facility_type)==0)return 1;
    }
    return 0;
}

List *station_facility_get_possible_from_list(List *typeList)
{
    int i,c;
    List *list;
    Bool unique;
    const char *name;
    const char *facility_type;
    SJson *def;
    if (!typeList)return NULL;
    list = gfc_list_new();
    c = config_def_get_resource_count("facilities");
    for (i = 0; i < c; i++)
    {
        def = config_def_get_by_index("facilities",i);
        if (!def)continue;
        if ((sj_object_get_value_as_bool(def,"unique",&unique))&&(unique))continue;
        facility_type = sj_object_get_value_as_string(def,"type");
        if (station_facility_types_in_list(typeList,facility_type))
        {
            name = sj_object_get_value_as_string(def,"name");
            list = gfc_list_append(list,(void *)name);
        }
    }
    return list;
}

List *station_facility_get_possible_list(StationSection *parent)
{
    int i,c;
    List *list;
    Bool unique;
    const char *name;
    const char *facility_type;
    SJson *facility_types,*def;
    if (!parent)return NULL;
    def = config_def_get_by_name("sections",parent->name);
    if (!def)return NULL;
    facility_types = sj_object_get_value(def,"facility_types");
    if (!facility_types)return NULL;
    list = gfc_list_new();
    c = config_def_get_resource_count("facilities");
    for (i = 0; i < c; i++)
    {
        def = config_def_get_by_index("facilities",i);
        if (!def)continue;
        if ((sj_object_get_value_as_bool(def,"unique",&unique))&&(unique))continue;
        facility_type = sj_object_get_value_as_string(def,"type");
        if (station_facility_types_valid(facility_types,facility_type))
        {
            name = sj_object_get_value_as_string(def,"name");
            list = gfc_list_append(list,(void *)name);
        }
    }
    return list;
}

int station_facility_change_staff(StationFacility *facility,int amount)
{
    if (!facility)return 0;
    facility->staffAssigned += amount;
    if (facility->staffAssigned > facility->staffPositions)
    {
        facility->staffAssigned = facility->staffPositions;
        return 1;//we had extra
    }
    if (facility->staffAssigned < facility->staffRequired)facility->inactive = 1;
    if (facility->staffAssigned < 0)
    {
        return -1;
        facility->staffAssigned = 0;
    }
    return 0;
}

void station_facility_check(StationFacility *facility)
{
    List *supply;
    if (!facility)return;
    supply = player_get_resources();
    if (!supply)return;
    if (facility->disabled)
    {
        facility->productivity = 0;
        facility->inactive = 1;
        return;
    }
    facility->productivity = 1.0;
    facility->inactive = 0;//default to okay, but any failed test can make it inactive
    if (facility->damage >= 0.5)
    {
        message_printf("Facility %s is too damaged to function.",facility->name);
        facility->inactive = 1;
        facility->disabled = 1;
        return;
    }
    facility->productivity *= (1 - facility->damage);
    if (facility->staffRequired)
    {
        if (facility->staffAssigned < facility->staffRequired)
        {
            message_printf("Facility %s requires %i staff to function.",facility->name,facility->staffRequired);
            facility->inactive = 1;
            facility->disabled = 1;
            return;
        }
        facility->productivity *= ((float)facility->staffAssigned / (float)facility->staffPositions);
    }
    if (facility->upkeep)
    {
        if (!resources_list_afford(supply, facility->upkeep))
        {
            message_printf("Facility %s: not enough resources to run",facility->name);
            facility->inactive = 1;
            facility->disabled = 1;
            return;
        }
    }
}

void station_facility_update(StationFacility *facility,float *energySupply)
{
    int newMass;
    int space;
    StationData *station;
    List *supply;
    if (!facility)return;
    if (facility->disabled)return;// not updating what has been turned off by the player
    station = player_get_station_data();
    if (!station)return;
    
    if (facility->energyDraw)
    {
        if (!energySupply)return;
        if (*energySupply < facility->energyDraw)//do we have energy for this?
        {
            facility->inactive = 1;
            message_printf("Facility %s does not have enough energy.",facility->name);
            facility->disabled = 1;
            return;
        }
        *energySupply -= facility->energyDraw;
    }
    
    station_facility_check(facility);
    if (facility->inactive)return;
    
    supply = player_get_resources();
    if (facility->upkeep)
    {
        resource_list_buy(supply, facility->upkeep);
    }
    if (gfc_list_get_count(facility->produces)>0)
    {
        newMass = resources_get_total_commodity_mass(facility->produces) * facility->productivity;
        if (newMass <= 0)
        {
            message_printf("Facility %s cannot produce any more, disabling...",facility->name);
            facility->disabled = 1;
            return;
        }
        space = station->storageCapacity - resources_get_total_commodity_mass(supply);
        if (space <= 0)
        {
            message_printf("Facility %s cannot store any more of its produce, disabling...",facility->name);
            facility->disabled = 1;
            return;
        }
        if (space < newMass)
        {
            message_printf("Facility %s cannot store any more of its produce, disabling...",facility->name);
            facility->disabled = 1;
            resource_list_sell(supply, facility->produces,facility->productivity * ((float)space / (float)newMass));
        }
        resource_list_sell(supply, facility->produces,facility->productivity);
    }
}

void station_facility_free(StationFacility *facility)
{
    if (!facility)return;
    resources_list_free(facility->upkeep);
    resources_list_free(facility->produces);
    free(facility);
}

SJson *station_facility_save(StationFacility *facility)
{
    SJson *json;
    if (!facility)return NULL;
    json = sj_object_new();
    sj_object_insert(json,"name",sj_new_str(facility->name));
    sj_object_insert(json,"id",sj_new_int(facility->id));
    sj_object_insert(json,"position",sj_vector2d_new(facility->position));
    sj_object_insert(json,"damage",sj_new_float(facility->damage));
    sj_object_insert(json,"staff",sj_new_int(facility->staffAssigned));
    sj_object_insert(json,"disabled",sj_new_bool(facility->disabled));
    if (strlen(facility->officer))
    {
        sj_object_insert(json,"officer",sj_new_str(facility->officer));
    }
    return json;
}

StationFacility *station_facility_load(SJson *config)
{
    int id = -1;
    const char *str;
    StationFacility *facility;
    if (!config)
    {
        slog("no config provided");
        return NULL;
    }
    str = sj_object_get_value_as_string(config,"name");
    if (!str)
    {
        slog("facility has no name");
        return NULL;
    }
    sj_object_get_value_as_int(config,"id",&id);
    facility = station_facility_new_by_name(str,id);
    if (!facility)
    {
        slog("failed to make facility %s",str);
        return NULL;
    }
    sj_value_as_vector2d(sj_object_get_value(config,"position"),&facility->position);
    sj_object_get_value_as_float(config,"damage",&facility->damage);
    sj_object_get_value_as_int(config,"staff",&facility->staffAssigned);
    sj_object_get_value_as_bool(config,"disabled",(short int*)&facility->disabled);
    str = sj_object_get_value_as_string(config,"officer");
    if (str)gfc_line_cpy(facility->officer,str);
    return facility;
}

StationFacility *station_facility_new_by_name(const char *name,int id)
{
    const char *str;
    StationFacility *facility;
    SJson *facilityDef,*res;
    if (!name)
    {
        slog("no name provided");
        return NULL;
    }
    facilityDef = config_def_get_by_name("facilities",name);
    if (!facilityDef)
    {
        slog("facility %s not found",name);
        return NULL;
    }
    facility = station_facility_new();
    if (!facility)
    {
        return NULL;
    }
    str = sj_object_get_value_as_string(facilityDef,"model");
    if (str)
    {
        gf3d_model_mat_reset(&facility->mat);
        facility->mat.model = gf3d_model_load(str);
        vector3d_set(facility->mat.scale,50,50,50);
    }

    str = sj_object_get_value_as_string(facilityDef,"name");
    if (str)gfc_line_cpy(facility->name,str);
    str = sj_object_get_value_as_string(facilityDef,"type");
    if (str)gfc_line_cpy(facility->facilityType,str);
    res = sj_object_get_value(facilityDef,"produces");
    if (res)facility->produces = resources_list_parse(res);
    res = sj_object_get_value(facilityDef,"upkeep");
    if (res)facility->upkeep = resources_list_parse(res);
    sj_object_get_value_as_int(facilityDef,"housing",&facility->housing);
    sj_object_get_value_as_int(facilityDef,"storage",&facility->storage);
    sj_object_get_value_as_int(facilityDef,"staffRequired",&facility->staffRequired);
    sj_object_get_value_as_int(facilityDef,"staffPositions",&facility->staffPositions);
    sj_object_get_value_as_int(facilityDef,"energyDraw",&facility->energyDraw);
    sj_object_get_value_as_int(facilityDef,"energyOutput",&facility->energyOutput);
    if (id < 0)
    {
        facility->id = player_get_new_id(name);
    }
    else facility->id = id;
    return facility;
}

void station_facility_free_list(List *list)
{
    if (!list)return;
    gfc_list_foreach(list,(void (*)(void *))station_facility_free);
    gfc_list_delete(list);
}

StationFacility *station_facility_new()
{
    StationFacility *facility;
    facility = gfc_allocate_array(sizeof(StationFacility),1);
    if (!facility)return NULL;
    facility->upkeep = resources_list_new();
    facility->produces = resources_list_new();
    return facility;
}

const char *station_facility_get_name_from_display(const char *display)
{
    SJson *facilityDef;
    facilityDef = config_def_get_by_parameter("facilities","displayName",display);
    if (!facilityDef)return NULL;
    return sj_object_get_value_as_string(facilityDef,"name");
}

const char *station_facility_get_display_name(const char *name)
{
    SJson *facilityDef;
    facilityDef = config_def_get_by_name("facilities",name);
    if (!facilityDef)return NULL;
    return sj_object_get_value_as_string(facilityDef,"displayName");
}

List *station_facility_get_resource_cost(const char *name,const char *resource_type)
{
    SJson *stationDef;
    if (!name)return NULL;
    stationDef = config_def_get_by_name("facilities",name);
    if (!stationDef)return NULL;
    return resources_list_parse(sj_object_get_value(stationDef,resource_type));
}



/*eol@eof*/
