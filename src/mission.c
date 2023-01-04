#include "simple_logger.h"

#include "gf2d_message_buffer.h"

#include "player.h"
#include "station_def.h"
#include "station_menu.h"
#include "resources.h"
#include "facility_menu.h"
#include "mission.h"


typedef struct
{
    List *mission_list;
    Uint32 missionIdPool;
}MissionManager;

static MissionManager mission_manager = {0};

void mission_close()
{
    if (mission_manager.mission_list != NULL)
    {
        gfc_list_foreach(mission_manager.mission_list,(void (*)(void *))free);
        gfc_list_delete(mission_manager.mission_list);
    }
    memset(&mission_manager,0,sizeof(MissionManager));
}

void mission_init()
{
    mission_manager.mission_list = gfc_list_new();
    atexit(mission_close);
}

void mission_free(Mission *mission)
{
    if (!mission)return;
    
    free(mission);
}

Mission *mission_get_by_id(Uint32 id)
{
    Mission *mission;
    int i,c;
    c = gfc_list_get_count(mission_manager.mission_list);
    for (i = c - 1; i  >= 0; i--)
    {
        mission = gfc_list_get_nth(mission_manager.mission_list,i);
        if (!mission)continue;
        if (mission->id == id)return mission;
    }
    return NULL;
}

Mission *mission_new()
{
    Mission *mission;
    mission = gfc_allocate_array(sizeof(Mission),1);
    if (!mission)return NULL;
    return mission;
}

void mission_cancel(Mission *mission)
{
    if (!mission)return;
    player_return_staff(mission->staff);
    mission_free(mission);
}

Mission *mission_begin(
    const char *missionTitle,
    const char *missionType,
    const char *missionSubject,
    const char *missionTarget,
    Uint32 dayStart,
    Uint32 dayFinished,
    Uint32 staff)
{
    Mission *mission;
    mission = mission_new();
    if (!mission)return NULL;
    
    mission->id = mission_manager.missionIdPool++;
    if (missionTitle)gfc_line_cpy(mission->title,missionTitle);
    if (missionType)gfc_line_cpy(mission->missionType,missionType);
    if (missionTarget)gfc_line_cpy(mission->missionTarget,missionTarget);
    if (missionSubject)gfc_line_cpy(mission->missionSubject,missionSubject);
    mission->dayStart = dayStart;
    mission->dayFinished = dayFinished;
    mission->staff = staff;
    
    mission_manager.mission_list = gfc_list_append(mission_manager.mission_list,mission);
    return mission;
}

void mission_build_facility(Mission *mission)
{
    int id;
    StationFacility *facility;
    if (!mission)return;
    id = atoi(mission->missionTarget);
    facility = player_get_facility_by_name_id(mission->missionSubject,id);
    if (!facility)return;
    facility->working = 0;
    facility->disabled = 0;
    facility->damage = 0;
    facility->mission = NULL;
    message_printf("Construction of Facility %s complete",facility->displayName);
}

void mission_build_section(Mission *mission)
{
    int id;
    StationSection *section;
    if (!mission)return;
    id = atoi(mission->missionTarget);
    section = station_get_section_by_id(player_get_station_data(),id);
    if (!section)return;
    section->working = 0;
    section->hull = section->hullMax;
    section->mission = NULL;
    message_printf("Construction of Station Section %s complete",section->displayName);
}

void mission_execute(Mission *mission)
{
    int parent;
    StationFacility *facility;
    StationSection *section;
    List *cost;
    char *str;
    int amount;
    int id;
    Window *win;
    if (!mission)return;
    player_return_staff(mission->staff);
    slog("executing mission %s:%i",mission->title,mission->id);
    if (gfc_strlcmp(mission->missionType,"build_facility") == 0)
    {
        mission_build_facility(mission);
        return;
    }
    if (gfc_strlcmp(mission->missionType,"build_section") == 0)
    {
        mission_build_section(mission);
        return;
    }
    if (gfc_strlcmp(mission->missionType,"section_sale") == 0)
    {
        id = atoi(mission->missionTarget);
        section = station_get_section_by_id(player_get_station_data(),id);
        if (!section)return;
        
        cost = station_get_resource_cost(section->name);
        if (section->parent)
        {
            parent = section->parent->id;
        }else parent = 0;
        resource_list_sell(player_get_resources(), cost,0.9);
        resources_list_free(cost);
        station_remove_section(player_get_station_data(),section);

        win = gf2d_window_get_by_name("station_menu");
        if (win)station_menu_select_segment(win,parent);

        message_printf("Section %s removal complete",section->displayName,id);
        return;
    }
    if (gfc_strlcmp(mission->missionType,"facility_sale") == 0)
    {
        id = atoi(mission->missionTarget);
        facility = player_get_facility_by_name_id(mission->missionSubject,id);
        if (!facility)return;
        
        cost = station_facility_get_resource_cost(facility->name,"cost");
        resource_list_sell(player_get_resources(), cost,0.9);
        resources_list_free(cost);

        station_facility_remove(facility);
        
        win = gf2d_window_get_by_name("station_facility_menu");
        if (win)facility_menu_set_list(win);

        message_printf("Facility %s removal complete",facility->displayName,id);
        return;
    }
    if (gfc_strlcmp(mission->missionType,"facility_production") == 0)
    {
        id = atoi(mission->missionTarget);
        facility = player_get_facility_by_name_id(mission->missionSubject,id);
        if (!facility)return;
        message_printf("Facility %s Production completed",facility->displayName,id);
        resource_list_sell(player_get_resources(), facility->produces,facility->productivity);
        return;
    }
    if (gfc_strlcmp(mission->missionType,"commodity_order") == 0)
    {
        amount = atoi(mission->missionTarget);
        resources_list_give(player_get_resources(),mission->missionSubject,amount);
        message_printf("You have received your order for %i tons of %s",amount,resources_get_display_name(mission->missionSubject));
        return;
    }
    if (gfc_strlcmp(mission->missionType,"repair") == 0)
    {
        slog("repair type mission");
        if (gfc_strlcmp(mission->missionSubject,"facility") == 0)
        {
            slog("facility repair");
            str = strchr(mission->missionTarget,':');
            *str = '\0';
            str++;
            id = atoi(mission->missionTarget);
            slog("repairing facility %s : %i",str,id);
            facility = player_get_facility_by_name_id(str,id);
            if (!facility)
            {
                slog("no facility found by that name");
                return;
            }
            station_facility_repair(facility);
            return;
        }
        if (gfc_strlcmp(mission->missionSubject,"section") == 0)
        {
            slog("section repair");
            id = atoi(mission->missionTarget);
            section = station_get_section_by_id(player_get_station_data(),id);
            if (!section)return;
            slog("repairing section: %i",id);
            station_section_repair(section);
            return;
        }
        return;
    }
}

void mission_update_all()
{
    Uint32 day= player_get_day();
    Mission *mission;
    int i,c;
    c = gfc_list_get_count(mission_manager.mission_list);
    for (i = c - 1; i  >= 0; i--)
    {
        mission = gfc_list_get_nth(mission_manager.mission_list,i);
        if (!mission)return;
        if (day >= mission->dayFinished)
        {
            mission_execute(mission);
            mission_free(mission);
            gfc_list_delete_nth(mission_manager.mission_list,i);
        }
    }
}

SJson *mission_save_to_config(Mission *mission)
{
    SJson *json;
    if (!mission)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
    
    sj_object_insert(json,"missionTitle",sj_new_str(mission->title));
    sj_object_insert(json,"id",sj_new_uint32(mission->id));
    sj_object_insert(json,"dayStart",sj_new_uint32(mission->dayStart));
    sj_object_insert(json,"dayFinished",sj_new_uint32(mission->dayFinished));
    sj_object_insert(json,"staff",sj_new_uint32(mission->staff));
    sj_object_insert(json,"missionType",sj_new_str(mission->missionType));
    sj_object_insert(json,"missionTarget",sj_new_str(mission->missionTarget));
    sj_object_insert(json,"missionSubject",sj_new_str(mission->missionSubject));
    
    return json;
}

SJson *missions_save_to_config()
{
    int i,c;
    Mission *mission;
    SJson *json,*item,*array;
    json = sj_object_new();
    if (!json)return NULL;
    array = sj_array_new();
    if (!array)
    {
        sj_free(json);
        return NULL;
    }
    sj_object_insert(json,"idPool",sj_new_uint32(mission_manager.missionIdPool));
    c = gfc_list_get_count(mission_manager.mission_list);
    for (i = 0; i < c; i ++)
    {
        mission = gfc_list_get_nth(mission_manager.mission_list,i);
        if (!mission)continue;
        item = mission_save_to_config(mission);
        if (!item)continue;
        sj_array_append(array,item);
    }
    sj_object_insert(json,"missionList",array);
    return json;
}

Mission *mission_parse_from_config(SJson *config)
{
    const char *str;
    Mission *mission;
    if (!config)return NULL;
    mission = mission_new();
    if (!mission)return NULL;
   
    str = sj_object_get_value_as_string(config,"missionTitle");
    if (str)gfc_line_cpy(mission->title,str);

    sj_object_get_value_as_uint32(config,"id",&mission->id);
    sj_object_get_value_as_uint32(config,"dayStart",&mission->dayStart);
    sj_object_get_value_as_uint32(config,"dayFinished",&mission->dayFinished);
    
    str = sj_object_get_value_as_string(config,"missionType");
    if (str)gfc_line_cpy(mission->missionType,str);
    str = sj_object_get_value_as_string(config,"missionTarget");
    if (str)gfc_line_cpy(mission->missionTarget,str);
    str = sj_object_get_value_as_string(config,"missionSubject");
    if (str)gfc_line_cpy(mission->missionSubject,str);
    return mission;
}

void missions_load_from_config(SJson *config)
{
    int i,c;
    Mission *mission;
    SJson *missions, *item;
    if (!config)return;
    if (mission_manager.mission_list!= NULL)
    {
        mission_close();//clear it all out!
    }
    mission_manager.mission_list = gfc_list_new();
    sj_object_get_value_as_uint32(config,"idPool",&mission_manager.missionIdPool);
    missions = sj_object_get_value(config,"missionList");
    c = sj_array_get_count(missions);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(missions,i);
        if (!item)continue;
        mission = mission_parse_from_config(item);
        if (!mission)continue;
        mission_manager.mission_list = gfc_list_append(mission_manager.mission_list,mission);
    }
}

/*eol@eof*/
