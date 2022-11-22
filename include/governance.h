#ifndef __GOVERNANCE_H__
#define __GOVERNANCE_H__

#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_types.h"

typedef struct
{
    TextLine    name;
    Uint32      amount;
    float       value;
}Resource;

typedef struct
{
    float   credits;            /**<cash on hand*/
    Uint32  population;         /**<people living on the station*/
    Uint32  staff;              /**<number of people employed by the station*/
    float   wages;              /**<how much you pay employees*/
    float   salesTaxRate;       /**<rate of income for commerce*/
    List   *resources;          /**<list of resources of the station*/
}Governance;

#endif
