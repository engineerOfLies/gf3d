#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "gf3d_entity.h"
#include "world.h"
#include "planet.h"
#include "station.h"
#include "ship.h"
#include "station_facility.h"

typedef struct
{
    float satisfaction; //overall view of the player's performance by the station population
    float basicNeeds;   //Are the basic needs of the people getting met
    float opportunities;//Are there enough jobs for people who seek them
    float commerce;     //can the people buy and sell things?
    float entertainment;//is there enough entertainment
    float safety;       //factor of crime and how often the station is directly attacked and damaged
}PlayerReputation;

typedef struct
{
    TextLine    name;               /**<player's name*/
    TextLine    filename;           /**<save game filename*/
    TextLine    assistantName;      /**<what the player calls the assistant droid*/
    Color       detailColor;        /**<station detail color*/
    float       wages;              /**<how much you pay employees*/
    float       taxRate;            /**<how much locals provide for the system*/
    float       salesTaxRate;       /**<rate of income for commerce*/
    int         population;         /**<how many people live in on the station.  This is for taing purposes*/
    int         staff;              /**<people hired by the station to work for the station*/
    PlayerReputation reputation;    /**<how the people who live in the station view the player*/
    Uint32      hour;               /**<incremented periodically for timing*/
    Uint32      day;                /**<incremented every 24 hours to track the passage of game-time*/
    List       *resources;          /**<list of resources of the station*/
    List       *yesterday;          /**<list of resources of the station from yesterday*/
    List       *lastMonth;          /**<list of resources of the station from last month*/
    List       *lastYear;           /**<list of resources of the station from last year*/
    List       *stockpile;          /**<list of amounts of a resource that will be kept on hand and not sold*/
    List       *salePrice;          /**<list of prices that the player will sell resources at*/
    List       *allowSale;          /**<list of commodities that will be auto-sold*/
    Entity     *station;            /**<the station of the player*/
    World      *world;              /**<the world (solar system really*/
    PlanetData *planet;             /**<the planet in the solar system the player controls*/
    SJson      *idPools;            /**<keep track of ids of station sections and facilties*/
    SJson      *history;            /**<keep track of events that have transpired*/
    List       *ships;              /**<all the player's ships*/
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
 * @brief get the current player's planet data
 * @return NULL if no player loaded, or the planetData for the player
 */
PlanetData *player_get_planet();

/**
 * @brief get the current player's history data
 * @return NULL if no player loaded, or no history for the player
 */
SJson *player_get_history();

/**
 * @brief get the current player's id pool data
 * @return NULL if no player loaded, or no id pool for the player
 */
SJson *player_get_id_pool();

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
 * @brief get the player's list of resoures that can be sold
 * @return NULL if no player data, the resource list otherwise
 */
List * player_get_allow_sale();

/**
 * @brief get the player's list of resource sale prices
 * @return NULL if no player data, the resource list otherwise
 */
List * player_get_sale_price();

/**
 * @brief get the player's list of resource reserve amounts (how much to hold onto before sale)
 * @return NULL if no player data, the resource list otherwise
 */
List * player_get_stockpile();

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

/**
 * @brief get a new ID for something named 'name'
 * @param name the name of a station section or facility
 * @return -1 on error or the id of a NEW item of the given name
 */
int player_get_new_id(const char *name);

/**
 * @brief after assigned staff are no longer needed, return them to the general staff of the station
 * @param staff how many staff to return;
 */
void player_return_staff(Uint32 staff);

/**
 * @brief search all of the facilities owned by the player for the first facility matching the name
 * @param name the name of the facility
 * @return NULL on error or not found, the facility in question otherwise
 */
StationFacility *player_get_facility_by_name(const char *name);

/**
 * @brief search all of the facilities owned by the player for a facility matching the name and id provided
 * @param name the name of the facility
 * @param id the id of the facility
 * @return NULL on error or not found, the facility in question otherwise
 */
StationFacility *player_get_facility_by_name_id(const char *name,Uint32 id);

/**
 * @brief given a facility, get its parent station section if it has one
 * @note many facility types do NOT belong to a station section (planet and ship) so expect NULL for those facilities
 * @param facilityTarget the facility to find a section for
 * @return NULL if not found among the station section facilities. or the section in question otherwise
 */
StationSection *player_get_section_by_facility(StationFacility *facilityTarget);

/**
 * @brief check if the player has a working dock or not
 * @return 0 is not, 1 if they do
 */
int player_has_working_dock();

/**
 * @brief get the count of the number of facilities that a player has between the planet and the station
 * @return the count or 0 if a problem
 */
int player_get_facility_count();

/**
 * @brief get the nth facility that the player owns (index should be less than the count provided above
 * @param index which facility to get (first the planet facilities, then each section facility.  In memory order
 * @return NULL if not found or the facility otherwise
 */
StationFacility *player_get_facility_nth(Uint32 index);

/**
 * @brief generate a new ship of the type specified for the player
 * @param name the name identifier for the ship
 */
void player_give_new_ship(const char *name);


#endif
