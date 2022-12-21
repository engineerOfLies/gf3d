#include <stdio.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_input.h"

#include "gf2d_windows.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_draw.h"
#include "gf2d_mouse.h"
#include "gf2d_windows_common.h"

#include "hud_window.h"

extern void exitGame();
extern void exitCheck();


typedef struct
{
    int selectedOption;
    Sprite *background;
    TextLine filename;
    Window *win;
}MainMenuData;


void onFileLoadCancel(void *Data)
{
    MainMenuData* data;
    if (!Data)return;
    data = Data;
    gfc_line_cpy(data->filename,"saves/");
    return;
}

void onFileLoadOk(void *Data)
{
    MainMenuData* data;
    if (!Data)return;
    data = Data;

    gf2d_window_free(data->win);
    return;
}


void main_menu_start_new_game(const char *savefile)
{
    hud_window(savefile);    
}

int main_menu_free(Window *win)
{
    MainMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    gf2d_sprite_free(data->background);
    free(data);

    return 0;
}

int main_menu_draw(Window *win)
{
    MainMenuData *data;
    if (!win)return 0;
    if (!win->data)return 0;
    data = win->data;
    gf2d_sprite_draw_image(data->background,vector2d(0,0));
    return 0;
}

int main_menu_update(Window *win,List *updateList)
{
    int i,count;
    Element *e;
    MainMenuData* data;
    if (!win)return 0;
    if (!updateList)return 0;
    data = (MainMenuData*)win->data;
    if (!data)return 0;
    
    if (gfc_input_command_pressed("nextelement"))
    {
        gf2d_window_next_focus(win);
        return 1;
    }
    
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"newgame")==0)
        {
            main_menu_start_new_game("saves/default.save");
            gf2d_window_free(win);
            return 1;
        }
        else if (strcmp(e->name,"continue")==0)
        {
            main_menu_start_new_game("saves/quick.save");
            gf2d_window_free(win);
            return 1;
        }
        else if (strcmp(e->name,"load")==0)
        {
            window_text_entry("Enter Save Game to Load", data->filename, win->data, GFCLINELEN, onFileLoadOk,onFileLoadCancel);
            return 1;
        }
        else if (strcmp(e->name,"item_down")==0)
        {
            data->selectedOption++;
            if (data->selectedOption > 4)data->selectedOption = 1;
            gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,data->selectedOption));
            return 1;
        }
        else if (strcmp(e->name,"item_up")==0)
        {
            data->selectedOption--;
            if (data->selectedOption <=0 )data->selectedOption = 4;
            gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,data->selectedOption));
            return 1;
        }
        else if (strcmp(e->name,"quit")==0)
        {
            gf2d_window_free(win);
            exitGame();
            return 1;
        }
    }
    return 0;
}


Window *main_menu()
{
    Window *win;
    MainMenuData* data;
    win = gf2d_window_load("menus/main_menu.json");
    if (!win)
    {
        slog("failed to load editor menu");
        return NULL;
    }
    win->update = main_menu_update;
    win->free_data = main_menu_free;
    win->draw = main_menu_draw;
    data = (MainMenuData*)gfc_allocate_array(sizeof(MainMenuData),1);
    data->background = gf2d_sprite_load_image("images/ui/title_screen.png");
    gfc_line_cpy(data->filename,"saves/");
    gf2d_window_set_focus_to(win,gf2d_window_get_element_by_name(win,"newgame"));
    win->data = data;
    data->win = win;
    return win;
}


/*eol@eof*/
