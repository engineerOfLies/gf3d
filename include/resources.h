#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_list.h"

typedef struct
{
    TextLine    name;
    float       amount;
}Resource;

void resources_list_load();


List *resources_list_new();

void resources_list_free(List *list);
/**
 * @brief give one
 */
void resources_list_give(List *list,const char *name,float count);

float resources_list_get_amount(List *list,const char *name);

Resource *resources_list_get(List *list,const char *name);

SJson *resources_get_def(const char *name);

#endif
