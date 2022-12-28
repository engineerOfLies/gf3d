#ifndef __MISSION_H__
#define __MISSION_H__

#include "gfc_callbacks.h"

typedef struct
{
    Callback callback;
    Uint32 dayStart;
    Uint32 dayFinished;
}Mission;

void mission_init();

Mission *mission_new(void *data, gfc_work_func callback, Uint32 dayStart, Uint32 dayFinished);

void mission_update_all();



#endif
