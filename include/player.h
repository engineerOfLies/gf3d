#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"
#include "station.h"

typedef struct
{
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

StationData *player_get_station_data();

/**
 * @brief save the player data to file
 * @param filename the file to save as
 */
void player_save(const char *filename);


#endif
