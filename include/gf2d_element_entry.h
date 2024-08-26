#ifndef __GF2D_ELEMENT_ENTRY_H__
#define __GF2D_ELEMENT_ENTRY_H__


#include "gf2d_elements.h"


typedef struct
{
    Element *label;
    Uint8 cursor_pos;
    char *text;         /**<pointer to string to modify*/
    size_t max_len;     /**<how many letter to support for text entry*/
}EntryElement;

/**
 * @brief sets the element to be the entry
 * @note the entry is now owned by the element, and should not be freed
 * @param e the element to make into a entry
 * @param entry the entry to use
 */
void gf2d_element_make_entry(Element *e,EntryElement *entry);

/**
 * @brief allocate a new entry and set the parameters
 * @param label (optional) use this element for the text display for the entry
 * @return NULL on error or a newly create entry element
 */
EntryElement *gf2d_element_entry_new_full(Element *label);

/**
 * @brief load entry configuration for a entry element from config
 * @param e the element to configure
 * @param json the json config to use
 * @param win the parent window
 */
void gf2d_element_load_entry_from_config(Element *e,SJson *json,Window *win);

/**
 * @brief set the output position for the text entry
 */
void gf2d_element_entry_set_text_pointer(Element *e,char *text,size_t len);


#endif
