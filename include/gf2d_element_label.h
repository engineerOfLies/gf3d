#ifndef __GF2D_LABEL_ELEMENT_H__
#define __GF2D_LABEL_ELEMENT_H__

#include "gf2d_font.h"
#include "gf2d_elements.h"

typedef enum
{
    LJ_Left,
    LJ_Center,
    LJ_Right
}LabelJustification;

typedef enum
{
    LA_Top,
    LA_Middle,
    LA_Bottom
}LabelAlignment;

typedef struct
{
    GFC_TextBlock text;     /**<the label text*/
    GFC_Color bgcolor;      /**<background color for the text*/
    int wraps;          /**<if the text should word wrap to match the dimensional width of its element space*/
    int style;          /**<which font style to use*/
    int justify;        /**<leaning left or right*/
    int alignment;      /**<vertical alignment top, middle or bottom*/
}LabelElement;


/**
 * @brief make a simple label element
 * @param win the parent window
 * @param index the identifying index
 * @param text the label text, also the label name
 * @param style the font style FT_H1 for example
 * @param color the draw color
 */
Element *gf2d_label_new_simple(Window *win,int index,const char *text,int style,GFC_Color color);

/**
 * @brief make a simple label element
 * @param win the parent window
 * @param index the identifying index
 * @param text the label text, also the label name
 * @param style the font style FT_H1 for example
 * @param size the size of the label in pixels
 * @param color the draw color
 */
Element *gf2d_label_new_simple_size(Window *win,int index,const char *text,int style,GFC_Vector2D size, GFC_Color color);

/**
 * @brief make a new label element
 * @param text the label text
 * @param color the label text color
 * @param style the font style
 * @param justify the LabelJustification
 * @param align the vertical alignment
 * @param wraps if the label should word wrap
 * @return NULL on error or a new label element
 */
LabelElement *gf2d_element_label_new_full(const char *text,GFC_Color color,int style,int justify,int align,int wraps);

/**
 * @brief set an element to be the label provided
 * @param e the element to set
 * @param label the label to set it too
 */
void gf2d_element_make_label(Element *e,LabelElement *label);

/**
 * @brief load label configuration for a label element from config
 * @param e the element to configure
 * @param json the json config to use
 */
void gf2d_element_load_label_from_config(Element *e,SJson *json);

/**
 * @brief set the label text
 * @param e the element to set the text to
 * @note the label keeps its own copy of the text it will not automatically update
 * @param text the text
 */
void gf2d_element_label_set_text(Element *e,const char *text);

/**
 * @brief set the label font
 * @param e the element to set the font for
 * @param font the font to use
 */
void gf2d_element_label_set_font(Element *e,FontTypes font);

/**
 * @brief enable/disable text wrapping for the label element
 * @param e the element to set
 * @param wraps 1 to enable, 0 to disable
 */
void gf2d_element_label_set_wrap(Element *e,Uint8 wraps);

/**
 * @brief Set the justification and alignment of the text element
 * @param e the element to set
 * @param justify the justification  LJ_Left, LJ_Center, or LJ_Right
 * @param align the alightment LA_Top, LA_Middle, or LA_Bottom
 */
void gf2d_element_label_set_alignment(Element *e,Uint8 justify,Uint8 align);

/**
 * @brief get the label text
 * @param e the element to get the text from
 * @return a pointer to the label text
 */
const char *gf2d_element_label_get_text(Element *e);
#endif
