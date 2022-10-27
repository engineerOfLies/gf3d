#ifndef __GF2D_ELEMENTS_H__
#define __GF2D_ELEMENTS_H__

/**
 * @purpose gf2d elements are window elements like labels, buttons, status bars, etc
 * This is a generic container class for all other elements
 * There are going to be 
 */

#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_color.h"
#include "gfc_config.h"
#include "gfc_shape.h"

#include "gf2d_sprite.h"
#include "gf2d_windows.h"

typedef enum
{
    ES_idle,        /**<normal*/
    ES_disable,     /**<greyed out*/
    ES_highlight,   /**<selected*/
    ES_hidden,
    ES_active       /**<pressed or whatever*/
}ElementState;

typedef enum
{
    EUR_None,
    EUR_Handled
}ElementUpdateReturn;

typedef enum
{
    ET_List,
    ET_Label,
    ET_Actor,
    ET_Button,
    ET_Entry,
    ET_Percent
}ElementTypes;

struct Element_S
{
    int      index;             /**<order of highlight in the menu, -1 for does not receive highlight*/
    TextLine name;              /**<name of the element should be unique per window*/
    
    Uint8   canHasFocus;        /**<if true, this element can be the focus for keyboard input*/
    Uint8   hasFocus;           /**<if true, this element does have focus*/
    Rect bounds;                /**<drawing bounds for the element*/
    Vector2D lastDrawPosition;  /**<location of the element on the screen of its last draw position*/
    Color color;                /**<color for the element*/
    
    Color backgroundColor;      /**<color for background of element*/
    int   backgroundDraw;       /**<if true, draw the background*/
    
    int state;                  /**<if true, drawn with highlight*/
    int type;                   /**<which type of element this is*/
    void (*draw)        (struct Element_S *element,Vector2D offset); /**<draw function, offset comes from draw position of window*/
    struct Element_S *(*get_next)(struct Element_S *element,struct Element_S *from); /**<search for the next element from (if NULL, it returns itself*/
    List *(*update)     (struct Element_S *element,Vector2D offset); /**<function called for updates  returns alist of all elements updated with input*/
    void (*free_data)   (struct Element_S *element);    /**<free function for the element to clean up any loaded custom data*/
    struct Element_S *(*get_by_name)(struct Element_S *element,const char *name);/**<get element by name, searches sub elements as well*/
    Window *win;                /**<my parent window*/
    void *data;                 /**<custom element data*/
};

/**
 * @brief allocate and initialize a new element
 * @return NULL on error or a new element otherwise;
 */
Element *gf2d_element_new();

/**
 * @brief allocate and set values for a new element
 * @param index the index of the element
 * @param name the name of the element
 * @param bounds the drawing bounds of the element
 * @param color the dra    return NULL;
w color of the element
 * @param state the initial state of the element
 * @param backgroundColor the color to draw for the background
 * @param backgroundDraw if true, draw a background for the element
 * @param win the window this element ultimately belongs to
 * @return NULL on error or a new element otherwise;
 */
Element *gf2d_element_new_full(
    Element *parent,
    int      index,
    TextLine name,
    Rect bounds,
    Color color,
    int state,
    Color backgroundColor,
    int backgroundDraw,
    Window *win
);

/**
 * @brief free an element.  Calls the custom free element as needed
 * @param element the element to free
 */
void gf2d_element_free(Element *element);

/**
 * @brief draw an element.  Calls the custom draw function
 * @param element the element to draw
 * @param offset comes from parent window position
 */
void gf2d_element_draw(Element *element, Vector2D offset);

/**
 * @brief get the screen position of the last time this element was drawn
 * @param e the element to draw
 * @return a zero vector on NULL entity, or the entity's position
 */
Vector2D gf2d_element_get_draw_position(Element *e);

/**
 * @brief set if the element has the focus.
 * @param element the element to modify
 * @param focus 1 if it has the keyboard focus, 0 if not.
 * @return returns 0 if the element can't have focus, 1 otherwise after it was set
 */
int gf2d_element_set_focus(Element *element,int focus);

/**
 * @brief set the color for the given element
 * @param element the element to set
 * @param color the color to set it to
 */
void gf2d_element_set_color(Element *element,Color color);

/**
 * @brief update an element.  checks input
 * @param element the element to draw
 * @param offset comes from parent window position
 * @return NULL if there was nothing to report or a pointer to a List of elements that have been updated.  This list needs to be freed
 */
List *gf2d_element_update(Element *element, Vector2D offset);

/**
 * @brief create a new element based on the information from a json config
 * @param json the json data to use to define the element
 * @param parent the parent element to this element
 * @param win the owning window
 * @return NULL on error or a newly configured element
 */
Element *gf2d_element_load_from_config(SJson *json,Element *parent,Window *win);

Rect gf2d_element_get_absolute_bounds(Element *element,Vector2D offset);

/**
 * @brief checks element and sub elements for element with the provided id
 * @param e the element to check
 * @param id the id to seach for
 * @return NULL on not found, or error, a pointer to the element with the matching id otherwise
 */
Element *gf2d_element_get_by_id(Element *e,int id);

/**
 * @brief get this element or any sub element that has the name searching for
 * @note: matches on the first occurance
 * @param e the element to check
 * @param name the name to search for
 * @returns NULL if not found, or a pointer to the element found
 */
Element *gf2d_get_element_by_name(Element *e,const char *name);

#endif
