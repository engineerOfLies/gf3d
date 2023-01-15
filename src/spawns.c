#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_list.h"
#include "gf3d_entity.h"

#include "gate.h"
#include "spawns.h"

typedef struct
{
    TextLine name;
    Entity *(*spawn)(SJson *config);
}Spawn;

static Spawn spawn_list[] = 
{
    {
        "tunnel_gate",
        gate_new
    },
    {
        "",
        NULL
    }
};

Entity *spawn_call(SJson *def)
{
    const char *str;
    int i;
    if (!def)return NULL;
    str = sj_object_get_value_as_string(def,"name");
    if (!str)return NULL;
    for (i = 0;spawn_list[i].spawn != NULL;i++)
    {
        if (gfc_strlcmp(str,spawn_list[i].name)==0)
        {
            return spawn_list[i].spawn(def);
        }
    }
    return NULL;
}

/*eol@eof*/
