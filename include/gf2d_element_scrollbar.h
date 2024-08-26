#ifndef __GF2D_ELEMENT_SCROLLBAR_H__
#define __GF2D_ELEMENT_SCROLLBAR_H__

#include "gf2d_elements.h"
#include "gf2d_element_list.h"

/**
 * @example
    {
        "type":"scrollbar",
        "name": "scrollbarName",    //OPTIONAL for referencing
        "id" : 0,                   //OPTIONAL for referencing
        "bounds": [0,4,1,1],        //remember things within 0.0-1.0 are percentages of parent
        "color" : [255,255,255,255],
        "list_target":"list element name", //this scrollbar will scroll for the provided element named list
        "style": "vertical",        //horizontal is also an option
    }
 */

typedef struct
{
    int position;           //our current scroll position
    int scrollCount;        //how much we can scroll
    Element *scrollUp;      //or left
    Element *scrollDown;    //or right
    Element *scrollSlider;  //The thing that indicates position
    Element *list;          //the list we are scrolling for.  We don't own this, we just point to it
    GFC_ListStyle scrollStyle;
    Bool drawSlider;         //if we should draw the slider or not
}ScrollbarElement;

Element *gf2d_element_scrollbar_new_simple(Element *list,GFC_Rect bounds);

ScrollbarElement *gf2d_element_scrollbar_new();

/**
 * @brief set which list this scrollbar is working for
 */
void gf2d_element_scrollbar_set_list_target(Element *scrollbar,Element *list);

/**
 * @brief set an element to be the scrollbar provided
 * @param e the element to set
 * @param scrollbar the scrollbar to set it too
 */
void gf2d_element_make_scrollbar(Element *e,ScrollbarElement *scrollbar);

/**
 * @brief load scrollbar configuration for an scrollbar element from config
 * @param e the element to configure
 * @param json the json config to use
 */
void gf2d_element_scrollbar_load_from_config(Element *e,SJson *json,Window *win);

#endif
