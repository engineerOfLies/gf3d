#include "simple_logger.h"
#include "gfc_text.h"
#include "gfc_pak.h"
#include "gf3d_cliplayers.h"

static SJson *cliplayers = NULL;

void gf3d_cliplayers_close()
{
    sj_free(cliplayers);
    cliplayers = NULL;
}

void gf3d_cliplayers_init(const char *file)
{
    SJson *json,*layers;
    if (!file)return;
    json = gfc_pak_load_json(file);
    if (!json)return;
    layers = sj_object_get_value(json,"clipLayers");
    if (!layers)
    {
        slog("cannot configure clip layers. config file %s missing clipLayers array");
        return;
    }
    cliplayers = sj_copy(layers);
    sj_free(json);
    atexit(gf3d_cliplayers_close);
}

Uint32 gf3d_cliplayers_get_value(const char *layer)
{
    int i,c; 
    const char *str;
    SJson *item;
    if (!layer)return 0;
    c = sj_array_get_count(cliplayers);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(cliplayers,i);
        if (!item)continue;
        str = sj_get_string_value(item);
        if (!str)continue;
        if (gfc_strlcmp(layer,str)==0)
        return 1<<i;
    }
    return 0;
}

Uint32 gf3d_cliplayers_from_config(SJson *layers)
{
    int i,c;
    SJson *item;
    const char *str;
    Uint32 mask = 0;
    if (!layers)return 0;
    c = sj_array_get_count(layers);
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(layers,i);
        if (!item)continue;
        str = sj_get_string_value(item);
        mask |= gf3d_cliplayers_get_value(str);
    }
    return mask;
}

/*eol@eof*/
