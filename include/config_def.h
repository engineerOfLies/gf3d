#ifndef __CONFIG_DEF_H__
#define __CONFIG_DEF_H__

#include "simple_json.h"

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
#endif
