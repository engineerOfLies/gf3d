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

/**
 * @brief give one
 */
void resources_list_give(List *list,const char *name,float count)
{
    Resource *resource;
    if (!list)return;
    resource = resources_list_get(list,name);
    if (!resource)
    {
        resource = resource_new();
        if (!resource)return;
        gfc_line_cpy(resource->name,name);
    }
    resource->amount += count;
}

float resources_list_get_amount(List *list,const char *name)
{
    Resource *resource;
    if (!list)return 0;
    resource = resources_list_get(list,name);
    if (!resource)return 0;
    return resource->amount;
}
