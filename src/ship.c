#include "simple_logger.h"
#include "config_def.h"
#include "world.h"
#include "resources.h"
#include "ship_entity.h"
#include "ship_facility.h"
#include "ship.h"

Ship *ship_new()
{
    Ship *ship = gfc_allocate_array(sizeof(Ship),1);
    if (!ship)return NULL;
    
    ship->facilities = gfc_list_new();
    return ship;
}

void ship_free(Ship *ship)
{
    if (!ship)return;
    gfc_list_foreach(ship->facilities,(gfc_work_func*)ship_facility_free);
    gfc_list_delete(ship->facilities);
    if (ship->entity)gf3d_entity_free(ship->entity);
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
    str = sj_object_get_value_as_string(json,"displayName");
    if (str)gfc_line_cpy(ship->displayName,str);
    str = sj_object_get_value_as_string(json,"captain");
    if (str)gfc_line_cpy(ship->captain,str);
    str = sj_object_get_value_as_string(json,"location");
    if (str)gfc_line_cpy(ship->location,str);
    sj_value_as_vector3d(sj_object_get_value(json,"position"),&ship->position);
    sj_object_get_value_as_uint32(json,"idPool",&ship->idPool);
    sj_object_get_value_as_int(json,"passengers",&ship->passengers);
    sj_object_get_value_as_float(json,"hull",&ship->hull);
    sj_object_get_value_as_int(json,"staffAssigned",&ship->staffAssigned);    
    item = sj_object_get_value(json,"cargo");
    if (item)
    {
        ship->cargo = resources_list_parse(item);
    }
    else
    {
        ship->cargo = gfc_list_new();//empty
    }
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
    if (strcmp(ship->location,"parking")==0)
    {
        //register parking spot
        ship->position = world_parking_claim_spot(ship->position);
    }
    ship_check(ship);
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
    sj_object_insert(json,"captain",sj_new_str(ship->captain));
    sj_object_insert(json,"id",sj_new_uint32(ship->id));
    sj_object_insert(json,"idPool",sj_new_uint32(ship->idPool));
    sj_object_insert(json,"passengers",sj_new_int(ship->passengers));
    sj_object_insert(json,"location",sj_new_str(ship->location));
    sj_object_insert(json,"position",sj_vector3d_new(ship->position));
    sj_object_insert(json,"staffAssigned",sj_new_int(ship->staffAssigned));
    sj_object_insert(json,"hull",sj_new_float(ship->hull));
    if (gfc_list_get_count(ship->cargo) > 0)
    {
        sj_object_insert(json,"cargo",resources_list_save(ship->cargo));
    }
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
    ship->hullMax = ship->hull;
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
    ship_check(ship);
    return ship;
}

void ship_check(Ship *ship)
{
    int i,c;
    ShipFacility *facility;
    if (!ship)return;
    ship->housing = 0;
    ship->energyOutput = 0;
    ship->energyDraw = 0;
    ship->storageCapacity = 0;
    ship->staffPositions = 0;
    ship->staffRequired = 0;
    ship->speed = 0;
    ship->disabled = 0;
    c = gfc_list_get_count(ship->facilities);
    for (i = 0;i < c; i++)
    {
        facility = gfc_list_get_nth(ship->facilities,i);
        if (!facility)continue;
        ship->storageCapacity += facility->storage;
        ship->staffPositions += facility->staffPositions;
        ship->staffRequired += facility->staffRequired;
        
        if ((facility->damage > 0.5)||(facility->inactive))continue;//the rest only for 
        ship->housing += facility->housing;
        ship->energyOutput += facility->energyOutput;
        ship->energyDraw += facility->energyDraw;
        ship->speed += facility->speed;
    }
    if (ship->staffAssigned < ship->staffRequired)ship->disabled = 1;
    if (ship->energyDraw > ship->energyOutput)ship->disabled = 1;
    if (ship->speed <= 0)ship->disabled = 1;
    if (ship->disabled)ship->efficiency = 0;
    else
    {
        ship->efficiency = ship->staffAssigned/(float)MIN(1,ship->staffPositions);
    }
}

void ship_set_location(Ship *ship,const char *location,Vector3D position)
{
    if (!ship)return;
    if (location)gfc_line_cpy(ship->location,location);
    vector3d_copy(ship->position,position);
    if (ship->entity)
    {
        vector3d_copy(ship->entity->mat.position,position);
    }
}

int ship_change_staff(Ship *ship,int amount)
{
    int diff;
    if (!ship)return 0;
    ship->staffAssigned += amount;
    if (ship->staffAssigned > ship->staffPositions)
    {
        diff = ship->staffAssigned - ship->staffPositions;
        ship->staffAssigned = ship->staffPositions;
        return diff;//we had extra
    }
    if (ship->staffAssigned < 0)
    {
        diff = ship->staffAssigned;
        ship->staffAssigned = 0;
        return diff;
    }
    ship_check(ship);
    return 0;
}

/*eol@eof*/
