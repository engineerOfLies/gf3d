#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "gf3d_draw.h"

#include "config_def.h"
#include "station_def.h"
#include "resources.h"
#include "station.h"
#include "station_facility.h"

int station_facility_types_valid(SJson *array,const char *check)
{
    int i,c;
    const char *str;
    SJson *item;
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

List *station_facility_get_possible_list(StationSection *parent)
{
    int i,c;
    List *list;
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
        facility_type = sj_object_get_value_as_string(def,"type");
        if (station_facility_types_valid(facility_types,facility_type))
        {
            name = sj_object_get_value_as_string(def,"name");
            list = gfc_list_append(list,(void *)name);
        }
    }
    return list;
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
    sj_object_insert(json,"staff",sj_new_int(facility->staffAssigned));
    sj_object_insert(json,"inactive",sj_new_bool(facility->inactive));
    sj_object_insert(json,"disabled",sj_new_bool(facility->disabled));
    if (strlen(facility->officer))
    {
        sj_object_insert(json,"officer",sj_new_str(facility->officer));
    }
    return json;
}

StationFacility *station_facility_load(SJson *config)
{
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
    facility = station_facility_new_by_name(str);
    if (!facility)
    {
        slog("failed to make facility %s",str);
        return NULL;
    }
    sj_object_get_value_as_int(config,"staff",&facility->staffAssigned);
    sj_object_get_value_as_int(config,"storage",&facility->storage);
    sj_object_get_value_as_bool(config,"inactive",(short int*)&facility->inactive);
    sj_object_get_value_as_bool(config,"disabled",(short int*)&facility->disabled);
    str = sj_object_get_value_as_string(config,"officer");
    if (str)gfc_line_cpy(facility->officer,str);
    return facility;
}

StationFacility *station_facility_new_by_name(const char *name)
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
    str = sj_object_get_value_as_string(facilityDef,"name");
    if (str)gfc_line_cpy(facility->name,str);
    str = sj_object_get_value_as_string(facilityDef,"type");
    if (str)gfc_line_cpy(facility->facilityType,str);
    res = sj_object_get_value(facilityDef,"produces");
    if (res)facility->produces = resources_list_parse(res);
    res = sj_object_get_value(facilityDef,"upkeep");
    if (res)facility->upkeep = resources_list_parse(res);
    sj_object_get_value_as_int(facilityDef,"staff",&facility->staffRequired);
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

List *station_facility_get_resource_cost(const char *name)
{
    SJson *stationDef;
    if (!name)return NULL;
    stationDef = config_def_get_by_name("facilities",name);
    if (!stationDef)return NULL;
    return resources_list_parse(sj_object_get_value(stationDef,"cost"));
}


/*eol@eof*/
