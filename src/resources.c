#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"

#include "gf2d_font.h"
#include "gf2d_element_label.h"
#include "gf2d_element_list.h"
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

void resource_list_sell(List *supply, List *cost,float rate)
{
    int i,c;
    Resource *resource;
    if ((!cost)||(!supply)||(!rate))return;
    c = gfc_list_get_count(cost);
    for (i = 0; i < c; i++)
    {
        resource = gfc_list_get_nth(cost,i);
        if (!resource)continue;
        supply = resources_list_give(supply,resource->name,resource->amount*rate);
    }
}

void resource_list_buy(List *supply, List *cost)
{
    int i,c;
    Resource *resource;
    if ((!cost)||(!supply))return;
    c = gfc_list_get_count(cost);
    for (i = 0; i < c; i++)
    {
        resource = gfc_list_get_nth(cost,i);
        if (!resource)continue;
        resources_list_withdraw(supply,resource->name,resource->amount);
    }
}

int resources_list_afford(List *supply, List *cost)
{
    int i,c;
    float amount;
    Resource *resource;
    if (!cost)return 1;
    if (!supply)return 0;
    c = gfc_list_get_count(cost);
    for (i = 0; i < c; i++)
    {
        resource = gfc_list_get_nth(cost,i);
        if (!resource)continue;
        amount = resources_list_get_amount(supply,resource->name);
        if (amount < resource->amount)return 0;
    }
    return 1;
}

void resources_list_withdraw(List *list,const char *name,float count)
{
    Resource *resource;
    if (!list)
    {
        slog("no resource list provided");
        return;
    }
    resource = resources_list_get(list,name);
    if (!resource)
    {
        return;
    }
    resource->amount -= count;
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

Element *resource_list_element_new(Window *win,const char *name, Vector2D offset,List *supply,List *cost)
{
    int i,c;
    Color color;
    float amount;
    TextLine buffer;
    Resource *resource;
    ListElement *le;
    Element *e;
    
    if (!supply)return NULL;
    
    le = gf2d_element_list_new_full(
        gfc_rect(0,0,1,1),
        vector2d(1,24),
        LS_Vertical,
        0,
        0,
        1,
        1);
    e = gf2d_element_new_full(
        NULL,
        0,
        (char *)name,
        gfc_rect(offset.x,offset.y,1,1),
        gfc_color(1,1,1,1),
        0,
        gfc_color(1,1,1,1),0,win);
    gf2d_element_make_list(e,le);
    
    
    if (!cost)
    {//raw data, no color
        c = gfc_list_get_count(supply);
        for (i = 0; i < c; i++)
        {
            resource = gfc_list_get_nth(supply,i);
            if (!resource)continue;
            gfc_line_sprintf(buffer,"%.2f: %s",resource->amount,resource->name);
            gf2d_element_list_add_item(e,gf2d_label_new_simple(win,10+i,buffer,FT_H5,gfc_color(1,1,1,1)));
        }
    }
    else 
    {//raw data, no color
        c = gfc_list_get_count(cost);
        for (i = 0; i < c; i++)
        {
            resource = gfc_list_get_nth(cost,i);
            if (!resource)continue;
            amount = resources_list_get_amount(supply,resource->name);
            if (amount >= resource->amount)color = gfc_color(0,1,0,1);
            else color = gfc_color(0.9,0.2,0.2,1);
            gfc_line_sprintf(buffer,"%.2f / %.2f: %s",amount, resource->amount,resource->name);
            gf2d_element_list_add_item(e,gf2d_label_new_simple(win,10+i,buffer,FT_H5,color));
        }
    }
    return e;
}

