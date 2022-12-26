#ifndef __PLANET_H__
#define __PLANET_H__

#include "simple_json.h"

#include "gfc_vector.h"
#include "gfc_text.h"
#include "gfc_list.h"

#include "gf3d_model.h"

typedef enum
{
    SRT_Nutrients,
    SRT_Minerals,
    SRT_Ores,
    SRT_MAX
}SiteResourceTypes;

typedef struct
{
    int surveyed;
    int resources[SRT_MAX];
    TextLine special;
}SiteData;

#define MAX_LATITUDE 17
#define MAX_LONGITUDE 36

typedef struct
{
    ModelMat mat;//Add This next
    float radius;
    SiteData    sites[MAX_LONGITUDE][MAX_LATITUDE];
    List       *facilities;   /**<facilities installed on the planet*/
}PlanetData;

/**
 * @brief free a loaded planet
 */
void planet_free(PlanetData *planet);

/**
 * @brief make a new blank planet
 */
PlanetData *planet_new();

/**
 * @brief draw the planet
 * @param planet the thing
 */
void planet_draw(PlanetData *planet);

/**
 * @brief draw the facilities on the planet
 * @param planet the thing
 */
void planet_draw_facilities(PlanetData *planet);

void planet_facilities_update(PlanetData *planet);


/**
 * @brief convert a planet data to config
 * @param planet the planet to save
 * @return NULL on error or configured JSON
 */
SJson *planet_save_to_config(PlanetData *planet);

/**
 * @brief load a planet from config
 * @param config the json data to config from
 * @return NULL on error, or the loaded planetData otherwise
 */
PlanetData *planet_load_from_config(SJson *config);

/**
 * @brief reset a planet's resource map back to zero
 */
void planet_reset_resource_map(PlanetData *planet);

/**
 * @brief set random deposits of planetary resources
 */
void planet_generate_resource_map(PlanetData *planet);

/**
 * @brief get a siteData by the world position
 * @param planet the planetData to poll
 * @param position longitude and latitude position on the planet
 * @return NULL if out of range or error, the site data otherwise
 */
SiteData *planet_get_site_data_by_position(PlanetData *planet,Vector2D position);

/**
 * @brief given a position on a planet surface, translate that into a cartesian coordinates relative the planet's center
 * @param radius distance from center of the planet
 * @param coorindate latitude / longitude (in degrees)
 * @return the spacial coordinates relative to planet center
 */
Vector3D planet_position_to_position(float radius, Vector2D coorindate);

/**
 * @brief given a position on a planet's surface, get the rotation angles to align with the sphere
 * @param coordinates where on the planet
 * @return rotation angles to align with planet
 */
Vector3D planet_position_to_rotation(Vector2D position);

#endif
