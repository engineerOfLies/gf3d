#ifndef __STATION_H__
#define __STATION_H__

#include "simple_json.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "entity.h"

typedef struct
{
    TextLine    name;  //its name identifier
    TextLine    facilityType;
    int         staffRequired;  //how many people are needed to run the facility
    int         staffAssigned;     //how many people are actually hired to do so
    int         inactive;//if the facility cannot run
    int         disabled;//if the player has chosen to turn it off
    TextLine    officer;//if there is an officer assigned to help run this facility
    List        *upkeep;    //what resources are needed to keep this running every update
    List        *produces;  //what this produces every update cycle
}StationFacility;

typedef struct StaionSection_S
{
    TextLine name;  //its name identifier
    Uint32 id;      //unique ID for the station section
    ModelMat mat;
    float hull,hullMax;
    float energyOutput,energyInput;
    float rotates;//if it rotates
    struct StaionSection_S *parent;// if not null, this is the parent
    Uint8 slot;                      // where the section is mounted on the parent
    List *children;
    List *facilities;
}StationSection;

typedef struct
{
    Uint32 idPool;      /**<keeps track of unique station IDs*/
    ModelMat *mat;
    int    sectionHighlight;
    float  sectionRotation;
    float  hull,hullMax;
    float  energyOutput,energyInput;
    List *sections;     /**<list of staiton sections*/
}StationData;

/**
 * @brief Create a new station entity
 * @param position where to spawn the aguman at
 * @param config if provided, load it from json object instead of hardcoded
 * @return NULL on error, or an agumon entity pointer on success
 */
Entity *station_new(Vector3D position,SJson *config);

/**
 * @brief convert station data into a saveable json object
 * @param data the station data to convert
 * @return NULL on error, or a valid json object to save
 */
SJson *station_save_data(StationData *data);

/**
 * @brief get a station section by it's unique ID
 * @param data the station to poll
 * @param id the searcch item
 * @return NULL if not not found, or a pointer to the station section
 */
StationSection *station_get_section_by_id(StationData *data,int id);

/**
 * @brief get a station section's child by its mounting slot
 * @param section the station section to query
 * @param slot the slot to check
 * @return NULL on error or no child at that slot
 */
StationSection *station_section_get_child_by_slot(StationSection *section,Uint8 slot);

#endif
