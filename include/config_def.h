#ifndef __CONFIG_DEF_H__
#define __CONFIG_DEF_H__

#include "simple_json.h"
#include "gfc_types.h"

/**
 * @purpose This will be used to quickly and easily pull config data from json files.
 * This will return the json object data for items from named lists of data.
 * Example:
 * {
 *
 *  "colors":[
 *      {
 *          "name":"blue",
 *          "any other data":"stuff"
 *      }
 * ]
 * }
 * You can load multiple files that all follow this format and the system will keep track of them and search them all.
 * As long as the list names never repeat, this will work
 */

/**
 * @brief initialize the internals of the config def system
 */
void config_def_init();

/**
 * @brief load config definition lists for a game resource
 * @param filename the json file containing the info
 */
void config_def_load(const char *filename);

/**
 * @brief get definition information for a given resource by its place in the list
 * @param resource the name of the resource list
 * @param index the search item
 * @return NULL if not found or error, the JSON otherwise.  DO NOT FREE IT, you do not own it.
 */
SJson *config_def_get_by_index(const char *resource,Uint8 index);

/**
 * @brief get definition information for a given resource by the "name" key
 * @param resource the name of the resource list
 * @param name the search item
 * @return NULL if not found or error, the JSON otherwise.  DO NOT FREE IT, you do not own it.
 */
SJson *config_def_get_by_name(const char *resource,const char *name);

/**
 * @brief get the name of a resource by its index
 * @param resource the name of the resource list
 * @param index the search item
 * @return NULL if not found or error, the name of the resource.  DO NOT FREE IT, you do not own it.
 */
const char *config_def_get_name_by_index(const char *resource,Uint8 index);

/**
 * @brief get definition name for a given resource by the "display_name" key
 * @param section the display name of the resource
 * @return NULL if not found or error, the name otherwise
 */
const char *station_def_get_name_by_display(const char *section);

/**
 * @brief get definition information for a given resource by the parameter key and name value
 * @param resource the name of the resource list
 * @param parameter the field to match with
 * @param name the search item
 * @return NULL if not found or error, the JSON otherwise.  DO NOT FREE IT, you do not own it.
 */
SJson *config_def_get_by_parameter(const char *resource,const char *parameter,const char *name);

/**
 * @brief get a specific parameter from a named resource by its key
 * @param resource the name of the resource list
 * @param name the search item
 * @param key the search item
 * @return NULL if not found, the json object value that matches the key otherwise
 */
SJson *config_def_get_value(const char *resource, const char *name, const char *key);

/**
 * @brief get the number of a resources that has been loaded
 * @param resource the resource type to check
 * @return 0 if not loaded or error, the count otherwise
 */
Uint32 config_def_get_resource_count(const char *resource);

#endif
