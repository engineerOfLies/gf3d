#ifndef __ARMADA_H__
#define __ARMADA_H__

#include "simple_json.h"

#include "gfc_list.h"
#include "gfc_text.h"

#include "ships.h"

typedef struct
{
    TextLine name;
    TextLine mission;
    TextLine capitolShip;//which ship is the one in command
    TextLine location;
    List *ships;
}Armada;

/**
 * @brief allocate a new armada
 * @return NULL on error or a blank armada
 */
Armada *armada_new();

/**
 * @brief free a previously allocated 
 * @param armada the armada to free
 */
void armada_free(Armada *armada);

/**
 * @brief add a ship to an armada
 * @param armada the armada to add the ship to
 * @param ship the ship to add
 */
void armada_add_ship(Armada *armada, Ship *ship);

/**
 * @brief specifiy which ship should be the lead ship
 * @param armada the armda to change
 * @param shipName the name of the ship to set to be the capitol ship.  If its not part of the armada, this will be a no-op
 */
void armada_set_capitol_ship(Armada *armada, const char *shipName);

/**
 * @brief rename the armada
 * @param armada the armada to rename
 * @param name the name to call the armada by
 */
void armada_name(Armada *armada, const char *name);



#endif
