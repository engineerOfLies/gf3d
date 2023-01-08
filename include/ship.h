#ifndef __SHIP_H__
#define __SHIP_H__

#include "simple_json.h"

#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_config.h"

#include "mission.h"

typedef struct
{
    TextLine    name;           //its name identifier
    TextLine    displayName;    //its name as displayed to the user, editable by the user
    Uint32      id;             //unique ID for the station section
    Uint32      idPool;         // for facilities on this ship
    int         housing;        //how much housing is provided by this S
    float       hull,hullMax;   //when hull <= 0 ship is destroyed
    Mission    *mission;        // if any mission is assigned to the section (mostly for repairs or building)
    Bool        working;  // if true, the ship is working
    float       energyOutput,energyDraw;//how much is produced, how much is needed, how much we have
    int         storageCapacity;
    int         staffAssigned,staffPositions; // how many staff have are working this section / how many positions there are to work
    List       *facilities;
}Ship;

/**
 * @brief allocate a blank ship
 */
Ship *ship_new();

/**
 * @brief free a ship from memory
 */
void ship_free(Ship *ship);

/**
 * @brief create a new ship based on def
 * @param name the ship def to make
 * @param id the id to set the ship to
 * @param defaults if true, also add the default facilities to the ship, else leave it empty
 * @return NULL on error or the ship
 */
Ship *ship_new_by_name(const char *name,int id,int defaults);

#endif
