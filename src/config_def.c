#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_list.h"

#include "config_def.h"

typedef struct
{
    List *defs;
}ConfigManager;

static ConfigManager config_manager = {0};

void config_def_close()
{
    if (config_manager.defs)
    {
        gfc_list_foreach(config_manager.defs,(void (*)(void *))sj_free);
        gfc_list_delete(config_manager.defs);
        config_manager.defs = NULL;
    }
    memset(&config_manager,0,sizeof(ConfigManager));
}

void config_def_init()
{
    config_manager.defs = gfc_list_new();
    atexit(config_def_close);
}

void config_def_load(const char *filename)
{
    SJson *json;
    if (!filename)return;
    

    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load config def file %s");
        return;
    }
    gfc_list_append(config_manager.defs,json);
}

SJson *config_def_get_resource_by_name(const char *resource)
{
    SJson *list,*item;
    int i,c;
    if (!resource)return NULL;
    if (!config_manager.defs)return NULL;
    c = gfc_list_get_count(config_manager.defs);
    for (i = 0; i < c;i++)
    {
        list = gfc_list_get_nth(config_manager.defs,i);
        if (!list)continue;
        item = sj_object_get_value(list,resource);
        if (item)return item;
    }
    return NULL;
}

SJson *config_def_get_by_index(const char *resource,Uint8 index)
{
    SJson *list;
    if (!config_manager.defs)
    {
        slog("config def file not loaded");
        return NULL;
    }
    if (!resource)return NULL;
    list = config_def_get_resource_by_name(resource);
    if (!list)return NULL;
    return sj_array_get_nth(list,index);
}

const char *config_def_get_name_by_index(const char *resource,Uint8 index)
{
    SJson *list;
    SJson *def;
    if (!config_manager.defs)
    {
        slog("config def file not loaded");
        return NULL;
    }
    if (!resource)return NULL;
    list = config_def_get_resource_by_name(resource);
    if (!list)return NULL;
    def = sj_array_get_nth(list,index);
    if (!def)return NULL;
    return sj_object_get_value_as_string(def,"name");
}


Uint32 config_def_get_resource_count(const char *resource)
{
    SJson *list;
    if (!config_manager.defs)return 0;
    list = config_def_get_resource_by_name(resource);
    if (!list)return 0;
    return sj_array_get_count(list);
}

SJson *config_def_get_by_parameter(const char *resource,const char *parameter,const char *name)
{
    const char *str;
    int i,c;
    SJson *item,*list;
    if (!config_manager.defs)return NULL;
    list = config_def_get_resource_by_name(resource);
    if (!list)return NULL;
    c = sj_array_get_count(list);
    for (i = 0; i < c;i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        str = sj_object_get_value_as_string(item,parameter);
        if (!str)continue;
        if (strlen(str) != strlen(name))continue;
        if (strcmp(name,str)==0)return item;
    }
    slog("no resource of %s found by parameter of %s and name of %s",resource,parameter,name);
    return NULL;
}

SJson *config_def_get_by_name(const char *resource,const char *name)
{
    const char *str;
    int i,c;
    SJson *item,*list;
    if (!config_manager.defs)return NULL;
    list = config_def_get_resource_by_name(resource);
    if (!list)return NULL;
    c = sj_array_get_count(list);
    for (i = 0; i < c;i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        str = sj_object_get_value_as_string(item,"name");
        if (!str)continue;
        if (strlen(str) != strlen(name))continue;
        if (strcmp(name,str)==0)return item;
    }
    slog("no resource of %s found by name of %s",resource,name);
    return NULL;
}

/*eol@eof*/
