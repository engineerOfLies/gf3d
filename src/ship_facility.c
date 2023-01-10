#include "simple_logger.h"
#include "config_def.h"
#include "ship_facility.h"

ShipFacility *ship_facility_new()
{
    return gfc_allocate_array(sizeof(ShipFacility),1);
}

SJson *ship_facility_save(ShipFacility *facility)
{
    SJson *json;
    if (!facility)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
    sj_object_insert(json,"name",sj_new_str(facility->name));
    sj_object_insert(json,"displayName",sj_new_str(facility->displayName));
    sj_object_insert(json,"id",sj_new_int(facility->id));
    sj_object_insert(json,"damage",sj_new_float(facility->damage));
    sj_object_insert(json,"inactive",sj_new_int(facility->inactive));
    return json;
}

ShipFacility *ship_facility_load(SJson *json)
{
    int id = -1;
    const char *str;
    ShipFacility *facility;
    if (!json)return NULL;
    str = sj_object_get_value_as_string(json,"name");
    if (!str)return NULL;
    sj_object_get_value_as_int(json,"id",&id);
    facility = ship_facility_new_by_name(str,id);
    if (!facility)return NULL;
    sj_object_get_value_as_float(json,"damage",&facility->damage);
    sj_object_get_value_as_int(json,"inactive",&facility->inactive);
    return facility;
}


void ship_facility_free(ShipFacility *facility)
{
    if (!facility)return;
    free(facility);
}

ShipFacility *ship_facility_new_by_name(const char *name,int id)
{
    const char *str;
    ShipFacility *facility;
    SJson *def;
    def = config_def_get_by_name("ship_facilities",name);
    if (!def)return NULL;
    facility = ship_facility_new();
    if (!facility)return NULL;
    gfc_line_cpy(facility->name,name);
    
    str = sj_object_get_value_as_string(def,"displayName");
    if (str)gfc_line_sprintf(facility->displayName,"%s %i",str,id);
    facility->id = id;
    str = sj_object_get_value_as_string(def,"slot_type");
    if (str)gfc_line_cpy(facility->slot_type,str);
    sj_object_get_value_as_int(def,"housing",&facility->housing);
    sj_object_get_value_as_int(def,"cargo",&facility->storage);
    sj_object_get_value_as_int(def,"staffRequired",&facility->staffRequired);
    sj_object_get_value_as_int(def,"staffPositions",&facility->staffPositions);
    sj_object_get_value_as_int(def,"energyDraw",&facility->energyDraw);
    sj_object_get_value_as_int(def,"energyOutput",&facility->energyOutput);    
    sj_object_get_value_as_int(def,"speed",&facility->speed);    
    return facility;
}

List *ship_facility_get_def_list_by_types(const char *facilityType)
{
    int i,c;
    const char *str;
    SJson *def;
    List *typeList = gfc_list_new();
    c = config_def_get_resource_count("ship_facilities");
    for (i = 0; i < c; i++)
    {
        def = config_def_get_by_index("ship_facilities",i);
        if (!def)continue;
        str = sj_object_get_value_as_string(def,"slot_type");
        if (!str)continue;
        if (gfc_strlcmp(str,"faiclityType")==0)
        {
            str = sj_object_get_value_as_string(def,"name");
            gfc_list_append(typeList,(void *)str);
        }
    }
    return typeList;
}

/*eol@eof*/
