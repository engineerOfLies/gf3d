#include "simple_logger.h"
#include "gf2d_font.h"
#include "gf2d_element_label.h"
#include "gf2d_element_list.h"
#include "resources.h"
#include "resource_list.h"


Element *resource_list_new(Window *win,const char *name, Vector2D offset,List *supply,List *cost)
{
    int i,c;
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
            gf2d_element_list_add_item(e,gf2d_label_new_simple(win,10+i,buffer,FT_H5,gfc_color8(1,1,1,1)));
        }
    }
    
    return e;
}

/*eol@eof*/
