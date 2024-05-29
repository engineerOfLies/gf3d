#include "simple_logger.h"

#include "gf3d_lights.h"

typedef struct
{
    GF3D_Light  ambientLight;
    GFC_List   *lights;
}LightManager;

static LightManager light_manager = {0};

void gf3d_lights_close()
{
    if (light_manager.lights)
    {
        gfc_list_foreach(light_manager.lights,(gfc_work_func*)free);
        gfc_list_delete(light_manager.lights);
    }
    memset(&light_manager,0,sizeof(LightManager));
}

void gf3d_lights_init()
{
    light_manager.lights = gfc_list_new();
    if (!light_manager.lights)
    {
        slog("failed to allocate lights");
    }
    atexit(gf3d_lights_close);
}

void gf3d_lights_clear_all()
{
    if (!light_manager.lights)return;
    gfc_list_foreach(light_manager.lights,(gfc_work_func*)free);
    memset(&light_manager.ambientLight,0,sizeof(GF3D_Light));
}

void gf3d_light_free(GF3D_Light *light)
{
    if (!light)return;
    gfc_list_delete_data(light_manager.lights,light);
    free(light);
}

GF3D_Light *gf3d_light_new()
{
    GF3D_Light *light;
    light = gfc_allocate_array(sizeof(GF3D_Light),1);
    if (!light)return NULL;
    gfc_list_append(light_manager.lights,light);
    return NULL;
}

void gf3d_light_set_ambient_light(GFC_Color color,GFC_Vector3D direction)
{
    memset(&light_manager.ambientLight,0,sizeof(GF3D_Light));
    light_manager.ambientLight.color = gfc_color_to_vector4f(color);
    gfc_vector3d_copy(light_manager.ambientLight.direction,direction);
}

GF3D_Light *gf3d_light_get_ambient_light()
{
    return &light_manager.ambientLight;
}

GF3D_Light *gf3d_light_new_spot(GFC_Color color, GFC_Vector3D position, GFC_Vector3D direction, float attenuation, float angle)
{
    GF3D_Light *light;
    light = gf3d_light_new();
    if (!light)return NULL;
    light->color = gfc_color_to_vector4f(color);
    light->direction = gfc_vector3dw(direction,1.0);
    light->position = gfc_vector3dw(position,1.0);
    light->attenuation = attenuation;
    light->angle = angle;
    return light;
}

GF3D_Light *gf3d_light_new_area(GFC_Color color, GFC_Vector3D position, float attenuation)
{
    GF3D_Light *light;
    light = gf3d_light_new();
    if (!light)return NULL;
    light->color = gfc_color_to_vector4f(color);
    gfc_vector3d_copy(light->position,position);
    light->position.w = 1.0;
    light->attenuation = attenuation;
    return light;
}


void gf3d_light_add_ambient_to_ubo(LightUBO *ubo,GF3D_Light *ambient)
{
    if ((!ubo)||(!ambient))return;
    memcpy(&ubo->ambient,ambient,sizeof(GF3D_Light));
}

void gf3d_light_add_global_ambient_to_ubo(LightUBO *ubo)
{
    gf3d_light_add_ambient_to_ubo(ubo,&light_manager.ambientLight);
}

void gf3d_light_build_ubo_from_list(LightUBO *ubo,GFC_List *lights)
{
    int i,c;
    GF3D_Light *light;
    if ((!ubo)||(!lights))return;
    memset(ubo,0,sizeof(LightUBO));
    c = gfc_list_get_count(lights);
    for (i = 0;i < MIN(c,MAX_SHADER_LIGHTS);i++)
    {
        light = gfc_list_get_nth(lights,i);
        if (!light)continue;
        memcpy(&ubo->lights[i],light,sizeof(GF3D_Light));
    }
    ubo->flags.y = i;
}

void gf3d_light_build_ubo_from_closest_list(LightUBO *ubo,GFC_List *lights, GFC_Vector3D relative)
{
    int i,j,c;
    float newDist;
    int least = -1;
    float bestDistances[MAX_SHADER_LIGHTS];
    GF3D_Light *bestLights[MAX_SHADER_LIGHTS] = {0};
    GF3D_Light *light;
    if (!ubo)return;
    c = gfc_list_get_count(lights);
    for (i = 0;i < c;i++)
    {
        light = gfc_list_get_nth(lights,i);
        if (!light)continue;
        if (i < MAX_SHADER_LIGHTS)
        {
            bestLights[i] = light;
            bestDistances[i] = gfc_vector3d_magnitude_between(gfc_vector4dxyz(light->position),relative);
            if (least == -1)least = i;
            else if (bestDistances[i] < bestDistances[least])least = i;
        }
        else //replace the least of the best
        {
            newDist = gfc_vector3d_magnitude_between(gfc_vector4dxyz(light->position),relative);
            if (newDist < bestDistances[least])
            {
                bestLights[least] = light;
                bestDistances[least] = newDist;
                least = 0;
                for (j = 1;j < MAX_SHADER_LIGHTS;j++)
                {
                    if (bestDistances[j] < bestDistances[least])least = j;
                }
            }
        }
    }
    for (i = 0;i < MIN(c,MAX_SHADER_LIGHTS);i++)
    {
        memcpy(&ubo->lights[i],bestLights[i],sizeof(GF3D_Light));
    }
    ubo->flags.y = i;
}

void gf3d_light_build_ubo_from_closest(LightUBO *ubo,GFC_Vector3D relative)
{
    gf3d_light_build_ubo_from_closest_list(ubo,light_manager.lights, relative);
}

LightUBO gf3d_light_basic_ambient_ubo()
{
    GF3D_Light *ambient;
    LightUBO lightUbo = {0};
    ambient = gf3d_light_get_ambient_light();
    if (!ambient)return lightUbo;
    memcpy(&lightUbo.ambient,ambient,sizeof(GF3D_Light));
    lightUbo.flags.x = 1;
    return lightUbo;
}

/*eol@eof*/
