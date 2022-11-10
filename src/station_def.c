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

SJson *station_def_get_by_name(const char *name)
{
    int i,c;
    SJson *item;
}

/*eol@eof*/
