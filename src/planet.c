#include "simple_logger.h"
#include "simple_json.h"

#include "gf2d_message_buffer.h"
#include "station_facility.h"
#include "player.h"
#include "planet.h"

SiteResourceTypes planet_site_resource_type_by_name(const char *name)
{
    if (!name)return SRT_MAX;
    if (strcmp(name,"nutrients")== 0)return SRT_Nutrients;
    if (strcmp(name,"minerals")== 0)return SRT_Minerals;
    if (strcmp(name,"ores")== 0)return SRT_Ores;
    return SRT_MAX;
}

void planet_site_survey(PlanetData *planet,Vector2D position)
{
    SiteData *site;
    if (!planet)return;
    site = planet_get_site_data_by_position(planet,position);
    if (!site)return;
    site->surveyed = 1;
    //TODO: check if there is anything special about this site and then reveal it
    
}

int planet_site_surveyed(PlanetData *planet,Vector2D position)
{
    SiteData *site;
    if (!planet)return 0;
    site = planet_get_site_data_by_position(planet,position);
    if (!site)return 0;
    return site->surveyed;
}


int planet_site_extract_resource(PlanetData *planet,Vector2D position,const char *resource)
{
    SiteResourceTypes rType = SRT_MAX;
    SiteData *site;
    if (!planet)return 0;
    site = planet_get_site_data_by_position(planet,position);
    if (!site)return 0;
    rType = planet_site_resource_type_by_name(resource);
    if (rType == SRT_MAX)return 0;
//    slog("site %i,%i has %i nutrients, %i minerals, %i ores",(int)position.x,(int)position.y,site->resources[SRT_Nutrients],site->resources[SRT_Minerals],site->resources[SRT_Ores]);
    if (site->resources[rType] <= 0)return 0;
    site->resources[rType]--;
    return 1;
}

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
    planet_draw_facilities(planet);
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
        planet_site_data_from_config(item,&planet->sites[i%MAX_LONGITUDE][i / MAX_LONGITUDE]);
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

Vector3D planet_site_get_relay(Vector2D site)
{
    Vector3D approach,delta;
    Vector3D position,dir;
    PlanetData *planet;
    planet = player_get_planet();
    if (!planet)return vector3d(-1,-1,-1);
    approach = planet_site_get_approach(site);
    vector3d_copy(position,planet->mat.position);
    if (vector3d_magnitude_squared(approach) < vector3d_magnitude_squared(planet->mat.position))
    {
        //if we are on the side of the planet facing the station, just set the position to a point near the station
        vector3d_copy(position,planet->mat.position);
        vector3d_set_magnitude(&position,500);
        return position;
    }
    //else we need to move parallel
    vector3d_copy(delta,approach);
    vector3d_normalize (&position);//planet 
    vector3d_normalize (&delta);//site
    vector3d_sub(dir,delta,position);
    vector3d_scale(dir,dir,planet->radius + 100);
    vector3d_add(position,approach,dir);
    return position;
}

Vector3D planet_site_get_approach(Vector2D site)
{
    PlanetData *planet;
    planet = player_get_planet();
    if (!planet)return vector3d(-1,-1,-1);
    return planet_position_to_position(planet->radius + 100, site);
}

Vector3D planet_position_to_position(float radius, Vector2D site)
{
    PlanetData *planet;
    Vector3D position = {0};
    position.y = radius;
    vector3d_rotate_about_x(&position, site.y * 10 * GFC_DEGTORAD);
    vector3d_rotate_about_z(&position, (site.x * 10 * GFC_DEGTORAD)+GFC_PI);
    
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
