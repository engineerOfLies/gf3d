#include "simple_logger.h"
#include "gfc_input.h"

#include "gf2d_draw.h"
#include "gf2d_element_entry.h"
#include "gf2d_element_label.h"
#include "gf2d_mouse.h"

void gf2d_element_entry_draw(Element *element,GFC_Vector2D offset)
{
    EntryElement *entry;
    GFC_Vector2D position;
    if (!element)return;
    entry = (EntryElement*)element->data;
    if (!entry)return;
    gfc_vector2d_add(position,offset,element->drawBounds);
    gf2d_element_draw(entry->label,position);
    if (!element->hasFocus)
    {
        gf2d_draw_rect(
            gfc_rect(
                element->drawBounds.x + position.x,
                element->drawBounds.y + position.y,
                element->drawBounds.w,
                element->drawBounds.h),
            gfc_color8(150,150,150,255));
    }
    else
    {
        gf2d_draw_rect(
            gfc_rect(
                element->drawBounds.x + position.x,
                element->drawBounds.y + position.y,
                element->drawBounds.w,
                element->drawBounds.h),
            gfc_color8(200,200,200,255));
    }
}

GFC_List *gf2d_element_entry_update(Element *element,GFC_Vector2D offset)
{
    GFC_Rect bounds;
    GFC_List *list;
    GFC_TextLine input = {0};
    char newLet = 0;
    SDL_Keymod mods;
    EntryElement *entry;
    if (!element)return NULL;
    entry = (EntryElement*)element->data;
    if (!entry)return NULL;
    bounds = gf2d_element_get_absolute_bounds(element,offset);
    if(gf2d_mouse_in_rect(bounds))
    {
        if (gf2d_mouse_button_pressed(0))
        {
//            slog("giving entry focus");
            gf2d_window_set_focus_to(element->win,element);
        }
    }
    // check for keyboard input
    if (entry->text == NULL)
    {
        slog("no text buffer provided for entry");
        return NULL;
    }
    if (element->hasFocus)
    {
        mods = SDL_GetModState();
        if (entry->cursor_pos < (GFCTEXTLEN -1))
        {
            for (input[0] = 'a';input[0] <= 'z';input[0]++)
            {
                if (gfc_input_key_pressed(input))
                {
                    if (mods & KMOD_SHIFT)
                    {
                        newLet = input[0] - 'a' + 'A';
                    }
                    else
                    {
                        newLet = input[0];
                    }
                }
            }
            for (input[0] = '0';input[0] <= '9';input[0]++)
            {
                if (gfc_input_key_pressed(input))
                {
                    if (mods & KMOD_SHIFT)
                    {
                        switch(input[0])
                        {
                            case '0':
                                newLet = ')';
                                break;
                            case '1':
                                newLet = '!';
                                break;
                            case '2':
                                newLet = '@';
                                break;
                            case '3':
                                newLet = '#';
                                break;
                            case '4':
                                newLet = '$';
                                break;
                            case '5':
                                newLet = '%';
                                break;
                            case '6':
                                newLet = '^';
                                break;
                            case '7':
                                newLet = '&';
                                break;
                            case '8':
                                newLet = '*';
                                break;
                            case '9':
                                newLet = '(';
                                break;
                        }
                    }
                    else
                    {
                        newLet = input[0];
                    }
                }
            }
            if (gfc_input_key_pressed("`"))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '~';
                }
                else newLet = '`';
            }
            if (gfc_input_key_pressed(";"))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = ':';
                }
                else newLet = ';';
            }
            if (gfc_input_key_pressed("="))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '+';
                }
                else newLet = '=';
            }
            if (gfc_input_key_pressed("-"))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '_';
                }
                else newLet = '-';
            }
            if (gfc_input_key_pressed("["))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '{';
                }
                else newLet = '[';
            }
            if (gfc_input_key_pressed("]"))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '}';
                }
                else newLet = ']';
            }
            if (gfc_input_key_pressed("."))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '>';
                }
                else newLet = '.';
            }
            if (gfc_input_key_pressed(","))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '<';
                }
                else newLet = ',';
            }
            if (gfc_input_key_pressed(" "))
            {
                newLet = ' ';
            }
            if (gfc_input_key_pressed("\'"))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '"';
                }
                else newLet = '\'';
            }
            if (gfc_input_key_pressed("\\"))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '|';
                }
                else newLet = '\\';
            }
            if (gfc_input_key_pressed("/"))
            {
                if (mods & KMOD_SHIFT)
                {
                    newLet = '?';
                }
                else newLet = '/';
            }
            if (newLet != 0)
            {
                entry->text[entry->cursor_pos++] = newLet;
                entry->text[entry->cursor_pos] = '\0';
                gf2d_element_label_set_text(entry->label,entry->text);
                list = gfc_list_new();
                gfc_list_append(list,element);
                return list;
            }
        }
        if (entry->cursor_pos > 0)
        {
            if (gfc_input_key_pressed("BACKSPACE"))
            {
                entry->text[--entry->cursor_pos] = '\0';
                gf2d_element_label_set_text(entry->label,entry->text);
                list = gfc_list_new();
                gfc_list_append(list,element);
                return list;
            }
        }
    }
    return NULL;
}

Element *entry_get_by_name(Element *e,const char *name)
{
    EntryElement *entry;
    Element *r;
    if (!e)return NULL;
    entry = (EntryElement*)e->data;
    r = gf2d_get_element_by_name(entry->label,name);
    if (r)return r;
    return NULL;
}

void gf2d_element_entry_free(Element *element)
{
    EntryElement *entry;
    if (!element)return;
    entry = (EntryElement*)element->data;
    if (entry != NULL)
    {
        gf2d_element_free(entry->label);
        free(entry);
    }
}

Element *gf2d_element_entry_get_next(Element *element,Element *from)
{
    EntryElement *entry;
    if (!element)return NULL;
    entry = (EntryElement*)element->data;
    if (!entry)return NULL;
    if (element == from)
    {
        return entry->label;
    }
    if (entry->label == from)return from;
    return NULL;
}

void gf2d_element_make_entry(Element *e,EntryElement *entry)
{
    if (!e)return;
    e->data = entry;
    e->type = ET_Entry;
    e->draw = gf2d_element_entry_draw;
    e->update = gf2d_element_entry_update;
    e->free_data = gf2d_element_entry_free;
    e->get_by_name = entry_get_by_name;
    e->get_next = gf2d_element_entry_get_next;
    e->canHasFocus = 1;
}

EntryElement *gf2d_element_entry_new()
{
    EntryElement *entry;
    entry = (EntryElement *)malloc(sizeof(EntryElement));
    if (!entry)
    {
        slog("failed to allocate memory for entry");
        return NULL;
    }
    memset(entry,0,sizeof(EntryElement));
    return entry;
}

EntryElement *gf2d_element_entry_new_full(Element *label)
{
    EntryElement *entry;
    entry = gf2d_element_entry_new();
    if (!entry)return NULL;
    entry->label = label;
    return entry;
}

void gf2d_element_entry_set_text_pointer(Element *e,char *text,size_t len)
{
    EntryElement *entry;
    if (!e)return;
    entry = (EntryElement*)e->data;
    if (entry == NULL)
    {
        return;
    }
    entry->text = text;
    entry->max_len = len;
    gf2d_element_label_set_text(entry->label,text);
    entry->cursor_pos = strlen(text);
}

void gf2d_element_load_entry_from_config(Element *e,SJson *json,Window *win)
{
    Element *label = NULL;
    SJson *value;
    
    if ((!e) || (!json))
    {
        slog("call missing parameters");
        return;
    }
    
    value = sj_object_get_value(json,"label");
    if (value)
    {
        label = gf2d_element_load_from_config(value,e,win);
    }
    gf2d_element_make_entry(e,gf2d_element_entry_new_full(label));
}

/*eol@eof*/
