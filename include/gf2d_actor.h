#ifndef __GF2D_ACTOR_H__
#define __GF2D_ACTOR_H__

#include <SDL.h>

#include "simple_json.h"

#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_vector.h"

#include "gf2d_sprite.h"


typedef enum
{
    AT_NONE = 0,        /**<not intended to animate*/
    AT_LOOP,        /**<animation is meant to loop*/
    AT_PASS,        /**<animaiton is a single pass and meant to end*/
    AT_MAX
}ActionType;

typedef enum
{
    ART_ERROR,      /**<something went wrong*/
    ART_NORMAL,     /**<still running fine*/
    ART_END,        /**<a Pass animation has completed*/
    ART_START       /**<just started*/
}ActionReturnType;

/**
 * @brief this structure describes an individual action
 */
typedef struct Action_S
{
    TextLine    name;
    int         startFrame;
    int         endFrame;
    float       frameRate;
    ActionType  type;
}Action;

typedef struct
{
    int         _refCount;      /**<set if the actor is in use*/
    TextLine    filename;       /**<filename of the actor json file*/
    Sprite     *sprite;         /**<which sprite to draw this entity with*/
    TextLine    spriteFile;
    int         frameWidth;
    int         frameHeight;
    int         framesPerLine;
    Vector2D    size;           /**<scaled heigth and width*/
    Vector2D    scale;          /**<scale to draw at*/
    Vector2D    center;         /**<center for rotation and scale*/
    Color       color;
    Vector2D    drawOffset;
    List       *al;             /**<action list for managing sprite animations*/
}Actor;

/**
 * @brief initialize the actor subsystem
 * @param max the maximum number of concurrent actors to support
 */
void gf2d_actor_init(Uint32 max);

/**
 * @brief free a previously loaded actor
 * @param actor the actor to free
 */
void gf2d_actor_free(Actor *actor);

/**
 * @brief delete all actions in the provided list, deletes the list too
 * @param list list contain Actions that will be deleted
 */
void gf2d_action_list_delete(List *list);

/**
 * @brief parse out an action list from json
 * @param al list to be populated with actions
 * @param actionList json contain a list of actions
 * @return the parsed list
 */
List *gf2d_action_list_parse(List *al,SJson *actionList);

/**
 * @brief encode an action list into json
 * @param actions list of Actions to encode
 * @return NULL on error or SJSon
 */
SJson *gf2d_action_list_to_json(List *actions);

/**
 * @brief given a list of Actions, search for the name
 * @param list list containing actions
 * @param name the search criteria
 * @return NULL on error or not found, the Action otherwise
 */
Action *gf2d_action_list_get_action_by_name(List *list,const char *name);

/**
 * @brief call if you insert an animation frame and need to keep the actions in line
 * @param list the action list to modify
 * @param index the index of the frame that was inserted
 */
void gf2d_action_list_frame_inserted(List *list,Uint32 index);

/**
 * @brief call if you delete an animation frame and need to keep the actions in line
 * @param list the action list to modify
 * @param index the index of the frame that was deleted
 */
void gf2d_action_list_frame_deleted(List *list,Uint32 index);

/**
 * @brief free all loaded actors
 */
void gf2d_actor_clear_all();

/**
 * @brief get an empty actor
 * @return NULL on error or out of memory, a blank actor otherwise
 */
Actor *gf2d_actor_new();

/**
 * @brief allocate a new action
 * @return NULL on error a new initialized action
 */
Action *gf2d_action_new();

/**
 * @brief get a previously loaded actor by filename
 * @param filename the search criteria
 * @return NULL on not found or the actor otherwise
 */
Actor *gf2d_actor_get_by_filename(const char * filename);

/**
 * @brief parse a text name into its actionType
 * @param text the text to parse (should be "pass", "loop" or "none"
 * @return the actiontype
 */
ActionType gf2d_actor_type_from_text(const char *text);

/**
 * @brief given an actiontype, get the text associated with it
 * @param type the action type
 * @return the name of the action typee
 */
const char *gf2d_actor_type_to_text(ActionType type);


/**
 * @brief save actor information to file
 * @param actor the actor to save
 * @param filename the file to save to
 */
void gf2d_actor_save(Actor *actor,const char *filename);

/**
 * @brief search an action list by name
 * @param al a list of Action
 * @param name the search criteria
 * @return NULL on error or not found, the action otherwise
 */
Action *gf2d_action_list_get_action(List *al, const char *name);

/**
 * @brief given an action get the next frame from the current frame
 * @param action the action to base the animation on
 * @param frame (input and output) given this starting frame, this frame will be set to the next frame
 * @return if not an ART_ERROR, it will let you know its return status.  
 */
ActionReturnType gf2d_action_next_frame(Action *action,float *frame);

/**
 * @brief given a frame, what is the next whole number frame that will come afterwards
 * @param action the action to query
 * @param frame the current frame
 * @return which whole number frame would be next
 */
Uint32 gf2d_action_next_frame_after(Action *action,float frame);

/**
 * @brief load an actor from file
 * @note if its already in memory you get a reference to that one
 * @param file the json file to load it from
 * @return NULL on error or the actor otherwise
 */
Actor *gf2d_actor_load(const char *file);

/**
 * @brief get an action from an actor by its name
 * @param actor the actor to query
 * @param name the search criteria
 * @return NULL on not found or error, the action otherwise
 */
Action *gf2d_actor_get_action_by_name(Actor *actor,const char *name);

/**
 * @brief get an action from an actor by the frame number
 * @note if there are any overlapping frames, this returns the first in the list
 * @param list the action list to query
 * @param frame the frame to search fo
 * @return NULL on error or no results, the action otherwise
 */
Action *gf2d_action_list_get_action_by_frame(List *list,Uint32 frame);

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
Action *gf2d_actor_get_action_by_index(Actor *actor,Uint32 index);

/**
 * @brief get the next action for the actor, it will loop back to start
 * @param actor the actor to query
 * @param action the current action, if NULL you get the first action
 * @return NULL on an error, or the action in question otherwise
 */
Action *gf2d_actor_get_next_action(Actor *actor,Action *action);

/**
 * @brief given an action, check how many frames of animation it has
 * @note this is not adjusted for frame rate
 * @param action the action to query
 * @return the difference between the action's start and end frames
 */
Uint32 gf2d_action_get_framecount(Action *action);

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
    Vector2D position,
    Vector2D * scale,
    Vector2D * center,
    float    * rotation,
    Color    * color,
    Vector2D * flip
);

/**
 * @brief given the action, get the number of frames of animation (accounting for frame rate)
 * @note: this DOES take into consideration frame Rate.  SO its more like the number of calls to next_frame
 * before the animation is completed
 * @param action the action in question
 * @return the number of frames (or calls to next_frame), -1 on error
 */
int gf2d_action_get_animation_frames(Action *action);

/**
 * @brief given the action and current frame, get the number of frames left
 * @note: this DOES take into consideration frame Rate.  SO its more like the number of calls to next_frame
 * before the animation is completed
 * @param action the action in question
 * @param frame the current frame
 * @return the number of frames (or calls to next_frame)before the current action is completed
 */
int gf2d_action_get_frames_remaining(Action *action,float frame);

/**
 * @brief returns the percentage of completion of the current action in integer space (0-100)
 * @note: this is for synching timing events
 * @param action the action in question
 * @param frame the current frame
 */
int gf2d_action_get_percent_complete(Action *action,float frame);

/**
 * @brief get the number of frames into the action we are based on the current frame
 * @param action the action in question
 * @param frame the current rendering frame
 * @return -1 on error or no frame rate, the number of frames into the action otherwise
 * @note:this DOES take into consideration frame Rate.  SO its more like the number of calls to next_frame since the action began
 */
int gf2d_action_get_action_frame(Action *action,float frame);

/**
 * @brief set an action for an actor by name
 * @param actor the actor in question
 * @param name the name of the action
 * @param frame (optional) if provided, the starting frame will be set here
 * @return NULL on error or not found, the action otherwise
 */
Action *gf2d_actor_set_action(Actor *actor, const char *name,float *frame);

#endif
