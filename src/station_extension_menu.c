#include "simple_logger.h"

#include "gf2d_item_list_menu.h"

#include "station_def.h"
#include "station_extension_menu.h"

Window *station_extension_menu(
    Window *parent,
    const char *title,
    Vector2D position,
    StationSection *section,
    void(*onSelect)(void *),
    void *callbackData,
    int *result)
{
    int i,c;
    const char *name;
    const char *empty = "<empty>";
    Window *win;
    StationSection *child;
    List *list;
    
    if (!section)return NULL;
    c = station_def_get_extension_count_by_name(section->name);
    if (!c)return NULL;
    
    list = gfc_list_new();
    if (!list)return NULL;
    for (i = 0; i < c;i++)
    {
        list = gfc_list_append(list,(void *)empty);//fill it with empties
    }
    
    c = gfc_list_get_count(section->children);
    for (i = 0;i < c; i++)
    {
        child = gfc_list_get_nth(section->children,i);
        if (!child)continue;
        gfc_list_set_nth(list,child->slot,child->displayName);
    }
    
    win = item_list_menu(parent,position,250,(char *)title,list,onSelect,callbackData,result);
    gfc_line_cpy(win->name,"station_extension_menu");
    
    gfc_list_delete(list);
    return win;
}


/*eol@eof*/
