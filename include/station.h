#ifndef __STATION_H__
#define __STATION_H__

#include "simple_json.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "gf3d_entity.h"
#include "mission.h"

typedef struct StationSection_S
{
    struct StationSection_S *parent;// if not null, this is the parent
    TextLine    name;  //its name identifier
    TextLine    displayName;  //its name as displayed to the user (including ID number)
    Uint32      id;      //unique ID for the station section
    ModelMat    mat;
    int         housing;        //how much housing is provided by this S

    float       hull,hullMax;
    float       crimeRate;  //how much crime goes on in the seciton as an aggregate of the facilities
    float       opportunities;  //how many business and employment opportunities are provided by the facility
    float       commerce;       //how much taxable commerce is conducted by the facility
    float       entertainment;  //how much public entertainment is provided by the facility
    Mission    *mission;    // if any mission is assigned to the section (mostly for repairs or building)
    Bool        working;  // if true, the station is working
    float       energyOutput,energyDraw;//how much is produced, how much is needed, how much we have
    float       rotates;//if it rotates
    int         storageCapacity;
    int         staffAssigned,staffPositions; // how many staff have are working this section / how many positions there are to work
    Vector3D    dockPosition;       //docking position before ship stops rendering / begins exit
    Vector3D    approach;           //relay point before making final docking approach
    Bool        drawGuideStrip;     //if we draw a guidesStrip for the section (only docks)
    Vector3D    guideStrip;         //position where the guide strip starts at
    Uint8       slot;                      // where the section is mounted on the parent
    Uint8       expansionSlots;            // how many sections can link off of this
    List       *children;
    Uint8       facilitySlots;             // how many facilities can be installed in this section
    List       *facilities;
}StationSection;

typedef struct
{
    ModelMat   *mat;
    int         housing;        //how much housing is provided by this station
    float       crimeRate;     //how much crime goes on in the seciton as an aggregate of the station
    float       opportunities;  //how many business and employment opportunities are provided by the facility
    float       commerce;       //how much taxable commerce is conducted by the facility
    float       entertainment;  //how much public entertainment is provided by the facility
    int         staffAssigned;  //how many staff are assigned to facilities
    int         staffPositions; //how many staff positions are available
    int         sectionHighlight;
    float       sectionRotation;
    float       hull,hullMax;
    float       energyOutput,energyDraw,energySupply;//how much is produced, how much is needed, how much we have
    int         storageCapacity,storageUsage;
    List       *sections;     /**<list of staiton sections*/
}StationData;

/**
 * @brief Create a new station entity
 * @param position where to spawn the aguman at
 * @param config if provided, load it from json object instead of hardcoded
 * @return NULL on error, or an agumon entity pointer on success
 */
Entity *station_new(Vector3D position,SJson *config);

/**
 * @brief update the global station values based on its sections and facilities.
 * @note should be called every time there is an update
 * @param station the station to recalc for
 */
void station_recalc_values(StationData *station);

/**
 * @brief run the upkeep on the station, its sections, and facilities
 * @param station the station to upkeep
 */
void station_upkeep(StationData *station);

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
 * @brief get a station section by it's unique displayName
 * @param data the station to poll
 * @param displayName the searcch item
 * @return NULL if not not found, or a pointer to the station section
 */
StationSection *station_get_section_by_display_name(StationData *data,const char *displayName);

/**
 * @brief get the nth child of a section of the given slot type
 * @param parent the section to check
 * @param n the nth 
 * @param slot_type the filter
 */
StationSection *station_section_get_nth_child_by_slot_type(StationSection *parent,Uint8 n,const char *slot_type);

/**
 * @brief get a station section's child by its mounting slot
 * @param section the station section to query
 * @param slot the slot to check
 * @return NULL on error or no child at that slot
 */
StationSection *station_section_get_child_by_slot(StationSection *section,Uint8 slot);

/**
 * @brief add a new section to the station
 * @param data the station to add to
 * @param sectionName the sectionName to add
 * @param id the id of the section
 * @param parent the parent of this section
 * @param slot the extension slot of the parent to mount to
 */
StationSection *station_add_section(StationData *data,const char *sectionName,int id,StationSection *parent,Uint8 slot);

/**
 * @brief removes a section from the station
 * @note this will NOT remove any children of the section.  those should be removed first
 * @param station the station to remove from
 * @param section the section to remove
 */
void station_remove_section(StationData *station,StationSection *section);

/**
 * @brief given a station section, get a list of possible facilities that can be installed.
 * @param parent the station section to query
 * @returns NULL if there are not valid types, or a list of names
 * @note the list should be freed, not the strings
 */
List *station_facility_get_possible_list(StationSection *parent);

/**
 * @brief set a section to repaired
 * @param section the section to repair
 */
void station_section_repair(StationSection *section);

/**
 * @brief check if any of the facilities in a section are working on a mission
 * @param section the section to check
 * @return 0 if there are no active jobs, 1 if there are at least one
 */
int station_section_facility_working(StationSection *section);

/**
 * @brief get the station seciton by its facility displayName
 * @param facilityName the displayName of the facility to search for
 * @return NULL if not found, otherwise the station section
 */
StationSection *station_section_get_by_facility(const char *facilityName);

/**
 * @brief get the approach vector for the given station section in world space
 * @param seciton the station section in question
 * @return a zero vector if not specified or error.  The position in space you should head to first otherwise
 */
Vector3D station_section_get_approach_vector(StationSection *section);

/**
 * @brief get the position of the dock for the given station section in world space
 * @param seciton the station section in question
 * @return a zero vector if not specified or error.  The position in space you should head to finally otherwise
 */
Vector3D station_section_get_docking_position(StationSection *section);

#endif
