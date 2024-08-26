#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "simple_logger.h"

#include "gfc_hashmap.h"
#include "gfc_audio.h"
#include "gfc_pak.h"

#include "gf3d_vgraphics.h"

#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "gf2d_windows.h"
#include "gf2d_elements.h"

typedef enum
{
    WDS_Tiled,
    WDS_Stretched,
    WDS_Max
}WindowDrawStyle;

typedef struct
{
    Sprite *generic_border;     /**<sprite to use to draw the border*/
    Sprite *generic_background; /**<sprite to use to draw the background*/
    WindowDrawStyle style;      /**<tiled or stretched*/
    GFC_List *windowList;        /**<list of all active windows*/
    GFC_List *window_deque;         /**<draw order is back to front, update order is front to back*/
    int drawbounds;             /**<if true draw rects around window bounds*/
    GFC_HashMap *sounds;            /**<sound pack for windows and elements*/
}WindowManager;

static WindowManager window_manager = {0};

void gf2d_window_delete(Window *win);
Window *gf2d_window_load_from_json(SJson *json);
Element *gf2d_window_get_next_element(Window *win,Element *from);
void gf2d_window_calibrate(Window *win);


void gf2d_draw_window_border_generic(GFC_Rect rect,GFC_Color color)
{
    if (window_manager.style == WDS_Tiled)
    {
        gf2d_draw_window_border_tiled(window_manager.generic_border,window_manager.generic_background,rect,color);
    }
    else if (window_manager.style == WDS_Stretched)
    {
        gf2d_draw_window_border_stretched(window_manager.generic_border,window_manager.generic_background,rect,color);
    }
}

void gf2d_draw_window_border_tiled(Sprite *border,Sprite *bg,GFC_Rect rect,GFC_Color color)
{
    int i;
    GFC_Vector4D clip = {0,0,0,0};
    GFC_Vector2D count = {0};
    GFC_Vector2D fraction = {0};
    GFC_Vector2D scale = {0};
    if ((bg)&&(bg->frameWidth != 0)&&(bg->frameHeight != 0))
    {
        
        scale.x = (rect.w + 0.5)/bg->frameWidth;
        scale.y = (rect.h + 0.5)/bg->frameHeight;
        gf2d_sprite_draw(
            bg,
            gfc_vector2d(rect.x,rect.y),
            &scale,
            NULL,
            NULL,
            NULL,
            &color,
            &clip,
            0);
    }
    if (!border)
    {
        slog("no border provided");
        return;
    }
    if ((border->frameWidth == 0)||(border->frameHeight == 0))return;
    count.x = (int)(rect.w / border->frameWidth) - 1;
    fraction.x = ((float)rect.w / (float)border->frameWidth) - 1;
    count.y = (int)(rect.h / border->frameHeight) - 1;
    fraction.y = ((float)rect.h / (float)border->frameHeight) - 1;
    gfc_vector2d_sub(fraction,fraction,count);
    // draw horizontal borders
    for (i = 0; i  < count.x; i++)
    {
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x + border->frameWidth/2 + (i * border->frameWidth),rect.y - border->frameWidth/2),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            NULL,
            BE_Top);
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x + border->frameWidth/2 + (i * border->frameWidth),rect.y + rect.h - border->frameWidth/2),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            NULL,
            BE_Bottom);
    }
    if (fraction.x > 0)
    {
        clip.z = (1.0 - fraction.x)*border->frameWidth;
        clip.w = 0;
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x + border->frameWidth/2 + (i * border->frameWidth),rect.y - border->frameWidth/2),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            &clip,
            BE_Top);
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x + border->frameWidth/2 + (i * border->frameWidth),rect.y + rect.h - border->frameWidth/2),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            &clip,
            BE_Bottom);
    }
    
    //vertical borders
    for (i = 0; i  < count.y; i++)
    {
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x - border->frameWidth/2,rect.y + border->frameWidth/2  + (i * border->frameHeight)),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            NULL,
            BE_Left);
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x + rect.w - border->frameWidth/2,rect.y + border->frameWidth/2  + (i * border->frameHeight)),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            NULL,
            BE_Right);
    }
    if (fraction.y > 0)
    {
        clip.x = 0;
        clip.y = 0;
        clip.z = 0;
        clip.w = (1- fraction.y)*border->frameHeight;
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x - border->frameWidth/2,rect.y + border->frameWidth/2  + (i * border->frameHeight)),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            &clip,
            BE_Left);
        gf2d_sprite_draw(
            border,
            gfc_vector2d(rect.x + rect.w - border->frameWidth/2,rect.y + border->frameWidth/2  + (i * border->frameHeight)),
            NULL,
            NULL,
            NULL,
            NULL,
            &color,
            &clip,
            BE_Right);
    }
    //corners
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x + rect.w - border->frameWidth/2,rect.y - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_TR);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x - border->frameWidth/2,rect.y + rect.h - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_BL);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x - border->frameWidth/2,rect.y - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_TL);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x + rect.w - border->frameWidth/2,rect.y + rect.h - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_BR);
    if (window_manager.drawbounds)
    {
        gf2d_draw_rect(rect,gfc_color8(255,100,100,255));
    }

}

void gf2d_draw_window_border_stretched(Sprite *border,Sprite *bg,GFC_Rect rect,GFC_Color color)
{
    GFC_Vector2D scale = {0};
    if ((bg)&&(bg->frameWidth != 0)&&(bg->frameHeight != 0))
    {
        scale.x = rect.w/bg->frameWidth;
        scale.y = rect.h/bg->frameHeight;
        gf2d_sprite_draw(
            bg,
            gfc_vector2d(rect.x,rect.y),
            &scale,
            NULL,
            NULL,
            NULL,
            &color,
            NULL,
            0);
    }
    if ((!border)||(border->frameHeight == 0)||(border->frameWidth == 0))return;
    scale.x = (rect.w - border->frameWidth)/(float)border->frameWidth;
    scale.y = 1;
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x + border->frameWidth/2,rect.y - border->frameWidth/2),
        &scale,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_Top);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x + border->frameWidth/2,rect.y + rect.h- border->frameWidth/2),
        &scale,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_Bottom);

    scale.y = (rect.h - border->frameHeight)/(float)border->frameHeight;
    scale.x = 1;

    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x - border->frameWidth/2,rect.y + border->frameWidth/2),
        &scale,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_Left);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x + rect.w - border->frameWidth/2,rect.y + border->frameWidth/2),
        &scale,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_Right);
    //corners
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x + rect.w - border->frameWidth/2,rect.y - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_TR);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x - border->frameWidth/2,rect.y + rect.h - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_BL);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x - border->frameWidth/2,rect.y - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_TL);
    gf2d_sprite_draw(
        border,
        gfc_vector2d(rect.x + rect.w - border->frameWidth/2,rect.y + rect.h - border->frameWidth/2),
        NULL,
        NULL,
        NULL,
        NULL,
        &color,
        NULL,
        BE_BR);
    if (window_manager.drawbounds)
    {
        gf2d_draw_rect(rect,gfc_color8(255,100,100,255));
    }
}

void gf2d_windows_close()
{
    if (window_manager.windowList)
    {
        gfc_list_foreach(window_manager.windowList,(gfc_work_func*)gf2d_window_delete);
        gfc_list_delete(window_manager.windowList);
    }
    gfc_list_delete(window_manager.window_deque);
   
    gf2d_sprite_free(window_manager.generic_border);
    gf2d_sprite_free(window_manager.generic_background);
    gfc_sound_pack_free(window_manager.sounds);
    memset(&window_manager,0,sizeof(WindowManager));
}

void gf2d_windows_init(int max_windows,const char *config)
{
    GFC_TextLine background,border;
    const char *str;
    GFC_Vector2D borderSize = {64,64};
    int      borderFPL = 8;
    SJson *file,*window,*sounds;
    if (max_windows <= 0)
    {
        slog("cannot initilize window system for 0 windows");
        return;
    }
    window_manager.windowList = gfc_list_new_size(max_windows);
    if(window_manager.windowList == NULL)
    {
        slog("failed to allocate memory for window system");
        return;
    }
    window_manager.window_deque = gfc_list_new();
    //defaults
    gfc_line_cpy(background,"images/ui/window_background.png");
    gfc_line_cpy(border,"images/ui/window_border.png");
    
    //config overrides
    if (config)
    {
        file = gfc_pak_load_json(config);
        if (file)
        {
            window = sj_object_get_value(file,"window");
            if (window)
            {
                str = sj_get_string_value(sj_object_get_value(window,"background"));
                if (str)gfc_line_cpy(background,str);
                str = sj_get_string_value(sj_object_get_value(window,"border"));
                if (str)gfc_line_cpy(border,str);
                sj_value_as_vector2d(sj_object_get_value(window,"borderSize"),&borderSize);
                sj_get_integer_value(sj_object_get_value(window,"borderFPL"),&borderFPL);
                str = sj_get_string_value(sj_object_get_value(window,"drawStyle"));
                if (str)
                {
                    if (strcmp(str,"tiled")==0)window_manager.style = WDS_Tiled;
                    else if (strcmp(str,"stretched")==0)window_manager.style = WDS_Stretched;
                }
            }
            sounds = sj_object_get_value(file,"sounds");
            if (sounds)
            {
                window_manager.sounds = gfc_sound_pack_parse(sounds);
            }
        }
        sj_free(file);
    }
    window_manager.generic_background = gf2d_sprite_load_image(background);
    window_manager.generic_border = gf2d_sprite_load(border,borderSize.x,borderSize.y,borderFPL);
    window_manager.drawbounds = 0;
    atexit(gf2d_windows_close);
}

int gf2d_window_check(Window *win,const char *name)
{
    if ((!win)||(!win->data))return 0;
    if (name)
    {
        if (gfc_strlcmp(win->name,name)!= 0)return 0;
    }
    return 1;
}

void gf2d_window_close_child(Window *parent,Window *child)
{
    if ((!parent)||(!child))return;
    if (parent->child == child)// double check that we are closing the reference to ourselves
    {
        parent->child = NULL;
    }
}

void gf2d_window_free(Window *win)
{
    int i,count;
    if (!win)return;
    if (win->free_data != NULL)
    {
        win->free_data(win);
    }
    if (win->child)gf2d_window_free(win->child);
    if (win->parent)gf2d_window_close_child(win->parent,win);
    gfc_list_delete_data(window_manager.window_deque,win);
    count = gfc_list_get_count(win->elements);
    for (i = 0;i < count;i++)
    {
        gf2d_element_free((Element*)gfc_list_get_nth(win->elements,i));
    }
    if (win->elements)gfc_list_delete(win->elements);
    if (win->focus_elements)gfc_list_delete(win->focus_elements);// only delete the list, the data is handled by the other list
    memset(win,0,sizeof(Window));
}

void gf2d_window_delete(Window *win)
{
    if (!win)return;
    if (win->_inuse)gf2d_window_free(win);
    free(win);
}


void gf2d_window_refresh(Window *win)
{
    if ((!win)||(!win->refresh))return;
    win->refresh(win);
}

void gf2d_window_refresh_by_name(const char *name)
{
    Window *win;
    if (!name)return;
    win = gf2d_window_get_by_name(name);
    if (!win)return;
    win->refresh(win);
}


void gf2d_window_set_dimensions(Window *win,GFC_Rect dimensions)
{
    int i,c;
    Element *e;
    if (!win)return;
    gfc_rect_copy(win->dimensions,dimensions);
    gf2d_window_calibrate(win);
    c = gfc_list_count(win->elements);
    for (i = 0; i< c; i++)
    {
        e = gfc_list_nth(win->elements,i);
        if (!e)continue;
        gf2d_element_recalibrate(e);
    }
}

void gf2d_window_set_position(Window *win,GFC_Vector2D position)
{
    if (!win)return;
    gfc_vector2d_copy(win->dimensions,position);
}

void gf2d_window_hide(Window *win)
{
    if (!win)return;
    win->hidden = 1;
}

void gf2d_window_unhide(Window *win)
{
    if (!win)return;
    win->hidden = 0;
}

void gf3d_window_hide_toggle(Window *win)
{
    if (!win)return;
    win->hidden = !win->hidden;
}


void gf2d_window_draw(Window *win)
{
    int count,i;
    GFC_Vector2D offset;
    if (!win)return;
    if (win->hidden)return;
    if (win->disabled)return;
    if (!win->no_draw_generic)
    {
        gf2d_draw_window_border_generic(win->dimensions,win->color);
    }
    offset.x = win->dimensions.x + win->canvas.x;
    offset.y = win->dimensions.y + win->canvas.y;
    count = gfc_list_get_count(win->elements);
    for (i = 0;i < count;i++)
    {
        gf2d_element_draw((Element *)gfc_list_get_nth(win->elements,i), offset);
    }
}

int gf2d_window_update(Window *win)
{
    int count,i;
    int retval = 0;
    GFC_Vector2D offset;
    GFC_List *updateGFC_List = gfc_list_new();
    GFC_List *updated = NULL;
    Element *e;
    if (!win)return 0;
    if (win->disabled)return 0;
    offset.x = win->dimensions.x + win->canvas.x;
    offset.y = win->dimensions.y + win->canvas.y;
    count = gfc_list_get_count(win->elements);
    for (i = 0;i < count;i++)
    {
        e = (Element *)gfc_list_get_nth(win->elements,i);
        if (!e)continue;
        updated = gf2d_element_update(e, offset);
        if (updated)
        {
            updateGFC_List = gfc_list_concat_free(updateGFC_List,updated);
        }
    }
    if (win->update)
    {
        retval = win->update(win,updateGFC_List);
    }
    gfc_list_delete(updateGFC_List);
    return retval;
}

void gf2d_window_make_focus_list(Window *win)
{
    Element *element = NULL;
    if (!win)return;
    element = gf2d_window_get_next_element(win,NULL);
    if (win->focus_elements)
    {
        gfc_list_delete(win->focus_elements);
        win->focus_elements = gfc_list_new();
    }
    while(element)
    {
        if (element->canHasFocus)
        {
            gfc_list_append(win->focus_elements,element);
        }
        element = gf2d_window_get_next_element(win,element);
    }
}

Element *gf2d_window_get_next_element(Window *win,Element *from)
{
    int c,i;
    Element *e,*sub,*e2;
    if (!win)return NULL;
    if (from == NULL)
    {
        return gfc_list_get_nth(win->elements,0);
    }
    c = gfc_list_get_count(win->elements);
    for (i = 0; i < c; i++)
    {
        e = gfc_list_get_nth(win->elements,i);
        if (!e)continue;
        if (e == from)
        {
            if (!e->get_next)continue;
            sub = e->get_next(e,from);
            if (sub == NULL)
            {
                //get next in the array
                e2 = gfc_list_get_nth(win->elements,i+1);
                if (!e2)// no more elements
                {
                    return NULL;// there is nothing left in the list
                }
                return e2;//next aquired
            }
            if (sub == from)
            {
                return NULL;
            }
            return sub;
        }
        if (!e->get_next)continue;
        sub = e->get_next(e,from);
        if (sub == from)
        {
            if ((i +1) >= c)return NULL;
            return gfc_list_get_nth(win->elements,i+1);
        }        
        if (sub != NULL)
        {
            return sub;
        }
    }
    return NULL;
}

Element *gf2d_window_get_element_by_focus(Window *win)
{
    int i,c;
    Element *e;
    if (!win)return NULL;
    if (win->focus) return win->focus;
    c = gfc_list_get_count(win->focus_elements);
    for (i = 0; i < c; i++)
    {
        e = gfc_list_get_nth(win->focus_elements,i);
        if (!e)continue;
        if (e->hasFocus)return e;
    }
    return NULL;
}

void gf2d_windows_play_sound(const char *name)
{
    if ((!name)||(!strlen(name)))return;
    gfc_sound_pack_play(window_manager.sounds, name,0,1,-1,0);
}

void gf2d_window_set_focus_to(Window *win,Element *e)
{
    Element *f;
    if (e == NULL)
    {
        return;
    }
    if (!e->canHasFocus)
    {
        return;
    }
    f = gf2d_window_get_element_by_focus(win);
    if (f)
    {
        f->hasFocus = 0;
        gfc_sound_pack_play(window_manager.sounds, "select",0,1,-1,0);
        //only play select sound when selection CHANGES, not when initially set
    }
    e->hasFocus = 1;
    win->focus = e;
}

void gf2d_window_prev_focus(Window *win)
{
    int c;
    Uint32 index;
    Element *old;
    
    old = gf2d_window_get_element_by_focus(win);
    c = gfc_list_get_count(win->focus_elements);
    index = gfc_list_get_item_index(win->focus_elements,old);
    if (index == 0)index = c;
    index--;
    gf2d_window_set_focus_to(win,gfc_list_get_nth(win->focus_elements,index));
}

void gf2d_window_next_focus(Window *win)
{
    Element *e;
    int c;
    Uint32 index;
    Element *old;
    old = gf2d_window_get_element_by_focus(win);
    c = gfc_list_get_count(win->focus_elements);
    index = gfc_list_get_item_index(win->focus_elements,old);
    index++;
    if (index >= c)index = 0;
    e = gfc_list_get_nth(win->focus_elements,index);
    gf2d_window_set_focus_to(win,e);
}

Window *gf2d_window_new()
{
    Window *win;
    win = gfc_allocate_array(sizeof(Window),1);
    if (!win)return NULL;
    win->_inuse = 1;
    gfc_list_append(window_manager.windowList,win);
    gfc_list_append(window_manager.window_deque,win);
    win->elements = gfc_list_new();
    win->focus_elements = gfc_list_new();
    return win;
}

Window *gf2d_window_get_by_name(const char *name)
{
    Window *win;
    int i,c;
    if (!name)return NULL;
    c = gfc_list_count(window_manager.windowList);
    for (i = 0;i < c;i++)
    {
        win = gfc_list_nth(window_manager.windowList,i);
        if (!win)continue;
        if (!win->_inuse)continue;
        if (strcmp(win->name,name) == 0)return win;
    }
    return NULL;
}

void gf2d_window_add_element(Window *win,Element *e)
{
    if (!win)return;
    if (!e)return;
    gfc_list_append(win->elements,e);
}

void gf2d_windows_draw_all()
{
    int i,count;
    Window* win;
    count = gfc_list_get_count(window_manager.window_deque);
    for (i = 0; i < count; i++)
    {
        win = (Window*)gfc_list_get_nth(window_manager.window_deque,i);
        if (!win)continue;
        if (win->draw)
        {
            if (!win->draw(win))
            {
                gf2d_window_draw(win);
            }
        }
        else gf2d_window_draw(win);
    }
}

int gf2d_windows_update_all()
{
    Window *win;
    int i,count;
    int retval = 0;
    count = gfc_list_count(window_manager.window_deque);
    for (i = count - 1; i >= 0; i--)
    {
        retval = gf2d_window_update((Window*)gfc_list_nth(window_manager.window_deque,i));
        if (retval)break;
    }
    count = gfc_list_count(window_manager.windowList);
    for (i = count -1 ; i >= 0; i--)
    {
        win = gfc_list_nth(window_manager.windowList,i);
        if (!win)continue;
        if (win->_inuse)continue;
        gfc_list_delete_nth(window_manager.windowList,i);
        gf2d_window_delete(win);
    }
    return retval;
}

void gf2d_window_align(Window *win,int vertical)
{
    GFC_Vector2D res;
    if (!win)return;
    res = gf3d_vgraphics_get_resolution();
    if (vertical < 0)
    {
        win->dimensions.y = 0;
    }
    else if (vertical == 0)
    {
        win->dimensions.y = res.y/2 - win->dimensions.h/2;
    }
    else
    {
        win->dimensions.y = res.y - win->dimensions.h;
    }
}

void gf2d_window_justify(Window *win,int horizontal)
{
    GFC_Vector2D res;
    if (!win)return;
    res = gf3d_vgraphics_get_resolution();
    if (horizontal < 0)
    {
        win->dimensions.x = 0;
    }
    else if (horizontal == 0)
    {
        win->dimensions.x = res.x/2 - win->dimensions.w/2;
    }
    else
    {
        win->dimensions.x = res.x - win->dimensions.w;
    }
}

void gf2d_window_calibrate(Window *win)
{
    GFC_Vector2D res;
    if (!win)return;
    res = gf3d_vgraphics_get_resolution();
    if ((win->dimensions.x > 0)&&(win->dimensions.x < 1.0))
    {
        win->dimensions.x *= res.x;
    }
    if ((win->dimensions.y > 0)&&(win->dimensions.y < 1.0))
    {
        win->dimensions.y *= res.y;
    }
    if ((win->dimensions.w > 0)&&(win->dimensions.w <= 1.0))
    {
        win->dimensions.w *= res.x;
    }
    if ((win->dimensions.h > 0)&&(win->dimensions.h <= 1.0))
    {
        win->dimensions.h *= res.y;
    }
    
    if (win->dimensions.x < 0)
    {
        win->dimensions.x = res.x + win->dimensions.x;
    }
    if (win->dimensions.y < 0)
    {
        win->dimensions.y = res.y + win->dimensions.y;
    }
}


Window *gf2d_window_load(char *filename)
{
    Window *win = NULL;
    SJson *json;
    json = gfc_pak_load_json(filename);
    if (!json)return NULL;
    win = gf2d_window_load_from_json(json);
    sj_free(json);
    if (win)
    {
        if (!strlen(win->name))gfc_line_cpy(win->name,filename);
        gf2d_window_make_focus_list(win);
    }
    return win;
}

int gf2d_window_named(Window *win,const char *name)
{
    if (!win)return 0;
    if (!name)return 0;
    if (strcmp(win->name,name)==0)return 1;
    return 0;
}

Window *gf2d_window_load_from_json(SJson *json)
{
    Window *win = NULL;
    int i,count;
    short int buul = 0;
    GFC_Vector4D gfc_vector = {255,255,255,255};
    SJson *elements,*value;
    const char *buffer;
    if (!json)
    {
        slog("json not provided");
        return NULL;
    }
    json = sj_object_get_value(json,"window");
    if (!json)
    {
        slog("json does not contain window definition");
        return NULL;
    }
    win = gf2d_window_new();
    if (!win)
    {
        slog("failed to create new window");
        return NULL;
    }
    
    buffer = sj_object_get_value_as_string(json,"name");
    if (buffer)gfc_line_cpy(win->name,buffer);
    sj_get_bool_value(sj_object_get_value(json,"no_draw_generic"),&buul);
    if (buul)win->no_draw_generic = 1;
    sj_value_as_vector4d(sj_object_get_value(json,"color"),&gfc_vector);
    win->color = gfc_color_from_vector4(gfc_vector);
    
    gfc_vector4d_clear(gfc_vector);
    sj_value_as_vector4d(sj_object_get_value(json,"dimensions"),&gfc_vector);
    win->dimensions = gfc_rect(gfc_vector.x,gfc_vector.y,gfc_vector.z,gfc_vector.w);
    gf2d_window_calibrate(win);
    
    value = sj_object_get_value(json,"justify");
    buffer = sj_get_string_value(value);
    if (buffer)
    {
        if (strcmp(buffer,"left") == 0)
        {
            gf2d_window_justify(win,-1);
        }
        else if (strcmp(buffer,"center") == 0)
        {
            gf2d_window_justify(win,0);
        }
        else if (strcmp(buffer,"right") == 0)
        {
            gf2d_window_justify(win,1);
        }
    }

    value = sj_object_get_value(json,"align");
    buffer = sj_get_string_value(value);
    if (buffer)
    {
        if (strcmp(buffer,"top") == 0)
        {
            gf2d_window_align(win,-1);
        }
        else if (strcmp(buffer,"middle") == 0)
        {
            gf2d_window_align(win,0);
        }
        else if (strcmp(buffer,"bottom") == 0)
        {
            gf2d_window_align(win,1);
        }
    }

    elements = sj_object_get_value(json,"elements");
    count = sj_array_get_count(elements);
    for (i = 0; i< count; i++)
    {
        value = sj_array_get_nth(elements,i);
        if (!value)continue;
        gf2d_window_add_element(win,gf2d_element_load_from_config(value,NULL,win));
    }
    return win;
}

Element *gf2d_window_get_element_by_name(Window *win,const char *name)
{
    Element *e,*q;
    int count, i;
    if (!win)return NULL;
    count = gfc_list_get_count(win->elements);
    for (i = 0;i < count;i++)
    {
        e = (Element *)gfc_list_get_nth(win->elements,i);
        if (!e)continue;
        q = gf2d_get_element_by_name(e,name);
        if (q)return q;
    }
    return NULL;
}

void gf2d_window_bring_to_front(Window *win)
{
    gfc_list_delete_data(window_manager.window_deque,win);
    gfc_list_append(window_manager.window_deque,win);
}

void gf2d_window_send_to_back(Window *win)
{
    gfc_list_delete_data(window_manager.window_deque,win);
    gfc_list_prepend(window_manager.window_deque,win);
}

Element *gf2d_window_get_element_by_id(Window *win,int id)
{
    Element *e,*q;
    int count, i;
    if (!win)return NULL;
    count = gfc_list_get_count(win->elements);
    for (i = 0;i < count;i++)
    {
        e = (Element *)gfc_list_get_nth(win->elements,i);
        if (!e)continue;
        q = gf2d_element_get_by_id(e,id);
        if (q)return q;
    }
    return NULL;
}

int gf2d_window_mouse_in(Window *win)
{
    if (!win)return 0;
    return gf2d_mouse_in_rect(win->dimensions);
}

/*eol@eof*/
