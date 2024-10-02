#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "gfc_vector.h"

#define  LIGHT_UBO_MAX 16

typedef struct
{
    GFC_Vector4D    lightPos;
    GFC_Vector4D    lightDir;
    GFC_Vector4D    lightColor;
    float           angle;
    float           brightness;
    float           falloff;
    float           inUse;         // if non zero, the light should be used
}Light;


#endif
