#include "simple_logger.h"
#include "config_def.h"
#include "ship_facility.h"
#include "ship.h"

Ship *ship_new()
{
    Ship *ship = gfc_allocate_array(sizeof(ship),1);
    if (!ship)return NULL;
    
    ship->facilities = gfc_list_new();
    return ship;
}

void ship_free(Ship *ship)
{
    if (!ship)return;
    gfc_list_foreach(ship->facilities,(gfc_work_func*)ship_facility_free);
    gfc_list_delete(ship->facilities);
    free(ship);
}

Ship *ship_load(SJson *json)
{
    Uint32 id = 0;
    int i,c;
    const char *str;
    Ship *ship;
    ShipFacility *facility;
    SJson *list,*item;
    if (!json)return NULL;
    str = sj_object_get_value_as_string(json,"name");
    if (!str)return NULL;
    sj_object_get_value_as_uint32(json,"id",&id);
    ship = ship_new_by_name(str,id,0);
    if (!ship)return NULL;
    str = sj_object_get_value_as_string(json,"location");
    if (str)gfc_line_cpy(ship->location,str);
    sj_value_as_vector3d(sj_object_get_value(json,"position"),&ship->position);
    sj_object_get_value_as_uint32(json,"idPool",&ship->idPool);
    sj_object_get_value_as_float(json,"hull",&ship->hull);
    if (sj_object_get_value_as_uint32(json,"mission",&id))
    {
        ship->mission = mission_get_by_id(id);
    }
    sj_object_get_value_as_bool(json,"working",&ship->working);
    list = sj_object_get_value(json,"facilities");
    c = sj_array_get_count(list);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        facility = ship_facility_load(item);
        if (!facility)continue;
        gfc_list_append(ship->facilities,facility);
    }
    return ship;
}

SJson *ship_save(Ship *ship)
{
    int i,c;
    ShipFacility *facility;
    SJson *json,*array;
    if (!ship)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
   
    sj_object_insert(json,"name",sj_new_str(ship->name));
    sj_object_insert(json,"displayName",sj_new_str(ship->displayName));
    sj_object_insert(json,"id",sj_new_uint32(ship->id));
    sj_object_insert(json,"idPool",sj_new_uint32(ship->idPool));
    sj_object_insert(json,"location",sj_new_str(ship->location));
    sj_object_insert(json,"position",sj_vector3d_new(ship->position));
    sj_object_insert(json,"hull",sj_new_float(ship->hull));
    if (ship->mission)
    {
        sj_object_insert(json,"mission",sj_new_uint32(ship->mission->id));
    }
    sj_object_insert(json,"working",sj_new_bool(ship->working));
    array = sj_array_new();
    c = gfc_list_get_count(ship->facilities);
    for (i = 0;i < c; i++)
    {
        facility = gfc_list_get_nth(ship->facilities,i);
        if (!facility)continue;
        sj_array_append(array,ship_facility_save(facility));
    }
    sj_object_insert(json,"facilities",array);
    return json;
}

Ship *ship_new_by_name(const char *name,int id,int defaults)
{
    const char *str;
    Ship *ship;
    int i,c;
    SJson *def,*list,*item;
    if (!name)return NULL;
    def = config_def_get_by_name("ships",name);
    if (!def)return NULL;
    ship = ship_new();
    if (!ship)return NULL;
    gfc_line_cpy(ship->name,name);
    
    str = sj_object_get_value_as_string(def,"displayName");
    if (str)gfc_line_sprintf(ship->displayName,"%s %i",str,id);
    
    ship->id = id;
    sj_object_get_value_as_float(def,"hull",&ship->hull);
    sj_object_get_value_as_float(def,"hullMax",&ship->hullMax);
    
    if (defaults)
    {
        list = sj_object_get_value(def,"default_facilities");
        c = sj_array_get_count(list);
        for (i = 0; i < c; i++)
        {
            item = sj_array_get_nth(list,i);
            if (!item)continue;
            str = sj_get_string_value(item);
            if (!str)continue;
            gfc_list_append(ship->facilities,ship_facility_new_by_name(str,++ship->idPool));
        }
    }
    return ship;
}

/*eol@eof*/
