#ifndef __STATION_DEF_H__
#define __STATION_DEF_H__

#include "simple_json.h"

/**
 * @brief load config definitions for the station sections
 * @param filename the json file containing the info
 */
void station_def_load(const char *filename);

/**
 * @brief get the station definition entry by its name
 * @param name the name to search for
 * @return NULL if not found, or a pointer to the data.  
 * @note YOU DO NOT OWN THAT DATA, do not free it!
 */
SJson *station_def_get_by_name(const char *name);

#endif
