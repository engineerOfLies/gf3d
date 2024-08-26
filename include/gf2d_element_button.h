#ifndef __GF2D_ELEMENT_BUTTON_H__
#define __GF2D_ELEMENT_BUTTON_H__

#include "gf2d_elements.h"
#include "gf2d_font.h"

/**
 * @example json
{
    "type": "button",                       //must be set to be a button
    "name":"newgame",                       //standard
    "id" : 1,                               //standard
    "canHasFocus":true,                     //if you are making a game with keyboard/controller menu nav, this is a must
    "sound":"confirm",                      //play this default sound when pressed
    "bounds": [0,0,160,42],                 //standard bounds of the element
    "hotkey":"newgame",                     //this hotkey should match a named input
    "highColor":[255,255,255,255],          //[optional] if set, the text will be in this color when highlighting
    "pressColor":[0,0,0,255],               //[optional] if set, the text will be in this color when pressed
    "repeat":true,                          //[optional] if set key repeat will be enabled for the button
    "customActions":0,                      //[optional] values must match ButtonCustomAction enum
    "actor" :                               //[optional] if you want an animated button button graphic.
    {
        "type": "actor",
        "actor" : "actors/button.actor"     // the actor needs to have defined these actions: idle, high, press
    },
    "label" :                               //[optional] if you want text to show up on your button
    {
        "type": "label",
        "bounds": [0,0,160,48],
        "text":"New Game",
        "justify" : "center",
        "align" : "middle",
        "style":"H4"
    }
}
 */


typedef enum
{
    BE_Hidden = 0,
    BE_Text = 1,
    BE_Actor = 2,
    BE_Both = 3
}BE_Style;

typedef enum
{
    BCA_Default = 0,
    BCA_Nothing,        // do nothing at all
    BCA_BackgroundHighlight, //use the highGFC_Colors for the background
    BCA_MAX
}ButtonCustomAction;

typedef struct
{
    Element            *label;
    Element            *actor;
    BE_Style            style;
    GFC_TextLine        hotkey; /**<input used to hotkey the button*/
    Bool                repeat; /**<if true, enable continuous updates while held*/
    ButtonCustomAction  customActions;
    // base color comes from element
    GFC_Color           highColor;    /**<color used when button is in highlight*/
    GFC_Color           pressColor;   /**<color used while pressed*/
    GFC_TextLine        sound;//sound file to play when pressed
}ButtonElement;

/**
 * @brief sets the element to be the button
 * @note the button is now owned by the element, and should not be freed
 * @param e the element to make into a button
 * @param button the button to use
 */
void gf2d_element_make_button(Element *e,ButtonElement *button);

/**
 * @brief allocate a new button and set the parameters
 * @param label (optional) use this element for the text display for the button
 * @param actor (optional) use this element for the image display for the button
 * @param highGFC_Color this color will be used when drawing the button when it has highlight
 * @param pressGFC_Color this color will be used when drawing the button when it is pressed
 * @param customActions if true, this will not look for actions idle, high and press for the button
 * @return NULL on error or a newly create button element
 */
ButtonElement *gf2d_element_button_new_full(Element *label,Element *actor,GFC_Color highColor,GFC_Color pressColor,int customActions);

/**
 * @brief load button configuration for a button element from config
 * @param e the element to configure
 * @param json the json config to use
 * @param win the parent window
 */
void gf2d_element_load_button_from_config(Element *e,SJson *json,Window *win);

/**
 * @brief search the sub elements of the button for the given id
 * @param e the element to search
 * @param id the id to search for
 * @return NULL on error or not found, the search item otherwise
 */
Element *gf2d_element_button_get_by_id(Element *e,int id);

/**
 * @brief quickly make a label button based on name and color of the text
 * @param win the window to add this to
 * @param index the index for the button
 * @param text the text for the label
 * @param ft the font to use
 * @param size the width and height of the button
 * @param color the color of the text
 * @return NULL on error, or a button element.  It will need to be parented or placed still
 */
Element *gf2d_button_new_label_simple(Window *win,int index,const char *text,FontTypes ft, GFC_Vector2D size, GFC_Color color);

/**
 * @brief quickly make an actor button based on actor, name and color of the text
 * @param win the window to add this to
 * @param index the index for the button
 * @param name the name of the button element for update checks
 * @param actorFile the path to the file where the actor is described
 * @param text the text for the label
 * @param scale how to scale the actor (1,1) is no scale
 * @param size the width and height of the button
 * @param color the color of the text
 * @return NULL on error, or a button element.  It will need to be parented or placed still
 */
Element *gf2d_button_new_simple(
    Window *win,
    int index,
    const char *name,
    const char *actorFile,
    const char *text,
    GFC_Vector2D scale,
    GFC_Vector2D size,
    GFC_Color color);

#endif
