#ifndef __GF3D_EFFECTS_H__
#define __GF3D_EFFECTS_H__

#include "simple_json.h"

#include "gfc_vector.h"
#include "gfc_actions.h"
#include "gfc_color.h"
#include "gfc_callbacks.h"
#include "gfc_primitives.h"

#include "gf2d_actor.h"
#include "gf3d_draw.h"
#include "gf3d_particle.h"
#include "gf3d_model.h"

typedef enum
{
    GF3D_ET_Particle = 0,
    GF3D_ET_Line,
    GF3D_ET_Model,
    GF3D_ET_MAX,
}GF3DEffectType;


typedef struct
{
    Uint8           _inuse;
    Sint32          ttd;                // when this time is passed, delete this effect
    ModelMat        mat;                // for keeping track of model data`
    GFC_Edge3D      edge;               // for drawing lines
    Particle        particle;           // particle data, partical data
    Actor          *actor;              // for animated particles
    Sprite         *image;              // for non animated particles
    GFC_Action     *action;             // current action
    float           frame;              // for animations
    float           size;               // how wide a line is, how large a particle is
    float           sizeVector;         // for particles, this changes the size of the particle to draw over time
    float           sizeAcceleration;   // for particles, this changes the change of size of the particle to draw over time
    GF3DEffectType  eType;              // which type of effect this one is
    Uint8           fadein;             // if true, start fading in from zero alpha to full by the time this has ended
    Uint8           fadeout;            // if true, start fading out when there is this much time left to live
    GFC_Vector3D    velocity;           // movement of the effect
    GFC_Vector3D    acceleration;       // acceleration of the effect
    GFC_Color       color;              // color mod for the effect
    GFC_Color       colorVector;        // color delta to be applied over time
    GFC_Color       colorAcceleration;  // color delta delta to be applied over time
    GFC_Callback    callback;           // if a callback has been set for when a particle dies
}GF3DEffect;

/**
 * @brief initialize the effect manager subsystem
 * @param maxEffect the upper limit for displaying effects concurrently.  Effects that have timed out will not count against this limit.
 */
void gf3d_effect_manager_init(Uint32 maxEffects);

/**
 * @brief since the effects are meant to be dumb and not based on anything, they can be updated and drawn in a single call
 */
void gf3d_effect_manager_draw_all();

/**
 * @brief create a new empty effect
 * @return NULL if we are out of,a pointer to a blank effect otherwise
 */
GF3DEffect *gf3d_effect_new();

/**
 * @brief make a new particle type temproary effect
 * @param position where to start it off from
 * @param velocity how fast it is moving
 * @param acceleration how fast it changes speed
 * @param size how large the particle is
 * @param sizeDelta if the particle size should change over time,  if not set this to 1
 * @param color the color for the particle
 * @param colorGFC_Vector added to the color every update
 * @param colorAcceleration added to the colorGFC_Vector every update
 * @param ttl the number of draw frames this should live for
 */
GF3DEffect *gf3d_effect_new_particle(
    GFC_Vector3D position,
    GFC_Vector3D velocity,
    GFC_Vector3D acceleration,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorGFC_Vector,
    GFC_Color colorAcceleration,
    Sint32 ttl);

/**
 * @brief make a new particle type temproary effect that will move towards a target for its life
 * @param position where to start it off from
 * @param target where the particle should end up
 * @param size how large the particle is
 * @param sizeDelta if the particle size should change over time,  if not set this to 1
 * @param color the color for the particle
 * @param colorGFC_Vector added to the color every update
 * @param colorAcceleration added to the colorGFC_Vector every update
 * @param ttl the number of draw frames before this reaches the target
 */
GF3DEffect *gf3d_effect_new_particle_target(
    GFC_Vector3D position,
    GFC_Vector3D target,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorGFC_Vector,
    GFC_Color colorAcceleration,
    Sint32 ttl);

/**
 * @brief make an explosion of particles from the given point
 * @param position where to spawn the explosion
 * @param size how large the particles should be
 * @param sizeDelta how the size changes over time
 * @param color the base color of the particles
 * @param colorVariation how much variability will be applied to each color
 * @param count how many particles to create
 * @param speed how fast they move
 * @param ttl how long they will last
 */
void gf3d_effect_make_particle_explosion(
    GFC_Vector3D position,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorVariation,
    int count,
    float speed,
    Uint32 ttl);

/**
 * @brief make a line effect
 * @param edge describes the line in 3d space
 * @param velocity how fast it is moving
 * @param acceleration how fast it changes speed
 * @param size how wide the line is
 * @param sizeDelta if the line size should change over time,  if not set this to 1
 * @param color the color for the line
 * @param colorGFC_Vector added to the color every update
 * @param colorAcceleration added to the colorGFC_Vector every update
 * @param ttl the number of draw frames this should live for
 */
GF3DEffect *gf3d_effect_new_line(
    GFC_Edge3D edge,
    GFC_Vector3D velocity,
    GFC_Vector3D acceleration,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorGFC_Vector,
    GFC_Color colorAcceleration,
    Sint32 ttl);


/**
 * @brief set a callback for when a particle dies
 */
void gf3d_effect_set_callback(GF3DEffect *effect,void (*callback)(void *data),void *data);

/**
 * @brief make a new particle based on a json config
 * @param config the json to configure the particle by
 * @param position where the particle should be
 * @param position2 if the particle is a line, this is the second endpoint
 * @param velocity where the particle is moving
 * @param acceleration and how its velocity changes over time
 * @param callback [optional] to be called when the particle expires
 */
GF3DEffect *gf3d_effect_from_config(
    SJson *config,
    GFC_Vector3D position,
    GFC_Vector3D position2,
    GFC_Vector3D velocity,
    GFC_Vector3D acceleration,
    GFC_Callback *callback);

/**
 * @brief emit multiple particles baased on the name of the effect provided.  
 * @param position where to emit from
 * @param direction direciton of emission
 * @param count how many to emit
 * @param effectName the name of the effect in the config_def effects list to use
 * @note you must have used gfc_config_def_load() on a json file that contains a list of "effects"
 * @example
    {
        "name":"flame",
        "ttd":300,
        "ttdVariance":5,
        "type":"particle",
        "color":[255,50,0,128],
        "color2":[255,255,50,128],
        "actor":"actors/flame.actor",
        "action":"default",
        "emitter":"point",       
        "speed":0.0005,
        "speedVariance":0.00025,
        "fadein":0,
        "fadeout":0,
        "size":0.5
    }
    @note emmitter currently supports "square" and point
 */
void gf3d_effect_emit(GFC_Vector3D position, GFC_Vector3D direction,Uint8 count,const char *effectName);


/**
 * @brief while most effects will timeout on their own
 */
void gf3d_effect_free(GF3DEffect *effect);

#endif
