#include "simple_logger.h"

#include "gfc_list.h"

#include "gf3d_lights.h"

typedef struct
{
    GFC_Vector4D  globalGFC_Color;
    GFC_Vector3D  globalDir;
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

void gf3d_lights_get_global_light(GFC_Vector4D *color, GFC_Vector4D *direction)
{
    if (color)gfc_vector4d_copy((*color),light_manager.globalGFC_Color);
    if (direction)gfc_vector3d_copy((*direction),light_manager.globalDir);
}

void gf3d_lights_set_global_light(GFC_Vector4D color,GFC_Vector4D direction)
{
    gfc_vector4d_copy(light_manager.globalGFC_Color,color);
    gfc_vector3d_copy(light_manager.globalDir,direction);
    gfc_vector3d_normalize(&light_manager.globalDir);//sanity check
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

void gf3d_lights_insert(GFC_Vector3D position,MeshLights *dynamicLights,Uint32 count,Gf3D_Light *light)
{
    int i;
    float magnitude_to_new;
    if ((!dynamicLights)||(!light))return;
    if (count < MESH_LIGHTS_MAX)
    {
        gfc_vector4d_copy(dynamicLights[count].color,light->color);
        gfc_vector3d_copy(dynamicLights[count].position,light->position);
        return;
    }
    magnitude_to_new = gfc_vector3d_magnitude_squared(gfc_vector3d(light->position.x - position.x,light->position.y - position.y,light->position.z - position.z));

    for (i = 0; i < MESH_LIGHTS_MAX; i++)
    {
        if (gfc_vector3d_magnitude_squared(gfc_vector3d(position.x - dynamicLights[i].position.x,position.y - dynamicLights[i].position.y,position.z - dynamicLights[i].position.z)) > magnitude_to_new)
        {
            gfc_vector4d_copy(dynamicLights[i].color,light->color);
            gfc_vector3d_copy(dynamicLights[i].position,light->position);
            return;
        }
    }
}

void gf3d_lights_get_closest_dynamic_lights(GFC_Vector3D position,float radius, float * dynamicLightCount,MeshLights dynamicLights[MESH_LIGHTS_MAX])
{
    int i,count = 0;
    if (!dynamicLightCount)return;
    for (i = 0;i < light_manager.max_lights; i++)
    {
        if (!light_manager.lights[i]._inuse)continue;
        if (gfc_vector3d_magnitude_compare(gfc_vector3d(position.x - light_manager.lights[i].position.x,position.y - light_manager.lights[i].position.y,position.z - light_manager.lights[i].position.z),radius))
        {
            gf3d_lights_insert(position,dynamicLights,count,&light_manager.lights[i]);
            if (count < MESH_LIGHTS_MAX)count++;
        }
    }
    *dynamicLightCount = count;
}


Gf3D_Light *gf3d_light_make(GFC_Vector4D color,GFC_Vector3D position,GFC_Vector3D direction)
{
    Gf3D_Light *light;
    light = gf3d_light_new();
    if (!light)return NULL;
    gfc_vector3d_copy(light->color,color);
    gfc_vector3d_copy(light->position,position);
    return light;
}


/*eol@eof*/
