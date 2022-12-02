#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"

#include "config_def.h"

#include "resources.h"


Resource *resource_new()
{
    return gfc_allocate_array(sizeof(Resource),1);
}

List *resources_list_new()
{
    return gfc_list_new();
}

Resource *resources_list_get(List *list,const char *name)
{
    Resource *resource;
    int i,c;
    if (!list)return NULL;
    if (!name)return NULL;
    c = gfc_list_get_count(list);
    for (i = 0; i < c; i++)
    {
        resource = gfc_list_get_nth(list,i);
        if (!resource)continue;
        if (strcmp(name,resource->name)==0)return resource;
    }
    return NULL;
}

void resources_list_free(List *list)
{
    if (!list)return;
    gfc_list_foreach(list,free);
    gfc_list_delete(list);
}

void resources_list_load()
{
    config_def_load("config/resources.def");
}

List *resources_list_parse(SJson *config)
{
    List *resources;
    SJson *item,*res;
    const char *name;
    float count;
    int i,c;
    if (!config)
    {
        slog("no config provided");
        return NULL;
    }
    resources = gfc_list_new();
    c = config_def_get_resource_count("resources");
    for (i = 0;i < c;i++)
    {
        item = config_def_get_by_index("resources",i);
        if (!item)continue;
        name = sj_object_get_value_as_string(item,"name");
        //check if this resource name is in the config
        res = sj_object_get_value(config,name);
        if (!res)continue;
        sj_get_float_value(res,&count);
        resources = resources_list_give(resources,name,count);
    }
    return resources;
}

SJson *resources_list_save(List *list)
{
    int i,c;
    Resource *resource;
    SJson *json;
    if (!list)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
    c = gfc_list_get_count(list);
    for (i =0; i < c; i++)
    {
        resource = gfc_list_get_nth(list,i);
        if (!resource)continue;
        sj_object_insert(json,resource->name,sj_new_float(resource->amount));
    }
    return json;
}

List *resources_list_give(List *list,const char *name,float count)
{
    Resource *resource;
    if (!list)
    {
        slog("no resource list provided");
        return NULL;
    }
    resource = resources_list_get(list,name);
    if (!resource)
    {
        resource = resource_new();
        if (!resource)return list;
        gfc_line_cpy(resource->name,name);
        list = gfc_list_append(list,resource);
    }
    resource->amount += count;
    return list;
}

float resources_list_get_amount(List *list,const char *name)
{
    Resource *resource;
    if (!list)return 0;
    resource = resources_list_get(list,name);
    if (!resource)return 0;
    return resource->amount;
}
