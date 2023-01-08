#ifndef __SHIP_FACILITY_H__
#define __SHIP_FACILITY_H__

#include "simple_json.h"

#include "gfc_text.h"
#include "gfc_list.h"


typedef struct
{
    TextLine    name;           //its name identifier
    TextLine    displayName;    //its name as displayed to the user (including ID number)
    Uint32      id;
    TextLine    slot_type;  
    float       damage;         //keeps track of damage.  Damaged facilities lave lower output  0 is no damage, 100 is destroyed anything else can be repaired.
    int         housing;        //how much housing is provided by this facility
    int         storage;        // how many commodities can be stored here
    int         staffRequired;  //how many people are needed to run the facility at a minimum
    int         staffAssigned;  //how many people are actually hired to do so
    int         staffPositions; //how many people are CAN be hired to do so
    int         energyDraw;     //how much energy is needed for the facility to run
    int         energyOutput;   //how much energy is produced by the facility
    int         inactive;       //if the facility cannot run
}ShipFacility;

/**
 * @brief allocate and initialize a new ship facility
 * @return NULL on error or the station facility
 */
ShipFacility *ship_facility_new();

/**
 * @brief free a ship facility
 */
void ship_facility_free(ShipFacility *facility);

/**
 * @brief create an initialize a facility based on its name
 * @param name the facility to make it as
 * @param id the id of the new facility
 * @return NULL on error, or the setup ship facility
 */
ShipFacility *ship_facility_new_by_name(const char *name,int id);

/**
 * @brief get a list of facility names that match the given slot type
 * @param slot_type the type to filter for
 * @return NULL on error or a list of pointers to names of ship facilities.
 * @note delete the list when you are done, not he data pointed to by the list
 */
List *ship_facility_get_def_list_by_types(const char *slot_type);

/**
 * @brief save a ship facility to json
 */
SJson *ship_facility_save(ShipFacility *facility);

/**
 * @brief load a saved ship facility from json
 */
ShipFacility *ship_facility_load(SJson *json);

#endif
