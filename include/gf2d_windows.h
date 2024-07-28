#ifndef __GF2D_MENUS_H__
#define __GF2D_MENUS_H__

#include <simple_json.h>
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_shape.h"

#include "gf2d_sprite.h"

typedef struct Element_S Element;

typedef enum
{
    BE_TL,
    BE_Top,
    BE_TR,
    BE_Left,
    BE_Right,
    BE_BL,
    BE_Bottom,
    BE_BR
}BorderElements;

typedef struct Window_S
{
    int                 _inuse;             /**<do not touch*/
    int                 hidden;             /**<if true, no drawing*/
    int                 disabled;           /**<if true, no drawing or updating*/
    GFC_TextLine        name;       /**<name of window*/
    int                 no_draw_generic;    /**<if true, do not use the generic window draw style*/
    GFC_List           *elements;         /**<all the components of the window*/
    GFC_List           *focus_elements;   /**<pointers to all of the elements that can have focus*/
    Element            *focus;         /**<this element has the focus*/
    GFC_Rect            dimensions;        /**<where on the screen*/
    GFC_Rect            canvas;            /**<Where within the window we draw things*/
    GFC_Color           color;         /**<color to draw the window with*/
    int                 blocks_input;       /**<if true, windows below will not be checked for input updates, but will update*/
    struct Window_S    *parent;/**<pointer to a parent window*/
    struct Window_S    *child; /**<pointer to a child window, used when only one at a time is allowed*/
    void              (*close_child)(struct Window_S *win,struct Window_S *child);
    int               (*update)(struct Window_S *win,GFC_List *updateList);//update function to be called whenever a window element is updated
    int               (*draw)(struct Window_S *win);                   //custom draw function, if it returns 1, skip the standard draw
    void              (*refresh)(struct Window_S *win);                //custom refresh function.  Called when window content is changed
    int               (*free_data)(struct Window_S *win);              //if you have custom data, you need to specify this to free it
    void               *data;             /**<custom data*/       
}Window;

/**
 * @brief initialize the window system
 * @param max_windows the limit of active windows
 * @param config [optional] if provided, this will attempt to load the json file to configure the defaul window
 */
void gf2d_windows_init(int max_windows,const char *config);

/**
 * @brief draw all active windows
 */
void gf2d_windows_draw_all();

/**
 * @brief update all active windows
 * @return 1 if a window handled input or 0 if everything was idle
 */
int gf2d_windows_update_all();

/**
 * @brief call the window's refresh function, if one has been specified
 */
void gf2d_window_refresh(Window *win);

/**
 * @brief find the first window by the name provided and run its refresh function if provided
 * @param name the name of the window to refresh
 */
void gf2d_window_refresh_by_name(const char *name);

/**
 * @brief get a new initialized window
 * @return NULL on error or a window pointer
 */
Window *gf2d_window_new();

/**
 * @brief load a window config from file and return a handle to it
 * @param the filename of the config file
 * @return NULL on error or a window pointer on success
 */
Window *gf2d_window_load(char *filename);

/**
 * @brief free a window no longer in use
 * @param win the window to free
 */
void gf2d_window_free(Window *win);

/**
 * @brief let a parent know that the child window is closing and close the child
 * @param parent the parent of the closing window
 * @param child the window that is closing
 */
void gf2d_window_close_child(Window *parent,Window *child);

/**
 * @brief add a gui element to the window
 * @param win the window to add an element to
 * @param w the element to add
 */
void gf2d_window_add_element(Window *win,Element *e);

/**
 * @brief should be called after an element is added or removed from a window
 * to keep the focus list current
 * @param win the window to update
 */
void gf2d_window_make_focus_list(Window *win);

/**
 * @brief update a window and all of its elements
 * @param win the window to update
 * @return 1 if the window in question should block input to windows below it
 */
int gf2d_window_update(Window *win);

/**
 * @brief check if the name of the window is the name in question
 * @param win the window to check
 * @param name the name to check against
 * @return 0 if no match or error, 1 if match
 */
int gf2d_window_named(Window *win,const char *name);

/**
 * @brief play one of the windows sounds by name
 * @param name the name of the sound to play
 */
void gf2d_windows_play_sound(const char *name);

/**
 * @brief hide and disable a window
 * @param win the window
 */
void gf2d_window_hide(Window *win);

/**
 * @brief enable a hidden window
 * @param win the window
 */
void gf2d_window_unhide(Window *win);

/**
 * @brief toggle a window's hidden state
 * @param win the window
 */
void gf3d_window_hide_toggle(Window *win);

/**
 * @brief draw a window to the screen.  
 * @note: This is done automatically for windows without a custom draw function or if that function returns 0
 * @param win the window to draw
 */
void gf2d_window_draw(Window *win);

/**
 * @brief draw a window given the border sprites and background image
 * @param border the sprite containing the border elements
 * @param bg the sprite to use for the background image (it will be stretch to match the rect
 * @param rect the dimensions of the window to draw
 * @param color the color to draw the window with
 */
void gf2d_draw_window_border_tiled(Sprite *border,Sprite *bg,GFC_Rect rect,GFC_Color color);
void gf2d_draw_window_border_stretched(Sprite *border,Sprite *bg,GFC_Rect rect,GFC_Color color);

/**
 * @brief draw a generic window using the common border assets
 * @param rect the dimensions of the window to draw
 * @param color the color to draw the window with
 */
void gf2d_draw_window_border_generic(GFC_Rect rect,GFC_Color color);

/**
 * @brief get the element from the window with the matching id
 * @param win the window to query
 * @param id the index to search for
 * @returns NULL on error or not found, a pointer to the element otherwise
 */
Element *gf2d_window_get_element_by_id(Window *win,int id);

/**
 * @brief get the element from the window with the matching name
 * @note it gets the FIRST occurance of the name so try to make them unique
 * @param win the window to query
 * @param name the name to search for
 * @returns NULL on error or not found, a pointer to the element otherwise
 */
Element *gf2d_window_get_element_by_name(Window *win,const char *name);

/**
 * @brief get the window element with the focus
 * @param win the window to query
 * @return NULL on no element with focus, or a pointer to the element otherwise
 */
Element *gf2d_window_get_element_by_focus(Window *win);

/**
 * @brief set the element with the id to the element with keyboard focus
 * @param win the window to modify
 * @param e the element to set the focus to
 */
void gf2d_window_set_focus_to(Window *win,Element *e);

/**
 * @brief change the focus of the keyboard to the next viable element
 * @param win the window to cycle
 */
void gf2d_window_next_focus(Window *win);

/**
 * @brief change the focus of the keyboard to the previous viable element
 * @param win the window to cycle
 */
void gf2d_window_prev_focus(Window *win);

/**
 * @brief change the draw order by bringing the window to the front
 * @param win the window 
 */
void gf2d_window_bring_to_front(Window *win);

/**
 * @brief change the draw order by sending the window to the back
 * @param win the window 
 */
void gf2d_window_send_to_back(Window *win);

/**
 * @brief check if the mouse is currently over the window specified
 * @param win the window to check;
 * @return 0 if the mouse is over the window, 1 otherwise
 */
int gf2d_window_mouse_in(Window *win);

/**
 * @brief set the upper left position of a window
 * @param win the window to move
 * @param position the place to the move the window to
 */
void gf2d_window_set_position(Window *win,GFC_Vector2D position);

/**
 * @brief change a window's dimensions
 * @param win the window to resize
 * @param dimensions the dimensions of the window
 */
void gf2d_window_set_dimensions(Window *win,GFC_Rect dimensions);

/**
 * @brief get the first window in the window list by the name provided
 * @param name the search criteria
 * @return NULL on error or not found, the window otherwise
 */
Window *gf2d_window_get_by_name(const char *name);

/**
 * @brief check to make sure the win pointer and the data pointer on the window is valid
 * @param win the window to check
 * @param name if not NULL, it will also check the win->name against the name provided.
 * @return 1 if everything checks out, 0 if not.
 */
int gf2d_window_check(Window *win,const char *name);


#endif
