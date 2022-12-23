#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"

#include "resources.h"
#include "config_def.h"
#include "station_def.h"

void station_def_load(const char *filename)
{
    if (!filename)return;
    config_def_load(filename);
}

List *station_def_get_section_list()
{
    int i,c;
    const char *str;
    SJson *item;
    List *list;
    c = config_def_get_resource_count("sections");
    if (!c)return NULL;
    list = gfc_list_new();
    for (i = 0; i < c;i++)
    {
        item = config_def_get_by_index("sections",i);
        if (!item)continue;
        str = sj_object_get_value_as_string(item,"name");
        if (!str)continue;
        list = gfc_list_append(list,(void *)str);
    }
    return list;
}

const char *station_def_get_name_by_display(const char *section)
{
    SJson *def;
    def = config_def_get_by_parameter("sections","display_name",section);
    if (!def)return NULL;
    return sj_object_get_value_as_string(def,"name");
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

int station_def_is_unique(const char *name)
{
    Bool unique;
    SJson *def;
    def = config_def_get_by_name("sections",name);
    if (!def)return 0;
    if ((sj_object_get_value_as_bool(def,"unique",&unique))&&(unique))return 1;
    return 0;
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

List *station_get_resource_cost(const char *name)
{
    SJson *stationDef;
    if (!name)return NULL;
    stationDef = config_def_get_by_name("sections",name);
    if (!stationDef)return NULL;
    return resources_list_parse(sj_object_get_value(stationDef,"cost"));

}

/*eol@eof*/
