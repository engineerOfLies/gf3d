#ifndef __GF3D_VGRAPHICS_H__
#define __GF3D_VGRAPHICS_H__

#include "gf3d_vector.h"

void gf3d_vgraphics_init(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen
);

void gf3d_vgraphics_clear();

void gf3d_vgraphics_render();


#endif
