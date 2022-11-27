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

/**
 * @brief get the station section extension information by its index
 * @param section the section def to check
 * @param index the index of the extension slot
 * @return NULL on error or not found, or the describing json otherwise
 * @note YOU DO NOT OWN THAT DATA, do not free it!
 */
SJson *station_def_get_extension_by_index(SJson *section,Uint8 index);

#endif
