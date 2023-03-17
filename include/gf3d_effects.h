#ifndef __GF3D_EFFECTS_H__
#define __GF3D_EFFECTS_H__

#include "simple_json.h"

#include "gfc_vector.h"
#include "gfc_color.h"
#include "gf3d_particle.h"
#include "gf3d_model.h"

typedef enum
{
    GF3D_ET_Particle = 0,
    GF3D_ET_Line,
    GF3D_ET_Model,
    GF3D_ET_Sprite,
    GF3D_ET_MAX,
}GF3DEffectType;


typedef struct
{
    Uint8           _inuse;
    Sint32          ttd;                // when this time is passed, delete this effect
    ModelMat        mat;                // for keeping track of model data`
    Particle        particle;           // particle data, partical data
    GF3DEffectType  eType;              // which type of effect this one is
    Uint16          fadein;             // if true, start fading in from zero alpha to full by the time this has ended
    Uint16          fadeout;            // if true, start fading out when there is this much time left to live
    float           sizeDelta;          // for particles, this changes the size of the particle to draw over time
    Vector3D        velocity;           // movement of the effect
    Vector3D        acceleration;       // acceleration of the effect
    Color           color;              // color mod for the effect
    Color           colorVector;        // color delta to be applied over time
    Color           colorAcceleration;  // color delta delta to be applied over time
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

GF3DEffect *gf3d_effect_new_particle(
    Vector3D position,
    Vector3D velocity,
    Vector3D acceleration,
    float size,
    float sizeDelta,
    Color color,
    Color colorVector,
    Color colorAcceleration,
    Sint32 ttl);

/**
 * @brief make a new particle based on a json config
 */
GF3DEffect *gf3d_effect_from_config(SJson *config);

/**
 * @brief while most effects will timeout on their own
 */
void gf3d_effect_free(GF3DEffect *effect);

#endif
