#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"


typedef struct
{
    TextLine    name;
    Uint32      amount;
    float       value;
}Resource;

typedef struct
{
    float   credits;            /**<cash on hand*/
    Uint32  population;         /**<people living on the station*/
    Uint32  staff;              /**<number of people employed by the station*/
    float   wages;              /**<how much you pay employees*/
    float   taxRate;            /**<how much locals provide for the system*/
    float   salesTaxRate;       /**<rate of income for commerce*/
    List   *resources;          /**<list of resources of the station*/
    Entity *station;            /**<the station of the player*/
}PlayerData;

/**
 * @brief Create a new player entity
 * @param file to load player data from
 * @return NULL on error, or an player entity pointer on success
 */
Entity *player_new(const char *file);

/**
 * @brief get the current player's game data
 * @return NULL if no player loaded, or the playerdata for the player
 */
PlayerData *player_get_data();

#endif
