#ifndef __GF3D_MATERIALS_H__
#define __GF3D_MATERIALS_H__


#include "gfc_types.h"
#include "gfc_color.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"

typedef struct
{
    GFC_Vector4D    ambient;        //how much ambient light affects this material
    GFC_Vector4D    diffuse;        //how much diffuse light affects this material - primary influcen for material color
    GFC_Vector4D    specular;       //color of the shine on the materials
    float           transparency;   //how translucent the material should be overall
    float           shininess;      //how shiny the materials is.  // how pronounced the specular is
    GFC_Vector2D    padding;        //for alignment

}MaterialUBO;


MaterialUBO gf3d_material_make_basic(GFC_Color diffuse);


#endif
