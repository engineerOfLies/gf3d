#ifndef __MISSION_H__
#define __MISSION_H__

#include "simple_json.h"
#include "gfc_callbacks.h"

typedef struct
{
    TextLine title;         // to display to user
    Uint32  id;             /**<unique mission ID*/
    Uint32 dayStart;        
    Uint32 dayFinished;
    Uint32 staff;           //how many staff are assigned to this mission
    TextLine leader;        //who is leading
    TextLine missionType;   //"repair" "research" "dossier" "escort"
    TextLine missionSubject;//"(facility, section)", "kosh", "centauri",
    TextLine missionTarget; //"target of the misssion, habitat_ring, loading_dock, Armada, casino"
    Uint32 targetId;        // facility->id, section->id, armada->id, npc->id, etc
}Mission;

void mission_init();

Mission *mission_begin(
    const char *missionTitle,
    const char *leader,
    const char *missionType,
    const char *missionSubject,
    const char *missionTarget,
    Uint32 targetId,
    Uint32 dayStart,
    Uint32 timeToComplete,
    Uint32 staff);

void mission_update_all();

SJson *missions_save_to_config();

/**
 * @brief get the actual mission list
 */
List *mission_list_get();

void missions_load_from_config(SJson *config);

/**
 * @brief get a mission by its unique ID
 * @param id the seach criteria
 * @return NULL if not found, the mission otherwise
 */
Mission *mission_get_by_id(Uint32 id);

void mission_cancel(Mission *mission);

#endif
