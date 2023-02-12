// need to make a clip mask system that can be configured
#ifndef __GF3D_CLIPLAYERS_H__
#define __GF3D_CLIPLAYERS_H__

#include "simple_json.h"
#include "gfc_types.h"

/**
 * @brief configure the clip layers from a config file
 * @param file the config file containing the clip layer list
 * example:
 * "clipLayers":[
 *      "monster_layer",
 *      "player_layer",
 *      "pickup_layer",
 *      "object_layer"
 * ]
 */
void gf3d_cliplayers_init(const char *file);

/**
 * @brief based on loaded config, configure a clip layer mask from config
 * @param layers a list of named layers
 * @return a bit mas based on the layers.
 */
Uint32 gf3d_cliplayers_from_config(SJson *layers);

#endif
