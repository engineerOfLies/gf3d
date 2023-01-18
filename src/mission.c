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

List *mission_list_get()
{
    return mission_manager.mission_list;
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
    const char *leader,
    const char *missionType,
    const char *missionSubject,
    const char *missionTarget,
    Uint32 targetId,
    Uint32 dayStart,
    Uint32 timeToComplete,
    Uint32 staff)
{
    Mission *mission;
    mission = mission_new();
    if (!mission)return NULL;
    
    mission->id = mission_manager.missionIdPool++;
    if (missionTitle)gfc_line_cpy(mission->title,missionTitle);
    if (leader)gfc_line_cpy(mission->leader,leader);
    if (missionType)gfc_line_cpy(mission->missionType,missionType);
    if (missionTarget)gfc_line_cpy(mission->missionTarget,missionTarget);
    if (missionSubject)gfc_line_cpy(mission->missionSubject,missionSubject);
    mission->dayStart = dayStart;
    mission->dayFinished = dayStart + timeToComplete;
    mission->staff = staff;
    mission->targetId = targetId;
    
    mission_manager.mission_list = gfc_list_append(mission_manager.mission_list,mission);
    return mission;
}

void mission_build_facility(Mission *mission)
{
    StationFacility *facility;
    if (!mission)return;
    facility = player_get_facility_by_name_id(station_facility_get_name_from_display(mission->missionTarget),mission->targetId);
    if (!facility)return;
    facility->working = 0;
    facility->disabled = 0;
    facility->damage = 0;
    facility->mission = NULL;
    if (facility->staffPositions > 0)
    {
        facility->staffAssigned = mission->staff;
        mission->staff = 0;
        if (facility->staffAssigned > facility->staffPositions)
        {
            mission->staff = (facility->staffAssigned - facility->staffPositions);
            facility->staffAssigned = facility->staffPositions;
        }
    }
    gf2d_window_refresh_by_name("section_view_menu");
    gf2d_window_refresh_by_name("station_facility_menu");
    gf2d_window_refresh_by_name("facility_list_menu");
    message_printf("Construction of Facility %s complete",facility->displayName);
}

void mission_build_section(Mission *mission)
{
    StationSection *section;
    if (!mission)return;
    section = station_get_section_by_id(player_get_station_data(),mission->targetId);
    if (!section)return;
    section->working = 0;
    section->hull = section->hullMax;
    section->mission = NULL;
    message_printf("Construction of Station Section %s complete",section->displayName);
    gf2d_window_refresh_by_name("section_view_menu");
    gf2d_window_refresh_by_name("section_list_menu");
}

void mission_remove_facility(Mission *mission)
{
    List *cost;
    StationFacility * facility;
    if (!mission)return;
    facility = player_get_facility_by_name_id(station_facility_get_name_from_display(mission->missionTarget),mission->targetId);
    if (!facility)return;
    cost = station_facility_get_resource_cost(facility->name,"cost");
    resource_list_sell(player_get_resources(), cost,0.9);
    resources_list_free(cost);
    station_facility_remove(facility);
    gf2d_window_refresh_by_name("section_view_menu");
    gf2d_window_refresh_by_name("station_facility_menu");
    gf2d_window_refresh_by_name("facility_list_menu");
    message_printf("Facility %s removal complete",facility->displayName);
}

void mission_remove_section(Mission *mission)
{
    List *cost;
    StationSection *section;
    if (!mission)return;
    section = station_get_section_by_id(player_get_station_data(),mission->targetId);
    if (!section)return;
    cost = station_get_resource_cost(section->name);
    resource_list_sell(player_get_resources(), cost,0.9);
    resources_list_free(cost);
    station_remove_section(player_get_station_data(),section);
    message_printf("Section %s removal complete",section->displayName);
    gf2d_window_refresh_by_name("section_view_menu");
    gf2d_window_refresh_by_name("section_list_menu");
}

void mission_buy_commodity(Mission *mission)
{
    if (!mission)return;
    resources_list_give(player_get_resources(),mission->missionTarget,mission->targetId);
    message_printf(
        "You have received your order for %i tons of %s",
        mission->targetId,resources_get_display_name(mission->missionSubject));
    return;
}

void mission_repair_section(Mission *mission)
{
    StationSection *section;
    if (!mission)return;
    section = station_get_section_by_id(player_get_station_data(),mission->targetId);
    if (!section)return;
    station_section_repair(section);
    gf2d_window_refresh_by_name("section_view_menu");
    gf2d_window_refresh_by_name("section_list_menu");
}

void mission_repair_facility(Mission *mission)
{
    StationFacility *facility;
    if (!mission)return;
    facility = player_get_facility_by_name_id(station_facility_get_name_from_display(mission->missionTarget),mission->targetId);
    if (!facility)
    {
        slog("no facility found by name of %s id %i",station_facility_get_name_from_display(mission->missionTarget),mission->targetId);
        return;
    }
    gf2d_window_refresh_by_name("section_view_menu");
    gf2d_window_refresh_by_name("station_facility_menu");
    gf2d_window_refresh_by_name("facility_list_menu");
    station_facility_repair(facility);
}

void mission_facility_produce(Mission *mission)
{
    StationFacility *facility;
    if (!mission)return;
    facility = player_get_facility_by_name_id(station_facility_get_name_from_display(mission->missionTarget),mission->targetId);
    if (!facility)return;
    message_printf("Facility %s Production completed",facility->displayName);
    resource_list_sell(player_get_resources(), facility->produces,facility->productivity);
}

// route the mission!
void mission_execute(Mission *mission)
{
    if (!mission)return;
    if (gfc_strlcmp(mission->missionType,"build") == 0)
    {
        if (gfc_strlcmp(mission->missionSubject,"facility") == 0)
        {
            mission_build_facility(mission);
            return;
        }
        if (gfc_strlcmp(mission->missionSubject,"section") == 0)
        {
            mission_build_section(mission);
            return;
        }
        return;
    }
    if (gfc_strlcmp(mission->missionType,"removal") == 0)
    {
        if (gfc_strlcmp(mission->missionSubject,"facility") == 0)
        {
            mission_remove_facility(mission);
            return;
        }
        if (gfc_strlcmp(mission->missionSubject,"section") == 0)
        {
            mission_remove_section(mission);
            return;
        }
        return;
    }
    if (gfc_strlcmp(mission->missionType,"repair") == 0)
    {
        if (gfc_strlcmp(mission->missionSubject,"facility") == 0)
        {
            mission_repair_facility(mission);
            return;
        }
        if (gfc_strlcmp(mission->missionSubject,"section") == 0)
        {
            mission_repair_section(mission);
            return;
        }
        return;
    }
    if (gfc_strlcmp(mission->missionType,"buy") == 0)
    {
        if (gfc_strlcmp(mission->missionSubject,"commodity") == 0)
        {
            mission_buy_commodity(mission);
            return;
        }
        return;
    }
    if (gfc_strlcmp(mission->missionType,"production") == 0)
    {
        if (gfc_strlcmp(mission->missionSubject,"facility") == 0)
        {
            mission_facility_produce(mission);
        }
        return;
    }
    slog("unknown missionType: %s",mission->missionType);
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
            player_return_staff(mission->staff);
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
    sj_object_insert(json,"leader",sj_new_str(mission->leader));
    sj_object_insert(json,"missionType",sj_new_str(mission->missionType));
    sj_object_insert(json,"missionTarget",sj_new_str(mission->missionTarget));
    sj_object_insert(json,"missionSubject",sj_new_str(mission->missionSubject));
    sj_object_insert(json,"targetId",sj_new_uint32(mission->targetId));
    
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
    sj_object_get_value_as_uint32(config,"targetId",&mission->targetId);
    
    str = sj_object_get_value_as_string(config,"missionType");
    if (str)gfc_line_cpy(mission->missionType,str);
    str = sj_object_get_value_as_string(config,"missionTarget");
    if (str)gfc_line_cpy(mission->missionTarget,str);
    str = sj_object_get_value_as_string(config,"missionSubject");
    if (str)gfc_line_cpy(mission->missionSubject,str);
    str = sj_object_get_value_as_string(config,"leader");
    if (str)gfc_line_cpy(mission->leader,str);
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
