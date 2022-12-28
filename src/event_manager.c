#include "simple_logger.h"

#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf3d_camera.h"

#include "config_def.h"
#include "player.h"
#include "player_history.h"
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
        slog("event missing options");
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

void event_manager_update()
{
    SJson *event,*history;
    Bool once = 0;
    const char *name;
    int startDate,endDate;
    int day;
    int i,c;
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
        if (sj_object_get_value_as_bool(event,"once",&once) && once)
        {
            if (player_history_get_event("event",name) != NULL)continue;
        }
        
        // other tests
        
        if ((sj_object_get_value_as_int(event,"startDate",&startDate))&&
            (sj_object_get_value_as_int(event,"endDate",&endDate)))
        {
            if ((day >= startDate)&&(day <= endDate))
            {
                event_window = event_menu(NULL,name);
                return;// only one event can happen at a time
            }
        }
    }
}


/*eol@eof*/
