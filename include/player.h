#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"
#include "world.h"
#include "station.h"

typedef struct
{
    TextLine name;              /**<player's name*/
    TextLine filename;          /**<save game filename*/
    float   wages;              /**<how much you pay employees*/
    float   taxRate;            /**<how much locals provide for the system*/
    float   salesTaxRate;       /**<rate of income for commerce*/
    int     population;         /**<how many people live in on the station.  This is for taing purposes*/
    int     staff;              /**<people hired by the station to work for the station*/
    Uint32  hour;               /**<incremented periodically for timing*/
    Uint32  day;                /**<incremented every 24 hours to track the passage of game-time*/
    List   *resources;          /**<list of resources of the station*/
    Entity *station;            /**<the station of the player*/
    World  *world;              /**<the world (solar system really*/
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

/**
 * @brief get the player's space station data
 * @return NULL if no player, the station data otherwise
 */
StationData *player_get_station_data();

/**
 * @brief get the player's list of resoures
 * @return NULL if no player data, the resource list otherwise
 */
List * player_get_resources();

/**
 * @brief get the player's world
 * @return NULL if no player data, the world data otherwise
 */
World *player_get_world();

/**
 * @brief call to advance the player time by an hour
 */
void player_hour_advance();

/**
 * @brief get the player game day
 */
Uint32 player_get_day();

/**
 * @brief get the player game hour
 */
Uint32 player_get_hour();

/**
 * @brief save the player data to file
 * @param filename the file to save as
 */
void player_save(const char *filename);


#endif
