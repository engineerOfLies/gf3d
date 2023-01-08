#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_list.h"
#include "gf2d_elements.h"
#include "gf2d_windows.h"

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
 * @brief make a separate copy of the resource list
 * @param list the resource list to duplicate
 * @return NULL if the list provided is NULL or out of memory, the resource list copy otherwise
 */
List *resources_list_duplicate(List *list);

/**
 * @brief get a resource's display name by its name id
 */
const char *resources_get_display_name(const char *name);

/**
 * @brief give count of a resource to a list
 * @param list the list to add to
 * @param name the name of the resource
 * @param count how much to give
 */
void resources_list_give(List *list,const char *name,float count);

/**
 * @brief take count of a resource from a list
 * @param list the list to take from
 * @param name the name of the resource
 * @param count how much to take
 */
void resources_list_withdraw(List *list,const char *name,float count);

/**
 * @brief deduct cost from supply of resources
 * @param supply the supply to take from
 * @param cost the cost of resources to take
 */
void resource_list_buy(List *supply, List *cost);

/**
 * @brief gain back the cost of something as it is being sold
 * @param supply the supply to give back to
 * @param cost how much to give back
 * @param rate the sellback rate.  
 */
void resource_list_sell(List *supply, List *cost,float rate);

/**
 * @brief query how much of a resource a list contains
 * @param list the list to check
 * @param name the name of the resource
 * @return how much
 */
float resources_list_get_amount(List *list,const char *name);

/**
 * @brief check if a resource is a commodity (measured in Physical Tons)
 * @param name the name of the resource in question
 * @return 1 if the resource is a commodity.  0 for any other reason
 */
int resource_is_commodity(const char *name);

/**
 * @brief check how much total mass the commodity takes up.
 * @note this only checks against resources with a T (tonnage) as the units
 * @param list the resource list to check
 * @return 0 if none, the amount otherwise
 */
int resources_get_total_commodity_mass(List *list);

/**
 * @brief get the resource struct
 * @param list the list to get it from
 * @param name the name to get
 * @return NULL if not found, the resource otherwise
 */
Resource *resources_list_get(List *list,const char *name);

/**
 * @brief get the default prices for each resource from the definition file
 * @return NULL if there is an error, or a list where each where each resource amount is the market value
 */
List *resources_get_default_prices();

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

/**
 * @brief check if the given supply has enough to afford a given cost
 * @param supply the wallet
 * @param cost the price
 * @return 0 is there is insufficient resources for the cost, 1 otherwise
 */
int resources_list_afford(List *supply, List *cost);

/**
 * @brief create an window element list of resources, optionally color code based on supply if provided
 * @param win the parent window
 * @param name the name id to get it by again later
 * @param offset offset position of the list.  relative to parent element
 * @param supply list of resources to display.  how much of a resource there is available.  
 * @param cost If provided, this will be shown as supply/cost and show in Green if supply>=cost or red otherwise
 * @param last if procided and no cost provided, this will color code the supply based on gains / losses relative to this last period
 * @note if a resource is more than the supply
 */
Element *resource_list_element_new(Window *win,const char *name, Vector2D offset,List *supply,List *cost,List *last);

#endif
