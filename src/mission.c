#include "simple_logger.h"

#include "player.h"
#include "mission.h"

static List *mission_list = {0};


void mission_init()
{
    mission_list = gfc_list_new();
}

void mission_free(Mission *mission)
{
    if (!mission)return;
    
    free(mission);
}

Mission *mission_new(void *data, gfc_work_func callback, Uint32 dayStart, Uint32 dayFinished)
{
    Mission *mission;
    mission = gfc_allocate_array(sizeof(Mission),1);
    if (!mission)return NULL;
    
    mission->callback.data = data;
    mission->callback.callback = callback;
    mission->dayStart = dayStart;
    mission->dayFinished = dayFinished;
    mission_list = gfc_list_append(mission_list,mission);
    return mission;
}

void mission_update_all()
{
    Uint32 day;
    Mission *mission;
    int i,c;
    day = player_get_day();
    c = gfc_list_get_count(mission_list);
    for (i = c - 1; i  >= 0; i--)
    {
        mission = gfc_list_get_nth(mission_list,i);
        if (!mission)return;
        if (day >= mission->dayFinished)
        {
            if (mission->callback.callback)gfc_callback_call(&mission->callback);
            mission_free(mission);
            gfc_list_delete_nth(mission_list,i);
        }
    }
}

/*eol@eof*/
