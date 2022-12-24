#include "simple_logger.h"
#include "simple_json.h"

#include "station_facility.h"
#include "planet.h"


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
    sj_object_insert(config,"special",sj_new_str(site->special));
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
    Vector3D position = {0};
    position.x = radius;
    vector3d_rotate_about_y(&position, site.y * -10 * GFC_DEGTORAD);
    vector3d_rotate_about_z(&position, site.x * 10 * GFC_DEGTORAD);
    return position;
}

Vector3D planet_position_to_rotation(Vector2D coordinates)
{
    Vector3D position = {0};
    Vector3D angles = {0};
    position = planet_position_to_position(100, coordinates);
    vector3d_angles (position, &angles);
    return angles;
}

/*eol@eof*/
