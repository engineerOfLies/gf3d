#include "simple_logger.h"

#include "combat.h"

void combat_save(CombatData *data);

CombatData *combat_load(SJson *data);

CombatData *combat_setup(CombatMode mode,CombatZone *zone,List *sideA,List *sideB)
{
    CombatData *combat;
    combat = combat_new();
    if (!combat)return NULL;
    combat->zone = zone;
    combat->mode = mode;
    combat->sideA = sideA;
    combat->sideB = sideB;
    
    return combat;
}

CombatData *combat_new()
{
    CombatData *combat;
    combat = gfc_allocate_array(sizeof(CombatData),1);
    if (!combat)return NULL;
    
    
    return combat;
}

void combat_free(CombatData *combat)
{
    if (!combat)return;
//     List       *sideA;  dunno who owns this yet
//     List       *sideB;
    free(combat);
}


/*eol@eof*/
