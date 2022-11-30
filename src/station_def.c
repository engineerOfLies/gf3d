#include "simple_logger.h"

#include "gfc_types.h"

#include "config_def.h"
#include "station_def.h"

void station_def_load(const char *filename)
{
    if (!filename)return;
    config_def_load(filename);
}

SJson *station_def_get_extension_by_index(SJson *section,Uint8 index)
{
    SJson *list;
    if (!section)return NULL;
    list = sj_object_get_value(section,"extensions");
    if (!list)return NULL;
    return sj_array_get_nth(list,index);
}

/*eol@eof*/
