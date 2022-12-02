#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_list.h"

typedef struct
{
    TextLine    name;
    float       amount;
}Resource;

/**
 * @brief load the internal definitions for the resources
 */
void resources_list_load();

/**
 * @brief allocate a new resource List
 * @return the new empty list
 */
List *resources_list_new();

/**
 * @brief free a resource list
 * @param list the list to free
 */
void resources_list_free(List *list);

/**
 * @brief give count of a resource to a list
 * @param list the list to add to
 * @param name the name of the resource
 * @param count how much to give
 * @return list the new resource list.  Old may be destroyed
 */
List *resources_list_give(List *list,const char *name,float count);

/**
 * @brief query how much of a resource a list contains
 * @param list the list to check
 * @param name the name of the resource
 * @return how much
 */
float resources_list_get_amount(List *list,const char *name);

/**
 * @brief get the resource struct
 * @param list the list to get it from
 * @param name the name to get
 * @return NULL if not found, the resource otherwise
 */
Resource *resources_list_get(List *list,const char *name);

/**
 * @brief get the resource definition data by its name
 */
SJson *resources_get_def(const char *name);

/**
 * @brief pull out the resources and counts from a json object
 * format:
 * {
 *  "name1":<count>,
 *  "name2":<count>,etc
 * }
 * @param config the json object containing resources
 * @return NULL if not valid config or the resource list otherwise
 */
List *resources_list_parse(SJson *config);

/**
 * @brief convert a resource list into a json object
 * @param list the resource list to convert
 * @return NULL on failure or a json object with resource information otherwise
 */
SJson *resources_list_save(List *list);

#endif
