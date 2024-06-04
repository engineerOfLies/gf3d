#ifndef __GF2D_ACTOR_H__
#define __GF2D_ACTOR_H__

#include <SDL.h>

#include "simple_json.h"

#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_vector.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"

typedef struct
{
    int             _refCount;      /**<set if the actor is in use*/
    GFC_TextLine    filename;       /**<filename of the actor json file*/
    Sprite         *sprite;         /**<which sprite to draw this entity with*/
    GFC_TextLine    spriteFile;
    int             frameWidth;
    int             frameHeight;
    int             framesPerLine;
    GFC_Vector2D    size;           /**<scaled heigth and width*/
    GFC_Vector2D    scale;          /**<scale to draw at*/
    GFC_Vector2D    center;         /**<center for rotation and scale*/
    GFC_Color       color;
    GFC_Vector2D    drawOffset;
    GFC_ActionList *al;             /**<action list for managing sprite animations*/
}Actor;

/**
 * @brief initialize the actor subsystem
 * @param max the maximum number of concurrent actors to support
 */
void gf2d_actor_init(Uint32 max);

/**
 * @brief free all loaded actors
 */
void gf2d_actor_clear_all();

/**
 * @brief load an actor from json file
 * @note if its already in memory you get a reference to that one
 * @param file the json file to load it from
 * @return NULL on error or the actor otherwise
 */
Actor *gf2d_actor_load(const char *file);

/**
 * @brief parse json into an actor
 * @note the actor->filename will not be set through this call
 * @note the should follow this format
 * {
        "actor":
        {
            "sprite": "images/ui/pointer.png",
            "frameWidth": 16,
            "frameHeight": 16,
            "framesPerLine": 1,
            "scaleTo": [64,64], 
            "scale":[2,2],      //overridden by scaleTo
            "color": [255,255,255,255], //defaults to white (no color shift)
            "actionGFC_List":
            [
                {
                    "action": "default",
                    "startFrame": 0,
                    "endFrame": 0,
                    "frameRate": 0,  //factor for advancing animation frames
                    "actionTime": 14 //how many frames this animation should take to complete one pass.  Overrides frameRate
                    "type": "loop"  //"loop" or "pass" for an animation that loops or pass for a single pass through then stop
                }
            ]
        }
    }
 * @param json the json to parse
 */
Actor *gf2d_actor_load_json(SJson *json);

/**
 * @brief get an empty actor
 * @return NULL on error or out of memory, a blank actor otherwise
 */
Actor *gf2d_actor_new();

/**
 * @brief free a previously loaded actor
 * @param actor the actor to free
 */
void gf2d_actor_free(Actor *actor);


/**
 * @brief get a previously loaded actor by filename
 * @param filename the search criteria
 * @return NULL on not found or the actor otherwise
 */
Actor *gf2d_actor_get_by_filename(const char * filename);

/**
 * @brief save actor information to file
 * @param actor the actor to save
 * @param filename the file to save to
 */
void gf2d_actor_save(Actor *actor,const char *filename);

/**
 * @brief get an action from an actor by its name
 * @param actor the actor to query
 * @param name the search criteria
 * @return NULL on not found or error, the action otherwise
 */
GFC_Action *gf2d_actor_get_action_by_name(Actor *actor,const char *name);

/**
 * @brief get how many actions are in the actor
 * @param actor the actor to query
 * @return the number of actions (it will be zero if anything went wrong)
 */
Uint32 gf2d_actor_get_action_count(Actor *actor);

/**
 * @brief get an action by its index in the actor
 * @param actor the actor to query
 * @param index which action to get
 * @return NULL on error or if the index is out of range, the action otherwise
 */
GFC_Action *gf2d_actor_get_action_by_index(Actor *actor,Uint32 index);

/**
 * @brief get the next action for the actor, it will loop back to start
 * @param actor the actor to query
 * @param action the current action, if NULL you get the first action
 * @return NULL on an error, or the action in question otherwise
 */
GFC_Action *gf2d_actor_get_next_action(Actor *actor,GFC_Action *action);

/**
 * @brief get how many frames have been configured for the actor
 * @note there may be more or less frames in the actual sprite, this just about what has been configured
 * @param actor the actor to query
 * @return the frame count (not adjusted for frame rate)
 */
Uint32 gf2d_actor_get_framecount(Actor *actor);

/**
 * @brief draws an actor to the current rendering context
 * @param actor which actor to draw with
 * @param frame use this frame of animation
 * @param position where on the screen to draw it to
 * @param scale (optional) if set the actor will be scaled accordingly
 * @param center (optional) if set it will use this point for scale and rotation reference
 * @param rotation (optional) if set it will rotate accordingly
 * @param color (optional) color is used as a modulation (limited)
 * @param flip (optional) if x or y is non-zero it will flip in that direction
 */
void gf2d_actor_draw(
    Actor *actor,
    float frame,
    GFC_Vector2D position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    float    * rotation,
    GFC_Color    * color,
    GFC_Vector2D * flip
);

/**
 * @brief get an action for an actor by name
 * @param actor the actor in question
 * @param name the name of the action
 * @param frame (optional) if provided, the starting frame will be set here
 * @return NULL on error or not found, the action otherwise
 */
GFC_Action *gf2d_actor_get_action(Actor *actor, const char *name,float *frame);

#endif
