#include "simple_logger.h"

#include "gf3d_materials.h"



MaterialUBO gf3d_material_make_basic(GFC_Color diffuse)
{
    MaterialUBO material = {0};
    material.ambient = gfc_vector4d(1,1,1,1);
    material.specular = gfc_vector4d(1,1,1,1);
    material.diffuse = gfc_color_to_vector4f(diffuse);
    material.shininess = 128;
    material.transparency = 1.0;
    return material;
}

/*eol@eof*/
