#include "simple_logger.h"

#include "gfc_config.h"

#include "gf3d_draw.h"
#include "gf3d_particle.h"
#include "gf3d_lights.h"

static const char *gf3d_light_types[] = 
{
    "Directional",
    "Area",
    "Spot"
};

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
    return light;
}

void gf3d_light_set_ambient_light(GFC_Color color,GFC_Vector3D direction,float strength)
{
    memset(&light_manager.ambientLight,0,sizeof(GF3D_Light));
    light_manager.ambientLight.color = gfc_color_to_vector4f(color);
    gfc_vector3d_copy(light_manager.ambientLight.direction,direction);
    light_manager.ambientLight.direction.w = 1.0;
    light_manager.ambientLight.ambientCoefficient = strength;
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


void gf3d_light_add_light_to_ubo(LightUBO *ubo,GF3D_Light *light)
{
    if ((!ubo)||(!light))return;
    if (ubo->flags.x >= MAX_SHADER_LIGHTS)return;//can't fit any more lights
    memcpy(&ubo->lights[(Uint32)ubo->flags.x],light,sizeof(GF3D_Light));
    ubo->flags.x++;
}

void gf3d_list_reset_ubo(LightUBO *lights)
{
    if (!lights)return;
    memset(lights,0,sizeof(LightUBO));
}

LightUBO gf3d_light_get_global_lights_ubo()
{
    LightUBO ubo = {0};
//    gf3d_light_add_global_ambient_to_ubo(&ubo);
    gf3d_light_build_ubo_from_list(&ubo,light_manager.lights);
    return ubo;
}

void gf3d_light_add_global_ambient_to_ubo(LightUBO *ubo)
{
    gf3d_light_add_light_to_ubo(ubo,&light_manager.ambientLight);
}

void gf3d_light_build_ubo_from_list(LightUBO *ubo,GFC_List *lights)
{
    int i,c;
    GF3D_Light *light;
    if ((!ubo)||(!lights))return;
    if (ubo->flags.x >= MAX_SHADER_LIGHTS)return;
    c = gfc_list_get_count(lights);
    for (i = 0;i < c;i++)
    {
        light = gfc_list_get_nth(lights,i);
        if (!light)continue;
        gf3d_light_add_light_to_ubo(ubo,light);
        if (ubo->flags.x >= MAX_SHADER_LIGHTS)break;
    }
}

void gf3d_light_build_ubo_from_array(LightUBO *ubo, GF3D_Light *lights[],Uint32 count)
{
    int i;
    if ((!ubo)||(!lights)||(!count))return;
    if (ubo->flags.x >= MAX_SHADER_LIGHTS)return;//all full
    for (i =0; i < count;i++)
    {
        gf3d_light_add_light_to_ubo(ubo,lights[i]);
        if (ubo->flags.x >= MAX_SHADER_LIGHTS)break;
    }
}

GFC_List *gf3d_list_list_filter_from_pov(GFC_List *lights,GFC_Vector3D point, GFC_Vector3D view)
{
    GFC_List *list;//output
    GF3D_Light *light;
    GFC_Vector3D dir;
    int i,c;
    if (!lights)return NULL;
    c = gfc_list_count(lights);
    if (!c)return NULL;
    list = gfc_list_new();
    if (!list)return NULL;
    for (i = 0; i < c; i++)
    {
        light = gfc_list_nth(lights,i);
        if (!light)continue;
        if (!light->position.w)
        {
            //light is global, add it
            gfc_list_append(list,light);
            continue;
        }
        gfc_vector3d_sub(dir,light->position,point);
        if (gfc_vector3d_dot_product(dir,view) > 0)
        {
            gfc_list_append(list,light);//its in front of the point of view, add it
        }
    }
    return list;
}

void gf3d_light_build_ubo_from_closest_list_limit(LightUBO *ubo,GFC_List *lights, GFC_Vector3D relative,int limit)
{
    int i,j,c;
    float newDist;
    int most = -1;
    float bestDistances[MAX_SHADER_LIGHTS];     //best is smallest
    GF3D_Light *bestLights[MAX_SHADER_LIGHTS] = {0};
    GF3D_Light *light;
    if (!ubo)return;
    if ((limit > MAX_SHADER_LIGHTS)||(limit < 0))limit = MAX_SHADER_LIGHTS;
    c = gfc_list_get_count(lights);
    for (i = 0;i < MIN(c,limit);i++)
    {
        light = gfc_list_get_nth(lights,i);
        if (!light)continue;
        bestLights[i] = light;
        bestDistances[i] = gfc_vector3d_magnitude_between_squared(gfc_vector4dxyz(light->position),relative);
        if (most == -1)most = i;
        else if (bestDistances[i] > bestDistances[most])most = i;//figure out the worst of the first limit
    }
    for (;i < c;i++)
    {
        light = gfc_list_get_nth(lights,i);
        if (!light)continue;
        newDist = gfc_vector3d_magnitude_between_squared(gfc_vector4dxyz(light->position),relative);
        if (newDist < bestDistances[most])//if this is better than the worst, swap me in
        {
            bestLights[most] = light;
            bestDistances[most] = newDist;
            for (j = 0;j < limit;j++)//find a new worst
            {
                if (bestDistances[j] > bestDistances[most])most = j;
            }
        }
    }
    gf3d_light_build_ubo_from_array(ubo, bestLights,MIN(c,limit));
}

void gf3d_light_build_ubo_from_closest_list(LightUBO *ubo,GFC_List *lights, GFC_Vector3D relative)
{
    gf3d_light_build_ubo_from_closest_list_limit(ubo,lights,relative,-1);
}

void gf3d_light_build_ubo_from_closest(LightUBO *ubo,GFC_Vector3D relative)
{
    gf3d_light_build_ubo_from_closest_list(ubo,light_manager.lights, relative);
}

LightUBO gf3d_light_basic_ambient_ubo()
{
    LightUBO lightUbo = {0};
    gf3d_light_add_global_ambient_to_ubo(&lightUbo);
    return lightUbo;
}

void gf3d_light_draw(GF3D_Light *light)
{
    GFC_Vector3D dir;
    float size;
    if (!light)return;
    size = 0.1 * light->attenuation;
    if (light->position.w)
    {
        gf3d_particle_draw(
            gf3d_particle(
                gfc_vector4dxyz(light->position),
                gfc_color_from_vector4f(light->color),
                size));
    }
    if (light->direction.w)
    {
        gfc_vector3d_copy(dir,light->direction);
        gfc_vector3d_set_magnitude(&dir,size * 5);
        gfc_vector3d_add(dir,dir,light->position);
        gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector4dxyz(light->position),dir),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.01 * size,gfc_color_from_vector4f(light->color));
    }
}

void gf3d_light_list_free(GFC_List *list)
{
    if (!list)return;
    gfc_list_foreach(list,(gfc_work_func*)gf3d_light_free);
    gfc_list_delete(list);
}

GFC_List *gf3d_light_list_load_from_config(SJson *config)
{
    GFC_List *list;
    GF3D_Light *light;
    int i,c;
    SJson *item;
    if (!config)return NULL;
    c = sj_array_get_count(config);
    if (!c)return NULL;
    list = gfc_list_new();
    for (i = 0; i < c; i++)
    {
        item = sj_array_get_nth(config,i);
        if (!item)continue;
        light = gf3d_light_load_from_config(item);
        if (!light)continue;
        gfc_list_append(list,light);
    }
    return list;
}

SJson *gf3d_light_list_save_to_config(GFC_List *lights)
{
    int i,c;
    GF3D_Light *light;
    SJson *json;
    if (!lights)return NULL;
    c = gfc_list_count(lights);
    if (!c)return NULL;
    json = sj_array_new();
    if (!json)return NULL;
    for (i = 0;i < c;i++)
    {
        light = gfc_list_nth(lights,i);
        if (!light)continue;
        sj_array_append(json,gf3d_light_save_to_config(light));
    }
    return json;
}

SJson *gf3d_light_save_to_config(GF3D_Light *light)
{
    GFC_Vector4D color;
    SJson *json;
    if (!light)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
    gfc_vector4d_scale(color,light->color,255);//always save in range 0-255
    sj_object_insert(json,"color",sj_vector4d_new(color));    
    sj_object_insert(json,"direction",sj_vector4d_new(light->direction));    
    sj_object_insert(json,"position",sj_vector4d_new(light->position));    
    sj_object_insert(json,"ambientCoefficient",sj_new_float(light->ambientCoefficient));
    sj_object_insert(json,"attenuation",sj_new_float(light->attenuation));
    sj_object_insert(json,"angle",sj_new_float(light->angle));
    sj_object_insert(json,"brightness",sj_new_float(light->brightness));
    return json;
}

GF3D_Light *gf3d_light_load_from_config(SJson *config)
{
    GFC_Color color;
    GF3D_Light *light;
    if (!config)return NULL;
    light = gf3d_light_new();
    if (!light)return NULL;
    color = sj_object_get_color(config,"color");
    light->color = gfc_color_to_vector4f(color);
    
    sj_object_get_vector4d(config,"direction",&light->direction);
    sj_object_get_vector4d(config,"position",&light->position);

    sj_object_get_value_as_float(config,"ambientCoefficient",&light->ambientCoefficient);
    sj_object_get_value_as_float(config,"attenuation",&light->attenuation);
    sj_object_get_value_as_float(config,"angle",&light->angle);
    sj_object_get_value_as_float(config,"brightness",&light->brightness);
    return light;
}

GF3D_LightTypes gf3d_light_get_type_by_name(const char *name)
{
    int i;
    if (!name)return LT_MAX;
    for (i = 0; i < LT_MAX;i++)
    {
        if (gfc_strlcmp(name,gf3d_light_types[i])==0)return (GF3D_LightTypes)i;
    }
    return LT_MAX;
}

const char *gf3d_light_get_type_name(GF3D_LightTypes lightType)
{
    if (lightType >= LT_MAX)return NULL;
    return gf3d_light_types[lightType];
}

/*eol@eof*/
