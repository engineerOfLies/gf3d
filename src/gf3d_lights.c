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

void gf3d_lights_insert(Vector3D position,MeshLights *dynamicLights,Uint32 count,Gf3D_Light *light)
{
    int i;
    float magnitude_to_new;
    if ((!dynamicLights)||(!light))return;
    if (count < MESH_LIGHTS_MAX)
    {
        vector3d_copy(dynamicLights[count].color,light->color);
        vector3d_copy(dynamicLights[count].position,light->position);
        vector3d_copy(dynamicLights[count].direction,light->direction);
        return;
    }
    magnitude_to_new = vector3d_magnitude_squared(vector3d(light->position.x - position.x,light->position.y - position.y,light->position.z - position.z));

    for (i = 0; i < MESH_LIGHTS_MAX; i++)
    {
        if (vector3d_magnitude_squared(vector3d(position.x - dynamicLights[i].position.x,position.y - dynamicLights[i].position.y,position.z - dynamicLights[i].position.z)) > magnitude_to_new)
        {
            vector3d_copy(dynamicLights[i].color,light->color);
            vector3d_copy(dynamicLights[i].position,light->position);
            vector3d_copy(dynamicLights[i].direction,light->direction);
            return;
        }
    }
}

void gf3d_lights_get_closest_dynamic_lights(Vector3D position,float radius, float * dynamicLightCount,MeshLights dynamicLights[MESH_LIGHTS_MAX])
{
    int i,count = 0;
    if (!dynamicLightCount)return;
    for (i = 0;i < light_manager.max_lights; i++)
    {
        if (!light_manager.lights[i]._inuse)continue;
        if (vector3d_magnitude_compare(vector3d(position.x - light_manager.lights[i].position.x,position.y - light_manager.lights[i].position.y,position.z - light_manager.lights[i].position.z),radius))
        {
            gf3d_lights_insert(position,dynamicLights,count,&light_manager.lights[i]);
            if (count < MESH_LIGHTS_MAX)count++;
        }
    }
    *dynamicLightCount = count;
}


Gf3D_Light *gf3d_light_make(Vector4D color,Vector3D position,Vector3D direction)
{
    Gf3D_Light *light;
    light = gf3d_light_new();
    if (!light)return NULL;
    vector3d_copy(light->color,color);
    vector3d_copy(light->position,position);
    vector3d_copy(light->direction,direction);
    vector3d_normalize(&light->direction);//sanity check
    return light;
}


/*eol@eof*/
