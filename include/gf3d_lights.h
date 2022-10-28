#ifndef __GF3D_LIGHTS_H__
#define __GF3D_LIGHTS_H__

#include "gf3d_vgraphics.h"

typedef struct
{
    Uint8   _inuse;
    Vector4D color;
    Vector3D position;
    Vector3D direction;     //for alignment
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
void gf3d_lights_set_global_light(Vector4D color,Vector3D direction);

#endif
