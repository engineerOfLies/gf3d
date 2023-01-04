#include "simple_logger.h"
#include "simple_json.h"

#include "station_facility.h"
#include "player.h"
#include "planet.h"

void planet_facilities_update(PlanetData *planet)
{
    int i,c;
    StationFacility *facility;
    if (!planet)return;
    c = gfc_list_get_count(planet->facilities);
    for (i = 0;i < c; i++)
    {
        facility = gfc_list_get_nth(planet->facilities,i);
        if (!facility)continue;
        station_facility_update(facility,NULL);
    }
}

void planet_draw_facilities(PlanetData *planet)
{
    int i,c;
    StationFacility *facility;
    if (!planet)return;
    c = gfc_list_get_count(planet->facilities);
    for (i = 0;i < c; i++)
    {
        facility = gfc_list_get_nth(planet->facilities,i);
        if (!facility)continue;
        station_facility_draw(facility);
    }
}

void planet_draw(PlanetData *planet)
{
    if (!planet)return;
    
    if (planet->mat.model)
    {
        vector3d_add(planet->mat.rotation,planet->mat.rotation,planet->mat.rotationDelta);
        gf3d_model_mat_set_matrix(&planet->mat);
        gf3d_model_draw(planet->mat.model,0,planet->mat.mat,vector4d(1,1,1,1),vector4d(1,1,1,1),vector4d(1,1,1,1));
    }
}

SiteData *planet_get_site_data_by_position(PlanetData *planet,Vector2D position)
{
    if (!planet)return NULL;
    if (((int)position.x < 0)||((int)position.x >= MAX_LONGITUDE))return NULL;
    if (((int)position.y < -8)||((int)position.y > 8 ))return NULL;
    return &planet->sites[(int)position.x][(int)position.y + 8];
}

void planet_free(PlanetData *planet)
{
    if (!planet)return;
    gfc_list_foreach(planet->facilities,(void (*)(void *))station_facility_free);
    gfc_list_delete(planet->facilities);
    free(planet);
}

PlanetData *planet_new()
{
    PlanetData *planet;
    planet = gfc_allocate_array(sizeof(PlanetData),1);
    if (!planet)return NULL;
    planet->facilities = gfc_list_new();
    return planet;
}

void planet_site_data_from_config(SJson *config,SiteData *site)
{
    const char *str;
    if ((!config)||(!site))return;
    str = sj_object_get_value_as_string(config,"special");
    if (str)gfc_line_cpy(site->special,str);
    sj_object_get_value_as_int(config,"surveyed",&site->surveyed);
    sj_object_get_value_as_int(config,"nutrients",&site->resources[SRT_Nutrients]);
    sj_object_get_value_as_int(config,"minerals",&site->resources[SRT_Minerals]);
    sj_object_get_value_as_int(config,"ores",&site->resources[SRT_Ores]);
}

SJson *planet_site_to_config(SiteData *site)
{
    SJson *config;
    if (!site)return NULL;
    config = sj_object_new();
    if (!config)return NULL;
    if (strlen(site->special) > 0)
    {
        sj_object_insert(config,"special",sj_new_str(site->special));
    }
    if (site->surveyed)sj_object_insert(config,"surveyed",sj_new_int(site->surveyed));
    sj_object_insert(config,"nutrients",sj_new_int(site->resources[SRT_Nutrients]));
    sj_object_insert(config,"minerals",sj_new_int(site->resources[SRT_Minerals]));
    sj_object_insert(config,"ores",sj_new_int(site->resources[SRT_Ores]));
    return config;
}

SJson *planet_save_to_config(PlanetData *planet)
{
    int i,j,c;
    StationFacility *facility;
    SJson *config,*array,*item;
    if (!planet)return NULL;
    config = sj_object_new();
    if (!config)return NULL;    
    c = gfc_list_get_count(planet->facilities);
    array = sj_array_new();
    for (i = 0;i < c;i++)
    {
        facility = gfc_list_get_nth(planet->facilities,i);
        if (!facility)continue;
        item = station_facility_save(facility);
        sj_array_append(array,item);
    }
    sj_object_insert(config,"facilities",array);
    
    array = sj_array_new();
    for (j = 0;j < MAX_LATITUDE; j++)
    {
        for (i = 0;i < MAX_LONGITUDE; i++)
        {
            item = planet_site_to_config(&planet->sites[i][j]);
            sj_array_append(array,item);
        }
    }
    
    sj_object_insert(config,"sites",array);
    sj_object_insert(config,"radius",sj_new_float(planet->radius));
    sj_object_insert(config,"modelMat",gf3d_model_mat_save(&planet->mat,0));
    return config;
}

PlanetData *planet_load_from_config(SJson *config)
{
    int i,c;
    SJson *array,*item;
    PlanetData *planet;
    if (!config)return NULL;
    planet = planet_new();
    if (!planet)return NULL;
    array = sj_object_get_value(config,"facilities");
    c = sj_array_get_count(array);
    for (i = 0;i < c; i++)
    {
        item = sj_array_get_nth(array,i);
        if (!item)continue;
        planet->facilities = gfc_list_append(planet->facilities,station_facility_load(item));
    }
    array = sj_object_get_value(config,"sites");
    c = sj_array_get_count(array);
    for (i = 0;i < c; i++)
    {
        item = sj_array_get_nth(array,i);
        if (!item)continue;
        planet_site_data_from_config(item,&planet->sites[c%MAX_LONGITUDE][c / MAX_LONGITUDE]);
    }
    sj_object_get_value_as_float(config,"radius",&planet->radius);
    item = sj_object_get_value(config,"modelMat");
    if (item)
    {
        gf3d_model_mat_parse(&planet->mat,item);
        gf3d_model_mat_set_matrix(&planet->mat);
    }
    return planet;
}

void planet_seed_area(PlanetData *planet,int lon,int lat,int amount,SiteResourceTypes which)
{
    int jdist,idist;
    int deposit;
    int i,j;
    if (!planet)return;
    for (j = 0; j < MAX_LATITUDE;j++)
    {
        jdist = abs(j - lon);
        for (i = 0; i < MAX_LONGITUDE;i++)
        {
            idist = MIN(abs(i - lat),abs((lat + MAX_LONGITUDE)- i));
            deposit = amount - (idist + jdist);
            if (deposit < 0)continue;
            planet->sites[i][j].resources[which] += deposit;
        }
    }
}

void planet_reset_resource_map(PlanetData *planet)
{
    if (!planet)return;
    memset(planet->sites,0,sizeof(SiteData)*MAX_LONGITUDE*MAX_LATITUDE);
}

void planet_generate_resource_map(PlanetData *planet)
{
    int count;
    int amount;
    int i,j,k;
    if (!planet)return;
    srand(100);
    count = 35;
    for (k = 0; k < count; k++)
    {
        amount = 14;
        i = gfc_random() * (float)MAX_LONGITUDE;
        j = gfc_random() * (float)MAX_LATITUDE;
        planet_seed_area(planet,i,j,amount,SRT_Nutrients);
    }
    count = 25;
    for (k = 0; k < count; k++)
    {
        amount = 18;
        i = gfc_random() * (float)MAX_LONGITUDE;
        j = gfc_random() * (float)MAX_LATITUDE;
        planet_seed_area(planet,i,j,amount,SRT_Minerals);
    }
    count = 20;
    for (k = 0; k < count; k++)
    {
        amount = 20;
        i = gfc_random() * (float)MAX_LONGITUDE;
        j = gfc_random() * (float)MAX_LATITUDE;
        planet_seed_area(planet,i,j,amount,SRT_Ores);
    }
}

Vector3D planet_position_to_position(float radius, Vector2D site)
{
    PlanetData *planet;
    Vector3D position = {0};
    position.y = radius;
    vector3d_rotate_about_x(&position, site.y * 10 * GFC_DEGTORAD);
    vector3d_rotate_about_z(&position, site.x * 10 * GFC_DEGTORAD);
    
    planet = player_get_planet();
    if (planet)
    {
        vector3d_rotate_about_z(&position, planet->mat.rotation.z);
    }
    return position;
}

Vector3D planet_position_to_rotation(Vector2D coordinates)
{
    Vector3D position = {0};
    Vector3D angles = {0};
    position = planet_position_to_position(100, coordinates);
    vector3d_angles (position, &angles);
    angles.x += GFC_HALF_PI;
    return angles;
}

/*eol@eof*/
