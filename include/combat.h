#ifndef __COMBAT_H__
#define __COMBAT_H__

#include "simple_json.h"
#include "gfc_list.h"
#include "world.h"

typedef enum
{
    CM_Auto = 0,    //player not observing
    CM_Player = 1   //player observing and controlling
}CombatMode;

typedef struct
{
    CombatMode  mode;
    CombatZone *zone;//which zone to use
    List       *sideA;
    List       *sideB;
}CombatData;


void combat_save(CombatData *data);

CombatData *combat_load(SJson *data);

CombatData *combat_setup();

CombatData *combat_new();

void combat_free(CombatData *combat);

#endif
