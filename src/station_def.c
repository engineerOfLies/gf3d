#include "simple_logger.h"

#include "gfc_types.h"

#include "station_def.h"

typedef struct
{
    SJson *defs;
}StationDefManager;

static StationDefManager station_def_manager = {0};

void station_def_close()
{
    if (station_def_manager.defs)sj_free(station_def_manager.defs);
    memset(&station_def_manager,0,sizeof(StationDefManager));
}

void station_def_load(const char *filename)
{
    if (!filename)return;
    
    station_def_manager.defs = sj_load(filename);
    if (!station_def_manager.defs)
    {
        slog("failed to load station def file %s");
        return;
    }
    
    atexit(station_def_close);
}

SJson *station_def_get_extension_by_index(SJson *section,Uint8 index)
{
    SJson *list;
    if (!section)return NULL;
    list = sj_object_get_value(section,"extensions");
    if (!list)return NULL;
    return sj_array_get_nth(list,index);
}

SJson *station_def_get_by_name(const char *name)
{
    const char *str;
    int i,c;
    SJson *item,*list;
    if (!station_def_manager.defs)return NULL;
    list = sj_object_get_value(station_def_manager.defs,"sections");
    c = sj_array_get_count(list);
    for (i = 0; i < c;i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        str = sj_object_get_value_as_string(item,"name");
        if (!str)continue;
        if (strcmp(name,str)==0)return item;
    }
    return NULL;
}

/*eol@eof*/
