#include "simple_logger.h"

#include "gfc_list.h"

#include "gf3d_lights.h"

typedef struct
{
    Vector4D  globalColor;
    Vector3D  globalDir;
    Gf3D_Light *lights;
    Uint32      max_lights;
}LightManager;

static LightManager light_manager = {0};

void gf3d_lights_close()
{
    free(light_manager.lights);
    memset(&light_manager,0,sizeof(LightManager));
}

void gf3d_lights_init(Uint32 max_lights)
{
    if (!max_lights)
    {
        slog("cannot allocate 0 lights");
        return;
    }
    light_manager.lights = gfc_allocate_array(sizeof(Gf3D_Light),max_lights);
    if (!light_manager.lights)
    {
        slog("failed to allocate %i lights",max_lights);
    }
    light_manager.max_lights = max_lights;
    atexit(gf3d_lights_close);
}

void gf3d_lights_set_global_light(Vector4D color,Vector3D direction)
{
    vector4d_copy(light_manager.globalColor,color);
    vector3d_copy(light_manager.globalDir,direction);
    vector3d_normalize(&light_manager.globalDir);//sanity check
}

void gf3d_light_free(Gf3D_Light *light)
{
    if (!light)return;
    memset(light,0,sizeof(Gf3D_Light));
}

Gf3D_Light *gf3d_light_new()
{
    int i;
    for ( i = 0; i < light_manager.max_lights; i++)
    {
        if (light_manager.lights[i]._inuse)continue;
        light_manager.lights[i]._inuse = 1;
        return &light_manager.lights[i];
    }
    return NULL;
}

Gf3D_Light *gf3d_light_make(Vector4D color,Vector3D position,Vector3D direction)
{
    Gf3D_Light *light;
    light = gf3d_light_new();
    if (!light)return NULL;
    vector4d_copy(light->color,color);
    vector3d_copy(light->position,position);
    vector3d_copy(light->direction,direction);
    vector3d_normalize(&light->direction);//sanity check
    return light;
}


/*eol@eof*/
