#ifndef __STATION_DEF_H__
#define __STATION_DEF_H__

#include "simple_json.h"
#include "gfc_list.h"

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

/**
 * @brief get the number of extension points by section name
 * @param name the name to search for
 * @return the number of extensions
 */
int station_def_get_extension_count_by_name(const char *name);

/**
 * @brief given a section name, get its display name
 * @param section the name of the section
 * @return NULL if not found, or the str otherwise.
 */
const char *station_def_get_display_name(const char *section);

const char *station_def_name_by_display(const char *display);

/**
 * @brief check if a station section is a unique one
 * @note unique ones cannot be bought or sold
 * @param name the section to check
 * @return 0 if not or error, 1 if it is unique
 */
int station_def_is_unique(const char *name);

/**
 * @brief get a list of section names
 * @return NULL if none loaded, or a list (the list should be deleted, but the items should not)
 */
List *station_def_get_section_list();

/**
 * @brief get a list of resources that the section in question costs
 * @note free the list when done with resources_list_free();
 * @param name the section name to get the cost for
 * @return NULL on error or a list of resources otherwise.
 */
List *station_get_resource_cost(const char *name);

#endif
