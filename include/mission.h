#ifndef __MISSION_H__
#define __MISSION_H__

#include "simple_json.h"
#include "gfc_callbacks.h"

typedef struct
{
    TextLine title;         // to display to user
    Uint32  id;         /**<unique mission ID*/
    Uint32 dayStart;
    Uint32 dayFinished;
    Uint32 staff;
    TextLine missionType;   //"repair" "research" "dossier" "escort"
    TextLine missionSubject;//"(facility, section)", "weapons systems" "personOfInterest" "vessel"
    TextLine missionTarget; //"which one is repairing (id of it)" "researcher" "investigator" "fleet"
}Mission;

void mission_init();

Mission *mission_begin(
    const char *missionTitle,
    const char *missionType,
    const char *missionSubject,
    const char *missionTarget,
    Uint32 dayStart,
    Uint32 dayFinished,
    Uint32 staff);

void mission_update_all();

SJson *missions_save_to_config();

void missions_load_from_config(SJson *config);

/**
 * @brief get a mission by its unique ID
 * @param id the seach criteria
 * @return NULL if not found, the mission otherwise
 */
Mission *mission_get_by_id(Uint32 id);

void mission_cancel(Mission *mission);

#endif
