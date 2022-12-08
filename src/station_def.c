#include "simple_logger.h"

#include "gfc_types.h"

#include "config_def.h"
#include "station_def.h"

void station_def_load(const char *filename)
{
    if (!filename)return;
    config_def_load(filename);
}

const char *station_def_get_display_name(const char *section)
{
    SJson *def;
    
    def = config_def_get_by_name("sections",section);
    if (!def)return NULL;
    return sj_object_get_value_as_string(def,"display_name");
}

int station_def_get_extension_count(SJson *section)
{
    SJson *list;
    if (!section)return 0;
    list = sj_object_get_value(section,"extensions");
    if (!list)return 0;
    return sj_array_get_count(list);
}

int station_def_get_extension_count_by_name(const char *name)
{
    return station_def_get_extension_count(config_def_get_by_name("sections",name));
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
