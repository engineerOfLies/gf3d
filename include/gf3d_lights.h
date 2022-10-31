#ifndef __GF3D_LIGHTS_H__
#define __GF3D_LIGHTS_H__

#include "gf3d_vgraphics.h"
#include "gf3d_mesh.h"

typedef struct
{
    Uint8   _inuse;
    Vector4D color;
    Vector4D position;
}Gf3D_Light;

/**
 * @brief initialize the lighting system
 */
void gf3d_lights_init(Uint32 max_light);

/**
 * @brief sets a single global light source from infinitely far away
 * @param color the color of the light
 * @param direction the direction of the light
 */
void gf3d_lights_set_global_light(Vector4D color,Vector4D direction);

/**
 * @brief get the stats for a single global light source
 * @param color populated with the color of the global light
 * @param direciton the unit vector for the direction of the light
 */
void gf3d_lights_get_global_light(Vector4D *color, Vector4D *direction);

#endif
