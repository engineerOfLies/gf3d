#ifndef __GF3D_MATERIALS_H__
#define __GF3D_MATERIALS_H__

#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_color.h"
#include "gfc_string.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"


/**
 * @brief the structure as it appears in the shaders.  Order must match what is in the vertex and fragment shaders
 */
typedef struct
{
    GFC_Vector4D    ambient;        //how much ambient light affects this material
    GFC_Vector4D    diffuse;        //how much diffuse light affects this material - primary influcen for material color
    GFC_Vector4D    specular;       //color of the shine on the materials
    GFC_Vector4D    emission;       //color that shows regardless of light
    float           transparency;   //how translucent the material should be overall
    float           shininess;      //how shiny the materials is.  // how pronounced the specular is
    GFC_Vector2D    padding;        //for alignment
}MaterialUBO;

/**
 * @brief the structure to hold material data loaded from file or to adjust them
 */
typedef struct
{
    GFC_String     *filename;       //name of the file this was loaded from
    GFC_String     *name;           //name of the material
    Uint32          _refCount;      //how many things want this resource
    GFC_Color       ambient;        //how much ambient light affects this material
    GFC_Color       diffuse;        //how much diffuse light affects this material - primary influcen for material color
    GFC_Color       specular;       //color of the shine on the materials
    GFC_Color       emissive;       //color of the emission
    Uint32          illum;          //illumination flags
    float           specularExponent;//specular power
    float           ior;            //index of refraction
    float           transparency;   //how translucent the material should be overall
    float           shininess;      //how shiny the materials is.  // how pronounced the specular is
}GF3D_Material;


/**
 * @brief initialize the materials manager and queue up the cleanup atexit
 */
void gf3d_materials_init();

/**
 * @brief clear all loaded materials from memory, keeping the system initialized
 * @note should called after purging an entire scene
 */
void gf3d_materials_clear();

/**
 * @brief extract material information from json
 * @param json the json to extract from
 * @param filename the filename to attach to this material for searching
 * @return NULL on failure, or the extracted material file otherwise
 */
GF3D_Material *gf3d_material_parse_js(SJson *json,const char *filename);

/**
 * @brief allocate and initialize a blank material
 * @return NULL on error or a blank material
 */
GF3D_Material *gf3d_material_new();

/**
 * @brief return a material resource you no longer need
 * @param material the material to free
 */
void gf3d_material_free(GF3D_Material *material);

/**
 * @brief given a material, get the MaterialUBO to describe it
 * @param material the material to convert
 * @return a zero material if no material provided, or one set to the provided one.
 */
MaterialUBO gf3d_material_get_ubo(GF3D_Material *material);

/**
 * @brief return a UBO based on a basic color
 * @param diffuse the color to set the diffuse color to
 * @return a default UBO that has set the diffuse to the color provided
 */
MaterialUBO gf3d_material_make_basic_ubo(GFC_Color diffuse);

/**
 * @brief create a new copy of an existing material
 * @note in case you want to make color variants for things
 * @param from the material to copy
 * @return NULL on error, or a copy of the original material
 * @note the name of the material will be appended with ".dup"
 */
GF3D_Material *gf3d_material_duplicate(GF3D_Material *from);

/**
 * @brief load a material from json file
 * @note the object key must be named "material"
 * @note if already loaded, gives back a reference to the exiting one
 * @param filename the json file containing the material definition
 * @return NULL on error or not found, the material otherwise
 * @note free with gf3d_material_free()
 */
GF3D_Material *gf3d_material_load(const char *filename);

/**
 * @brief search the loaded materials for the material by its name.  
 * @note there may be duplicate names.  You may need to use gf3d_material_get_by_file_name to specify which file it came from
 * @param name the name of the material to find
 * @return NULL if not found, or a pointer to the first material with that name otherwise
 */
GF3D_Material *gf3d_material_get_by_name(const char *name);

/**
 * @brief search the loaded materials for the material by its filename
 * @param filename the file that provided this material
 * @return NULL if not found, or a pointer to the material otherwise
 */
GF3D_Material *gf3d_material_get_by_file_name(const char *filename);

/**
 * @brief load the material information from an obj mtl file.
 * @note this doesn't return anything, but all the materials are loaded into memory
 * @note they can be retreived with calls to gf3d_material_get_by_file_name or gf3d_material_get_by_name
 * @param filename the file to load and parse
 */
void gf3d_material_load_mtl_file(const char *filename);
#endif
