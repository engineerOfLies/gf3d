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
#include "facility_menu.h"
#include "station_facility.h"

extern int freeBuildMode;

void station_facility_draw(StationFacility *facility)
{
    PlanetData *planet;
    if (!facility)return;
    if (!facility->mat.model)return;
    if (strcmp(facility->facilityType,"planetary")==0)
    {
        planet = player_get_planet();
        if (!planet)return;
        facility->mat.position = planet_position_to_position(planet->radius + 5, facility->position);
        vector3d_add(facility->mat.position,facility->mat.position,planet->mat.position);
        facility->mat.rotation = planet_position_to_rotation(facility->position);
        gf3d_model_mat_set_matrix(&facility->mat);
    }
    gf3d_model_draw(facility->mat.model,0,facility->mat.mat,vector4d(1,1,1,1),vector4d(1,1,1,1),vector4d(1,1,1,1));
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

StationFacility *station_facility_get_by_name(List *facilityList, const char *name)
{
    int i,c;
    StationFacility *facility;
    if (!facilityList)return NULL;
    if (!name)return NULL;
    c = gfc_list_get_count(facilityList);
    for (i = 0; i < c; i++)
    {
        facility = gfc_list_get_nth(facilityList,i);
        if (!facility) continue;
        if (gfc_strlcmp(facility->name,name)== 0)
        {
            return facility;
        }
    }
    return NULL;
}

StationFacility *station_facility_get_by_name_id(List *facilityList, const char *name,Uint32 id)
{
    int i,c;
    StationFacility *facility;
    if (!facilityList)return NULL;
    if (!name)return NULL;
    c = gfc_list_get_count(facilityList);
    for (i = 0; i < c; i++)
    {
        facility = gfc_list_get_nth(facilityList,i);
        if (!facility) continue;
        if (gfc_strlcmp(facility->name,name)== 0)
        {
            if (facility->id == id)
            {
                return facility;
            }
        }
    }
    return NULL;
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
    if (facility->damage < 0)
    {
        //under construction
        message_printf("Facility %s is under construction.",facility->displayName);
        facility->inactive = 1;
        facility->disabled = 1;
        return;
    }
    if (facility->damage >= 0.5)
    {
        message_printf("Facility %s is too damaged to function.",facility->displayName);
        facility->inactive = 1;
        facility->disabled = 1;
        return;
    }
    if (facility->damage > 0)
    {
        facility->productivity *= (1.0 - facility->damage);
    }
    if (facility->staffRequired)
    {
        if (facility->staffAssigned < facility->staffRequired)
        {
            message_printf("Facility %s requires %i staff to function.",facility->displayName,facility->staffRequired);
            facility->inactive = 1;
            facility->disabled = 1;
            return;
        }
        if (facility->staffAssigned < facility->staffPositions)
        {
            facility->productivity *= ((float)facility->staffAssigned / (float)facility->staffPositions);
        }
    }
    if (facility->upkeep)
    {
        if (!resources_list_afford(supply, facility->upkeep))
        {
            message_printf("Facility %s: not enough resources to run",facility->displayName);
            facility->inactive = 1;
            facility->disabled = 1;
            return;
        }
    }
}

void station_facility_remove(StationFacility *facility)
{
    Window *win;
    PlanetData *planet;
    StationSection *section;
    if (!facility)return;
    player_return_staff(facility->staffAssigned);

    section = player_get_section_by_facility(facility);
    if (section)
    {
        //station section
        gfc_list_delete_data(section->facilities,facility);
        station_facility_free(facility);
        win = gf2d_window_get_by_name("station_facility_menu");
        if (win)facility_menu_set_list(win);
        return;
    }
    planet = player_get_planet();
    if (planet)
    {
        if (gfc_list_get_item_index(planet->facilities,facility) >= 0)
        {
            gfc_list_delete_data(planet->facilities,facility);
            slog("planet facility %s removed",facility->displayName);
            station_facility_free(facility);
            win = gf2d_window_get_by_name("station_facility_menu");
            if (win)facility_menu_set_list(win);
            return;
        }
    }
    // any other place where a facility may be
    station_facility_free(facility);//couldn't find it anywhere, but still delete it
    win = gf2d_window_get_by_name("station_facility_menu");
    if (win)facility_menu_set_list(win);
}

Uint32 station_facility_get_build_time(const char *name)
{
    SJson *def;
    Uint32 workTime = 0;
    def = config_def_get_by_name("facilities",name);
    if (!def)return 0;
    sj_object_get_value_as_uint32(def,"buildTime",&workTime);
    return workTime;
}

Uint32 station_facility_get_work_time(const char *name)
{
    SJson *def;
    Uint32 workTime = 0;
    def = config_def_get_by_name("facilities",name);
    if (!def)return 0;
    sj_object_get_value_as_uint32(def,"workTime",&workTime);
    return workTime;
}

void station_facility_market_update()
{
    int i,c;
    float value;
    int stockpile = 0;
    int supply = 0;
    int nodock = 0;
    Resource *resource;
    PlayerData *player;
    //todo make it based on galactic faction
    player = player_get_data();
    if (!player)return;
    //check if there is a working port
    if (!player_has_working_dock())
    {
        nodock = 1;
    }
    
    c = gfc_list_get_count(player->allowSale);
    for (i =0; i < c; i++)
    {
        resource = gfc_list_get_nth(player->allowSale,i);
        if (!resource)continue;
        if (resource->amount <= 0)continue;
        if (nodock)
        {
            message_printf("No working dock, cannot sell goods on the open market");
            return;
        }
        stockpile = resources_list_get_amount(player->stockpile,resource->name);
        supply = resources_list_get_amount(player->resources,resource->name);
        value = resources_list_get_amount(player->salePrice,resource->name);
        if (supply < (stockpile + 100))continue;
        // where I find the best buyer, for now just go with market price
        resources_list_withdraw(player->resources,resource->name,100);
        resources_list_give(player->resources,"credits",100 * value);
        message_printf("Sold %i of %s for %0.2f",100,resources_get_display_name(resource->name),100 * value);
    }
}

void station_facility_update(StationFacility *facility,float *energySupply)
{
    int newMass;
    int space;
    SiteData *site;
    Uint32 workTime = 0;
    StationData *station;
    const char *extract = NULL;
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
    workTime = station_facility_get_work_time(facility->name);
    if (facility->lastProduction + workTime > player_get_day())return;//haven't gotten here yet
    
    
    if (strcmp("survey_site",facility->name) == 0)
    {
        //survey site is now working
        planet_site_survey(player_get_planet(),facility->position);
        site = planet_get_site_data_by_position(player_get_planet(),facility->position);
        if (site)
        {
            message_printf("Survery complete.  Site revealed %i nutrients, %i minerals, and %i ores",site->resources[SRT_Nutrients],site->resources[SRT_Minerals],site->resources[SRT_Ores]);
        }
        player_return_staff(facility->staffAssigned);
        facility->staffAssigned = 0;
        facility->disabled = 1;
        facility->inactive = 1;
        return;
    }
    
    extract = sj_get_string_value(config_def_get_value("facilities", facility->name, "extract"));
    if (extract != NULL)
    {
        // if this is defined, then this is a planet facility that needs to extract resources to work
        if (!planet_site_extract_resource(player_get_planet(),facility->position,extract))
        {
            // resource is not at the site, cancel production
            facility->disabled = 1;
            message_printf("Production at %s has halted, no more %s to extract!",facility->displayName,extract);
            return;
        }
    }
        
    facility->lastProduction = player_get_day();
    if (strcmp(facility->name,"commodities_market")==0)
    {   //override for the market sale
        station_facility_market_update();
        return;
    }
    
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
        if (!facility->disabled)
        {
            resource_list_sell(supply, facility->produces,facility->productivity);
        }
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
    sj_object_insert(json,"displayName",sj_new_str(facility->displayName));
    sj_object_insert(json,"id",sj_new_int(facility->id));
    sj_object_insert(json,"position",sj_vector2d_new(facility->position));
    sj_object_insert(json,"damage",sj_new_float(facility->damage));
    sj_object_insert(json,"staff",sj_new_int(facility->staffAssigned));
    sj_object_insert(json,"disabled",sj_new_bool(facility->disabled));
    sj_object_insert(json,"working",sj_new_bool(facility->working));
    if (strlen(facility->officer))
    {
        sj_object_insert(json,"officer",sj_new_str(facility->officer));
    }
    if (facility->mission)
    {
        sj_object_insert(json,"mission",sj_new_uint32(facility->mission->id));
    }
    return json;
}

StationFacility *station_facility_load(SJson *config)
{
    Uint32 missionId = -1;
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
    str = sj_object_get_value_as_string(config,"displayName");
    if (str)
    {
        gfc_line_cpy(facility->displayName,str);
    }
    else
    {
        str = station_facility_get_display_name(facility->name);
        if (str)
        {
            gfc_line_sprintf(facility->displayName,"%s %i",str,facility->id);
        }
        else slog("failed to get display name for facility %s",facility->name);
    }

    sj_value_as_vector2d(sj_object_get_value(config,"position"),&facility->position);
    sj_object_get_value_as_float(config,"damage",&facility->damage);
    sj_object_get_value_as_int(config,"staff",&facility->staffAssigned);
    sj_object_get_value_as_bool(config,"disabled",(short int*)&facility->disabled);
    sj_object_get_value_as_bool(config,"working",(short int*)&facility->working);
    str = sj_object_get_value_as_string(config,"officer");
    if (str)gfc_line_cpy(facility->officer,str);
    if (sj_object_get_value_as_uint32(config,"mission",&missionId))
    {
        facility->mission = mission_get_by_id(missionId);
    }
    return facility;
}

int station_facility_is_singleton(const char *name)
{
    Bool value;
    SJson *facilityDef;
    if (!name)
    {
        slog("no name provided");
        return 0;
    }
    facilityDef = config_def_get_by_name("facilities",name);
    if (!facilityDef)
    {
        slog("facility %s not found",name);
        return 0;
    }
    if ((sj_object_get_value_as_bool(facilityDef,"singleton",&value))&&(value))return 1;
    return 0;
}

void station_facility_build(const char *name,Vector2D position,List *parentList,Uint32 staff,int buildTime)
{
    TextLine buffer;
    StationFacility *new_facility;
    List *cost;
    
    new_facility = station_facility_new_by_name(name,-1);
    vector2d_copy(new_facility->position,position);
    gfc_list_append(parentList,new_facility);
    if (!freeBuildMode)
    {
        cost = station_facility_get_resource_cost(name,"cost");
        resource_list_buy(player_get_resources(), cost);
        new_facility->working = 1;
        new_facility->disabled = 1;
        new_facility->inactive = 1;
        new_facility->damage = -1;
        gfc_line_sprintf(buffer,"%i",new_facility->id);
        new_facility->mission = mission_begin(
            "Facility Construction",
            NULL,
            "build",
            "facility",
            station_facility_get_display_name(new_facility->name),
            new_facility->id,
            player_get_day(),
            buildTime,
            staff);
    }
    facility_menu_set_list(gf2d_window_get_by_name("facility_menu"));
    resources_list_free(cost);
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

    sj_object_get_value_as_int(facilityDef,"storage",&facility->storage);
    sj_object_get_value_as_int(facilityDef,"housing",&facility->housing);
    sj_object_get_value_as_float(facilityDef,"crimeRate",&facility->crimeRate);
    sj_object_get_value_as_float(facilityDef,"opportunities",&facility->opportunities);
    sj_object_get_value_as_float(facilityDef,"commerce",&facility->commerce);
    sj_object_get_value_as_float(facilityDef,"entertainment",&facility->entertainment);

    sj_object_get_value_as_int(facilityDef,"staffRequired",&facility->staffRequired);
    sj_object_get_value_as_int(facilityDef,"staffPositions",&facility->staffPositions);
    sj_object_get_value_as_int(facilityDef,"energyDraw",&facility->energyDraw);
    sj_object_get_value_as_int(facilityDef,"energyOutput",&facility->energyOutput);
    if (id < 0)
    {
        facility->id = player_get_new_id(name);
    }
    else facility->id = id;
    str = sj_object_get_value_as_string(facilityDef,"displayName");
    if (str)gfc_line_sprintf(facility->displayName,"%s %i",str,facility->id);
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

int station_facility_supports_officer(const char *name)
{
    Bool officer = 0;
    SJson *facilityDef;
    facilityDef = config_def_get_by_name("facilities",name);
    if (!facilityDef)return 0;
    sj_object_get_value_as_bool(facilityDef,"officerSlot",&officer);
    return officer;
}

List *station_facility_get_resource_cost(const char *name,const char *resource_type)
{
    SJson *stationDef;
    if (!name)return NULL;
    stationDef = config_def_get_by_name("facilities",name);
    if (!stationDef)return NULL;
    return resources_list_parse(sj_object_get_value(stationDef,resource_type));
}

void station_facility_repair(StationFacility *facility)
{
    Window *win;
    if (!facility)return;
    message_printf("Facility %s has been repaired!",facility->displayName);
    facility->working = 0;
    facility->damage = 0;
    facility->mission = NULL;
    win = gf2d_window_get_by_name("station_facility_menu");
    if (win)
    {
        facility_menu_select_item(win,-1);
    }

}



/*eol@eof*/
