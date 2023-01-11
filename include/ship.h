#ifndef __SHIP_H__
#define __SHIP_H__

#include "simple_json.h"

#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_config.h"

#include "gf3d_entity.h"
#include "mission.h"

typedef struct
{
    TextLine    name;           //its name identifier
    TextLine    displayName;    //its name as displayed to the user, editable by the user
    Uint32      id;             //unique ID for the station section
    Uint32      idPool;         // for facilities on this ship
    TextLine    location;       // where is the ship
    Vector3D    position;       // more specifically
    int         housing;        //how much housing is provided by this S
    int         passengers;     //how many people are being transported
    float       hull,hullMax;   //when hull <= 0 ship is destroyed
    TextLine    captain;        //who is in command of the ship (usually who is in the helm)
    Mission    *mission;        // if any mission is assigned to the section (mostly for repairs or building)
    Bool        working;        // if true, the ship is working
    int         staffPositions; // how many positions there are to work
    int         staffRequired;  //how many people are needed to run the facility at a minimum
    int         staffAssigned;  //how many people are actually hired to this ship
    float       energyOutput,energyDraw;//how much is produced, how much is needed, how much we have
    int         storageCapacity;//ship total
    List       *cargo;          //a resources list of the cargo
    int         disabled;       // if the ship cannot run.  No crew, no power, no engines
    int         speed;          // top speed for the ship
    float       efficiency;     // factor for overal ship performance
    List       *facilities;     //list of ship facilities
    Entity     *entity;         // pointer to the ship entity
}Ship;

/**
 * @brief allocate a blank ship
 */
Ship *ship_new();

/**
 * @brief aggregate stats from facilities and check if everything is working
 * @param ship the ship to check;
 */
void ship_check(Ship *ship);

/**
 * @brief change assigned staff to the ship
 * @param ship the ship to assign
 * @param amount how many staff
 * @return > 0 if the ship couldn't handle any more, < 0 if ship didn't have enough to remove 0 otherwise
 * @note the size of the return value is amount it couldn't handle
 */
int ship_change_staff(Ship *ship,int amount);

/**
 * @brief free a ship from memory
 */
void ship_free(Ship *ship);

/**
 * @brief load a ship instance from json
 */
Ship *ship_load(SJson *json);

/**
 * @brief save a ship instance to json
 */
SJson *ship_save(Ship *ship);

/**
 * @brief create a new ship based on def
 * @param name the ship def to make
 * @param id the id to set the ship to
 * @param defaults if true, also add the default facilities to the ship, else leave it empty
 * @return NULL on error or the ship
 */
Ship *ship_new_by_name(const char *name,int id,int defaults);

/**
 * @brief set a ship's location and position
 */
void ship_set_location(Ship *ship,const char *location,Vector3D position);

/**
 * @brief get how many slots a ship has for facilities of the specified type
 * @param ship the ship to check
 * @param slot_type the facility slot to check for
 * @return how many
 */
int ship_get_slot_count_by_type(Ship *ship,const char *slot_type);

/**
 * @brief get how many facility types the ship can have
 * @param ship the ship to check
 * @return how many
 */
int ship_get_slot_name_count(Ship *ship);

/**
 * @brief get the name of the nth facility slot for the ship
 * @param ship the ship to check
 * @param index which one
 * @return NULL if not found or error.  Or the name of it otherwise
 */
const char *ship_get_slot_name_by_index(Ship *ship, Uint32 index);

/**
 * @brief get how many facilities are installed to a ship of a given slot type
 * @param ship the ship to check
 * @param slot_type the facility slot to check for
 * @return how many facilities are installed
 */
int ship_get_slot_usage_by_type(Ship *ship,const char *slot_type);

#endif
