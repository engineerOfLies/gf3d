#ifndef __GF2D_ELEMENT_LIST_H__
#define __GF2D_ELEMENT_LIST_H__

#include "gfc_list.h"
#include "gf2d_elements.h"
/**
 * @example
 * 
 * {
        "type": "list",
        "name": "listName",         //OPTIONAL for referencing
        "id" : 0,                   //OPTIONAL for referencing
        "bounds": [0,4,1,1],        //remember things within 0.0-1.0 are percentages of parent
        "color" : [255,255,255,255],
        "style": "vertical",        //horizontal is also an option
        "item_size" : [1,24],       //default size of items
        "packed":true,              //OPTIONAL if false, item size is used for spacing
        "cropped":true,             //OPTIONAL if true, cut off any list items that exceed bounds
        "scrolls":true,             //OPTIONAL, only matters if crapped.  Allows items to scroll within bounds
        "scroolbar":{},             //if scrolling, use this scrollbar element
        "elements":                 // list of other elements
        []
 * 
 */


typedef enum
{
    LS_Horizontal,
    LS_Vertical
}GFC_ListStyle;

typedef struct
{
    GFC_List *list;
    GFC_Vector2D itemSize;
    GFC_ListStyle listStyle;
    int   cropped;          /**<if true, check for items being out bounds of the list, skip the ones that are*/
    int   packed;           /**<if true, items are spaced by their individual size, otherwise by the list itemSize*/
    int   wraps;            /**<if true, this will wrap when the items clip the edge*/
    int   scrolls;          /**<if true, allow for scrolling position*/
    int   scrollOffset;     /**<offset for drawing based on scrolling position*/
    int   itemsPerLine;     /**<how many items will fit per line*/
    int   itemsPerColumn;   /**<how many items will fit per columns*/
    Element *scrollbar;     /**<sub_element*/
}GF2D_ListElement;


/**
 * @brief make a new list element based on the parameters provided
 * @param bounds the size of the list
 * @param itemSize dimensions for uniform items in the list
 * @param ls the style of the list (LS_Horizontal or LS_Vertical)
 * @param wraps if the list should wrap around if it exceeds the width
 * @param scrolls if the list should scroll (not yet implemented)
 * @param packed if the items should be spaced according to their own size or the itemSize
 * @param packed if true, items will be skipped if they are out of bounds of the list
 * @return NULL on error or a formatted listElement otherwise
 */
GF2D_ListElement *gf2d_element_list_new_full(
    GFC_Rect bounds,
    GFC_Vector2D itemSize,
    GFC_ListStyle ls,
    int wraps,
    int scrolls,
    int packed,
    int cropped);

/**
 * @brief free all of the elements in the list, leaving the list element, but empty
 * @param list the list element to clear our
 */
void gf2d_element_list_free_items(Element *list);

/**
 * @brief add an element to the list
 * @param e the list to add to
 * @param item the item to add
 */
void gf2d_element_list_add_item(Element *e,Element *item);

/**
 * @brief set an element to be the list provided
 * @param e the element to set
 * @param list the list to set it too
 */
void gf2d_element_make_list(Element *e,GF2D_ListElement *list);

/**
 * @brief load list configuration for a list element from config
 * @param e the element to configure
 * @param json the json config to use
 */
void gf2d_element_load_list_from_config(Element *e,SJson *json,Window *win);

/**
 * @brief seach an element list for an item with id provided
 * @param e the list element to search
 * @param id the index to search for
 * @returns NULL on not found or error, the element with index that matches otherwise
 */
Element *gf2d_element_list_get_item_by_id(Element *e,int id);

/**
 * @brief set the scroll offset
 */
void gf2d_element_list_set_scroll_offset(Element *element,int offset);

/**
 * @brief get the number of rows that can be drawn at once based on itemSize
 * @param element the list element
 * @return 0 on error or zero itemSize.y, number of items that can be drawn otherwise
 */
int gf2d_element_list_get_row_count(Element *element);

/**
 * @brief get the number of items in the element list
 * @param element the element list
 * @return the number of items in the list
 */
int gf2d_element_list_get_item_count(Element *element);

/**
 * @brief get the number of items that fit per line in a wrapping list
 * @param e the list element to check
 * @returns 0 on error, the number of items otherwise
 */
int gf2d_element_list_get_items_per_line(Element *e);

/**
 * @brief get the number of items that fit per column in a wrapping list
 * @param e the list element to check
 * @returns 0 on error, the number of items otherwise
 */
int gf2d_element_list_get_items_per_column(Element *e);

/**
 * @brief get the configured size of each item for a list element
 * @param e the list element 
 * @return (-1,-1) on error or the size othereise
 */
GFC_Vector2D gf2d_element_list_get_item_size(Element *e);

/**
 * @brief make a new list element, and specify all of its parameters
 */
Element *gf2d_element_list_new_complete(
    Element *parent,
    Window *win,
    int      index,
    GFC_TextLine name,
    GFC_Rect bounds,
    GFC_Color color,
    GFC_Color backgroundGFC_Color,
    int backgroundDraw,
    GFC_Vector2D itemSize,
    GFC_ListStyle ls,
    int wraps,
    int scrolls,
    int packed,
    int cropped);

#endif
