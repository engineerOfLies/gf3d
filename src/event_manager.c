#include "simple_logger.h"

#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "config_def.h"
#include "player.h"
#include "player_history.h"
#include "station_menu.h"
#include "hud_window.h"
#include "main_menu.h"
#include "event_menu.h"
#include "event_manager.h"

static Window *event_window = {0};


void event_manager_execute_effect(SJson *effect)
{
    PlayerData *player;
    const char *str;
    const char *event;
    const char *key;
    const char *value;
    if (!effect)return;
    player = player_get_data();
    if (!player)return;
    str = sj_object_get_value_as_string(effect,"effect");
    if (!str)
    {
        slog("effect not valid");
        return;
    }
    if (strcmp("game_over",str)==0)
    {
        value = sj_object_get_value_as_string(effect,"value");
        if (strcmp("quit",value)==0)
        {
            main_menu();
            return;
        }
        if (strcmp("reload",value)==0)
        {
            gf2d_window_free(gf2d_window_get_by_name("hud_window"));//shut it all down
            hud_window("saves/quick.save");
            return;
        }
        if (strcmp("restart",value)==0)
        {
            gf2d_window_free(gf2d_window_get_by_name("hud_window"));//shut it all down
            main_menu_start_new_game();
            return;
        }
        return;
    }
    if (strcmp("set_history",str)==0)
    {
        event = sj_object_get_value_as_string(effect,"event");
        key = sj_object_get_value_as_string(effect,"key");
        value = sj_object_get_value_as_string(effect,"value");
        player_history_set_event(event,key,value);
        return;
    }
    if (strcmp("trigger_event",str)==0)
    {
        value = sj_object_get_value_as_string(effect,"value");
        event_window = event_menu(NULL,value);
        return;
    }
    if (strcmp("set_assisstant_name",str)==0)
    {
        value = sj_object_get_value_as_string(effect,"value");
        gfc_line_cpy(player->assistantName,value);
        return;
    }
}

void event_manager_handle_choice(SJson *event, int choice)
{
    const char *name;
    int i,c;
    SJson *options,*option,*effect;
    event_window = NULL;
    if (!event)return;
    name = sj_object_get_value_as_string(event,"name");
    player_history_set_event(
        "event",
        name,
        "completed");
    options = sj_object_get_value(event,"options");
    if (!options)
    {
        slog("event missing options");
        return;
    }
    option = sj_array_get_nth(options,choice);
    if (!option)
    {
        slog("event missing option %i",choice);
        return;
    }
    options = sj_object_get_value(option,"effects");
    if (!options)
    {
        return;
    }
    c = sj_array_get_count(options);
    for (i = 0; i < c; i++)
    {
        effect = sj_array_get_nth(options,i);
        if (!effect)continue;
        event_manager_execute_effect(effect);
    }
}

void event_manager_event_end()
{
    event_window = NULL;
}

int event_manager_evaluate_conditions(SJson *conditions)
{
    Window *win;
    const char *condition_type;
    const char *event;
    const char *key;
    const char *value;
    const char *str;
    int i,c;
    SJson *condition;
    c = sj_array_get_count(conditions);
    for (i = 0;i < c; i++)
    {
        condition = sj_array_get_nth(conditions,i);
        if (!condition)continue;
        condition_type = sj_object_get_value_as_string(condition,"type");
        if (!condition_type)continue;
        if (strcmp(condition_type,"check_history")==0)
        {
            event = sj_object_get_value_as_string(condition,"event");
            key = sj_object_get_value_as_string(condition,"key");
            value = sj_object_get_value_as_string(condition,"value");
            str = player_history_get_event(event,key);
            if (!str)return 0;
            if (strcmp(str,value)!=0)return 0;
            continue;
        }
        if (strcmp(condition_type,"repair_mission")==0)
        {
            return 0;
        }
        if (strcmp(condition_type,"window_open")==0)
        {
            str = sj_object_get_value_as_string(condition,"window");
            if (!str)continue;
            if (!gf2d_window_get_by_name(str))return 0;// failed
            continue;
        }
        if (strcmp(condition_type,"section_view")==0)
        {
            str = sj_object_get_value_as_string(condition,"section");
            win = gf2d_window_get_by_name("station_menu");
            if (!win)return 0;
            value = station_menu_get_selected(win);
            if (!value)return 0;
            if (strcmp(str,value)!=0)return 0;
            continue;
        }
    }
    return 1;
}

void event_manager_update()
{
    SJson *event,*history,*conditions;
    Bool once = 0;
    const char *name;
    int startDate,endDate;
    int day;
    int i,c;
    float chance;
    day = player_get_day();
    history = player_get_history();
    if (event_window)return;
    if (!history)return;
    c = config_def_get_resource_count("events");
    for (i = 0; i < c; i++)
    {
        event = config_def_get_by_index("events",i);
        if (!event)continue;
        name = sj_object_get_value_as_string(event,"name");
        if (!name)continue;
        if (sj_object_get_value_as_bool(event,"once",&once) && (once))
        {
            if (player_history_get_event("event",name) != NULL)
            {
                continue;   
            }
        }
        conditions = sj_object_get_value(event,"conditions");
        if (conditions)
        {
            if (!event_manager_evaluate_conditions(conditions))continue;// something wasn't cool
        }
        
        // other tests
        chance = 1;
        sj_object_get_value_as_float(event,"chance",&chance);
        if ((sj_object_get_value_as_int(event,"startDate",&startDate))&&
            (sj_object_get_value_as_int(event,"endDate",&endDate)))
        {
            if ((day >= startDate)&&(day <= endDate))
            {
                if (gfc_random() <= chance)
                {
                    event_window = event_menu(NULL,name);
                    return;// only one event can happen at a time
                }
            }
        }
    }
}


/*eol@eof*/
