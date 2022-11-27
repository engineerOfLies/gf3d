#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "gf3d_draw.h"

#include "station_def.h"
#include "station.h"


// there is going to be a station section definition json to describe what sections will be
// there is going to be the instance of the station sections

typedef struct StaionSection_S
{
    TextLine name;  //its name identifier
    Uint32 id;      //unique ID for the station section
    ModelMat mat;
    float rotates;//if it rotates
    struct StaionSection_S *parent;// if not null, this is the parent
    List *children;
}StationSection;

typedef struct
{
    Uint32 idPool;      /**<keeps track of unique station IDs*/
    float  sectionRotation;
    List *sections;     /**<list of staiton sections*/
}StationData;

void station_update(Entity *self);
void station_draw(Entity *self);
void station_think(Entity *self);
void station_free(Entity *self);

StationSection *station_add_section(StationData *data,const char *sectionName,int id,StationSection *parent,Uint8 slot);

StationSection *station_get_section_by_id(StationData *data,int id)
{
    int i,c;
    StationSection *section;
    if (!data)return NULL;
    c = gfc_list_get_count(data->sections);
    for (i = 0; i < c; i++)
    {
        section = gfc_list_get_nth(data->sections,i);
        if (!section)continue;
        if (section->id == id)return section;
    }
    return NULL;
}

StationData *station_load_data(const char *filename)
{
    const char *name;
    int id;
    int parentId;
    StationSection *parent;
    int slot;
    StationData *data;
    int i,c;
    SJson *json,*station,*list,*item;
    json = sj_load(filename);
    if (!json)return NULL;
    station = sj_object_get_value(json,"station");
    if (!station)
    {
        slog("no station object in %f",filename);
        sj_free(json);
        return NULL;
    }
    data = gfc_allocate_array(sizeof(StationData),1);
    if (!data)
    {
        sj_free(json);
        return NULL;
    }
    data->sections = gfc_list_new();
    sj_get_integer_value(sj_object_get_value(station,"idPool"),(int *)&data->idPool);
    list = sj_object_get_value(station,"sections");
    c = sj_array_get_count(list);
    for (i = 0; i < c;i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        name = sj_object_get_value_as_string(item,"name");
        sj_get_integer_value(sj_object_get_value(item,"id"),&id);
        slog("section id: %i",id);
        if (sj_object_get_value(item,"parent"))
        {
            sj_get_integer_value(sj_object_get_value(item,"parent"),&parentId);
            slog("parentId: %i",parentId);
            parent = station_get_section_by_id(data,parentId);
            if (!parent)
            {
                slog("failed to find parent section with id %i",parentId);
            }
            sj_get_integer_value(sj_object_get_value(item,"slot"),&slot);
        }
        else parent = NULL;
        station_add_section(data,name,id,parent,slot);
    }
    sj_free(json);
    return data;
}

StationSection *station_add_section(StationData *data,const char *sectionName,int id,StationSection *parent,Uint8 slot)
{
    const char *str;
    Vector3D offsetPosition = {0},offsetRotation = {0};
    StationSection *section;
    SJson *sectionDef,*parentDef,*extension;
    if (!sectionName)return NULL;
    sectionDef = station_def_get_by_name(sectionName);
    if (!sectionDef)
    {
        slog("could not load section named %s",sectionName);
        return NULL;
    }
    section = gfc_allocate_array(sizeof(StationSection),1);
    if (!section)return NULL;
    
    section->children = gfc_list_new();
    data->sections = gfc_list_append(data->sections,section);// add us to the station sections list
    if (parent)
    {
        //get offset from parent
        parentDef = station_def_get_by_name(parent->name);
        if (parentDef)
        {
            extension = station_def_get_extension_by_index(parentDef,slot);
            if (extension)
            {
                sj_value_as_vector3d(sj_object_get_value(extension,"offset"),&offsetPosition);
                sj_value_as_vector3d(sj_object_get_value(extension,"rotation"),&offsetRotation);
                vector3d_scale(offsetRotation,offsetRotation,GFC_DEGTORAD);

            }
            else
            {
                slog("no extension found for slot %i",slot);
            }
        }
        //set myself up as a child of the parent
        section->parent = parent;
        parent->children = gfc_list_append(parent->children,section);
        
        vector3d_scale_by(offsetPosition,offsetPosition,parent->mat.scale);
    
        vector3d_rotate_about_z(&offsetPosition,parent->mat.rotation.z);
        vector3d_rotate_about_y(&offsetPosition,parent->mat.rotation.y);
        vector3d_rotate_about_x(&offsetPosition,parent->mat.rotation.x);

        vector3d_add(offsetPosition,offsetPosition,parent->mat.position);
        vector3d_add(offsetRotation,offsetRotation,parent->mat.rotation);
    }
    gf3d_model_mat_reset(&section->mat);
    gf3d_model_mat_parse(&section->mat,sectionDef);
    
    vector3d_scale_by(section->mat.position,section->mat.position,section->mat.scale);
    vector3d_add(section->mat.position,offsetPosition,section->mat.position);
    
    vector3d_add(section->mat.rotation,offsetRotation,section->mat.rotation);
    slog("section rotation: %f,%f,%f",section->mat.rotation.x,section->mat.rotation.y,section->mat.rotation.z);
    sj_get_float_value(sj_object_get_value(sectionDef,"rotates"),&section->rotates);
    str = sj_object_get_value_as_string(sectionDef,"name");
    if (id >= 0)
    {
        section->id = id;
    }
    else
    {
        section->id = data->idPool++;
    }
    if (str)gfc_line_cpy(section->name,str);
    return section;
}



Entity *station_new(Vector3D position,const char *stationFile)
{
    Entity *ent = NULL;
    StationData *data = NULL;
    StationSection *section;
    
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
    
    if (stationFile)
    {
        data = station_load_data(stationFile);
    }
    
    if (!data)
    {
        data = gfc_allocate_array(sizeof(StationData),1);
        data->sections = gfc_list_new();
        
        section = station_add_section(data,"station_core",-1,NULL,0);
        station_add_section(data,"solar_collector",-1,section,2);
        station_add_section(data,"station_reactor",-1,section,1);
        section = station_add_section(data,"station_habitat",-1,section,0);
        section = station_add_section(data,"station_dock",-1,section,0);
    }
    
    ent->mat.scale.x = ent->mat.scale.y = ent->mat.scale.z = 100;
    ent->data = data;
    ent->selectedColor = gfc_color(0.9,0.7,0.1,1);
    ent->color = gfc_color(1,1,1,1);
    ent->think = station_think;
    ent->draw = station_draw;
    ent->update = station_update;
    ent->free = station_free;
    vector3d_copy(ent->mat.position,position);
    return ent;
}

void station_free(Entity *self)
{
    int i,c;
    StationSection *section;
    StationData *data;
    if (!self)return;
    data = (StationData *)self->data;
    if (data)
    {
        c = gfc_list_get_count(data->sections);
        for (i = 0; i < c; i++)
        {
            section = gfc_list_get_nth(data->sections,i);
            if (!section)continue;
            gfc_list_delete(section->children);
            gf3d_model_free(section->mat.model);
            free(section);
        }
        free(data);
    }
}

void station_update(Entity *self)
{
    StationData *data;
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
    if (!self->data)return;
    data = self->data;
    data->sectionRotation += 0.0005;
}

void station_draw(Entity *self)
{
    int i,c;
    Matrix4 mat;
    StationSection *section;
    StationData *data;
    Vector3D position,rotation,scale;
    if ((!self)||(!self->data))return;
    data = (StationData *)self->data;
    
    c = gfc_list_get_count(data->sections);
    for ( i = 0; i< c; i++)
    {
        section = gfc_list_get_nth(data->sections,i);
        if (!section)continue;
        
        vector3d_scale_by(position,section->mat.position,self->mat.scale);
        vector3d_add(position,self->mat.position,position);
        
        vector3d_add(rotation,self->mat.rotation,section->mat.rotation);
        rotation.x += (data->sectionRotation * section->rotates);
        scale = vector3d_multiply(self->mat.scale,section->mat.scale);
        
        gfc_matrix4_from_vectors(
            mat,
            position,
            rotation,
            scale);
        gf3d_model_draw(section->mat.model,0,mat,gfc_color_to_vector4f(self->color),vector4d(1,1,1,1));
        gf3d_model_draw_highlight(section->mat.model,0,mat,vector4d(0,1,0,1));
        
    }
}

void station_think(Entity *self)
{
    if (!self)return;
    // do maintenance
}

/*eol@eof*/
