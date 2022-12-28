#include "simple_logger.h"
#include "simple_json.h"

#include "player.h"
#include "player_history.h"

void player_history_set_event(
    const char *event,
    const char *key,
    const char *value)
{
    SJson *history;
    SJson *eventJS;
    history = player_get_history();
    if (!history)
    {
        slog("cannot set history event, no player history");
        return;// nothing to do
    }
    eventJS = sj_object_get_value(history,event);
    if (!eventJS)
    {
        eventJS = sj_object_new();
        if (!eventJS)return;
        sj_object_insert(history,event,eventJS);
    }
    else
    {
        sj_object_delete_key(eventJS,key);// delete it if it already exists
    }
    sj_object_insert(eventJS,key,sj_new_str(value));
}

const char *player_history_get_event(
    const char *event,
    const char *key)
{
    const char *value;
    SJson *history;
    SJson *eventJS;
    history = player_get_history();
    if (!history)
    {
        slog("cannot get history event, no player history!");
        return NULL;// nothing to do
    }
    eventJS = sj_object_get_value(history,event);
    if (!eventJS)
    {
        return NULL;
    }
    value = sj_get_string_value(sj_object_get_value(eventJS,key));
    return value;
}


/*eol@eof*/
