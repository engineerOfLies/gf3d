#ifndef __GF2D_FIGURE_H__
#define __GF2D_FIGURE_H__

/**
 * @purpose group armature animations and sprite pieces together
 */

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"

#include "gf2d_sprite.h"
#include "gf2d_actor.h"
#include "gf2d_armature.h"


typedef struct
{
    GFC_TextLine    bonename;       /**<name of the bone this is associated with*/
    GFC_TextLine    actorname;      /**<name of the bone this is associated with*/
    GFC_TextLine    action;         /**<name of the action to use*/
    GFC_Vector2D    offset;         /**<position offset from the bone root*/
    float           rotation;       /**<rotation offset from the bone*/
    GFC_Vector2D    scale;          /**<scale*/
    Bone           *bone;           /**<link to the bone*/
}FigureLink;

typedef struct
{
    int             _inuse;       /**<set if the actor is in use*/
    GFC_TextLine    filename;     /**<the filename*/
    GFC_TextLine    name;         /**<figure name*/
    Armature2D     *armature;     /**<which armature is used for this figure*/
    GFC_List       *links;        /**<list of FigureLinks to draw this with*/
}Figure;

// below are for keeping track of individual instanes of figures
typedef struct
{
    Actor          *actor;          /**<loaded actor file for drawing*/
    GFC_Action     *action;         /**<configured action*/
    float           frame;          /**<animation frame*/
    FigureLink     *link;           /**<pointer to the link*/
}FigureLinkInstance;

typedef struct
{
    Uint8    _inuse;
    Figure     *figure;
    GFC_List   *instances;
    GFC_Action *action;
    float       frame;
    Uint32      nextFrame;
}FigureInstance;

/**
 * @brief initialize the figure management system
 * @param maxFigures how many figures to support concurently
 */
void gf2d_figure_init(Uint32 MaxFigures);

/**
 * @brief allocate memory for a new figure
 * @return NULL on out of memory, a Figure otherwise
 */
Figure *gf2d_figure_new();

/**
 * @brief load a figure from config file
 * @param filepath the filename/path to load from
 * @return NULL on error or the loaded figure otherwise
 */
Figure *gf2d_figure_load(const char *filepath);

/**
 * @brief save a figure to a config file
 * @param figure the figure to save
 * @param filepath location and filename to save to
 */
void gf2d_figure_save(Figure *figure,const char *filepath);

/**
 * @brief free a figure from memory
 * @param figure the figure to free
 */
void gf2d_figure_free(Figure *figure);

/**
 * @brief set the current action of the figure
 * @param figure the figure to set
 * @param name the name of the action to set it to
 * @param float (optional) if provided, the frame will be set to the start of the action
 * @return the new action
 */
GFC_Action *gf2d_figure_get_action(Figure *figure, const char *name,float *frame);

/**
 * @brief get the action from the figure by its index
 * @param figure the figure to query
 * @param index the index of the action to get
 * @return NULL on bad index or no actions or no figure
 */
GFC_Action *gf2d_figure_get_action_by_index(Figure *figure,Uint32 index);

/**
 * @brief free a figure link and clean up its data
 * @param link A pointer to the link to free.  point is set to NULL
 */
void gf2d_figure_link_free(FigureLink **link);

/**
 * @brief remove a link from a figure and delete it
 * @param figure the figure with the link
 * @param link the link to remove
 */
void gf2d_figure_link_remove(Figure *figure,FigureLink *link);

/**
 * @brief call whenever you set the armature for a figure to establish links
 * @param figure the figure to link
 * @param armature the new armature to link.  Can be NULL if the figure already has an armature
 */
void gf2d_figure_link_armature(Figure *figure,Armature2D *armature);

/**
 * @brief reload actor and action for a link after it's information has changed
 * @param linkInstance the link instance to update
 */
void gf2d_figure_link_instance_update_link(FigureLinkInstance *linkInstance);

/**
 * @brief allocate a new FigureLink
 * @note it sets some default information like color and scale
 * @return NULL on error or a new FigureLink otherwise
 */
FigureLink *gf2d_figure_link_new();

/**
 * @brief create a link to a bone
 * @param figure if provided, the bone will be added to this figure
 * @note if the figure has an armature, that link will be made as well
 * @param bone name of the bone
 * @param actor name of the actor
 * @param action name of the action
 */
FigureLink *gf2d_figure_add_link_to_bone(Figure *figure, const char *bone,const char *actor,const char *action);

/**
 * @brief search a figure for a link by the name of its bone
 * @param figure the figure to query
 * @param bonename the name of the bone to search for
 * @return NULL on error or not found, the FigureLink otherwise
 */
FigureLink *gf2d_figure_link_get_by_bonename(Figure *figure,const char *bonename);

/**
 * @brief set a figure link's actor and action
 * @param link the link to set
 * @param actor the name of the actorfile to set it too
 * @param action the name of the action to specify
 */
void gf2d_figure_link_set_actor(FigureLink *link,const char *actor,const char *action);

/**
 * @brief add a link to the instance
 * @note: loads what needs to be loaded
 * @param instance pointer to the instance to add to
 * @param link the link to add
 */
void gf2d_figure_instance_add_link(FigureInstance *instance, FigureLink *link);

/**
 * @brief remove a link from an instance
 * @note the instance link is deleted as well as the link from the figure
 * @param instance the instance to remove the link from
 * @param link the linkinstance to remove
 */
void gf2d_figure_instance_remove_link(FigureInstance *instance,FigureLinkInstance *link);

/**
 * @brief change the actor/action for a figure link instance
 * @note it the link in question doesn't exist, none is created
 * @param instance the instance to change
 * @param name the name of the link to change
 * @param actor the name of the actor to change it to, if NULL, the actor is cleared (but the link remains)
 * @param action the action to use for the given actor
 */
void gf2d_figure_instance_link_change_actor(FigureInstance *instance,const char *name,const char *actor, const char *action);

/**
 * @brief get a link instance from a figure instance by the name of the bone
 * @param instance the instance to search
 * @param name the search criteria
 * @return NULL on error or not found, the link otherwise
 */
FigureLinkInstance *gf2d_figure_instance_get_link(FigureInstance *instance, const char *name);

/**
 * @brief initial setup of an instance
 * @param instance the instance to setup
 * @note: if _inuse is true, this is a no-op
 */
void gf2d_figure_instance_new(FigureInstance *instance);

/**
 * @brief clean up an instance
 * @param instance the instance to free
 */
void gf2d_figure_instance_free(FigureInstance *instance);

/**
 * @brief link a figure to an instance
 * @note instance->figure should be loaded, if its NULL this removes all links
 * @param instance the instance to link
 */
void gf2d_figure_instance_link(FigureInstance *instance);

/**
 * @brief draw a figure to the screen at the specified location in link order
 * @param instance the figure to draw
 * @param position relative to this position
 * @param scale (optional) and scale it
 * @param rotation (optional) and rotate it
 * @param color (optional) and modulate the color
 * @param tween if true, use tweening between poses
 */
void gf2d_figure_instance_draw(
    FigureInstance *instance,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    float *rotation,
    GFC_Color *color,
    int tween);

/**
 * @brief draw a figure link to the screen at the specified location
 * @param figure the figure to draw from
 * @param link the link to draw
 * @param poseIndex which pose to use (tweening allowed)
 * @param position relative to this position
 * @param scale (optional) and scale it
 * @param rotation (optional) and rotate it
 * @param color (optional) and modulate the color
 */
void gf2d_figure_link_draw(
    Figure *figure,
    FigureLink *link,
    float       poseIndex,
    GFC_Vector2D    position,
    GFC_Vector2D   *scale,
    float      *rotation,
    GFC_Color      *color);

#endif
