#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "gf3d_draw.h"

#include "config_def.h"
#include "station_def.h"
#include "station.h"


// there is going to be a station section definition json to describe what sections will be
// there is going to be the instance of the station sections

typedef struct StaionSection_S
{
    TextLine name;  //its name identifier
    Uint32 id;      //unique ID for the station section
    ModelMat mat;
    float hull,hullMax;
    float energyOutput,energyInput;
    float rotates;//if it rotates
    struct StaionSection_S *parent;// if not null, this is the parent
    Uint8 slot;                      // where the section is mounted on the parent
    List *children;
    List *facilities;
}StationSection;

typedef struct
{
    Uint32 idPool;      /**<keeps track of unique station IDs*/
    int    sectionHighlight;
    float  sectionRotation;
    float  hull,hullMax;
    float  energyOutput,energyInput;
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

SJson *station_section_to_json(StationSection *section)
{
    SJson *json;
    if (!section)return NULL;
    json = sj_object_new();
    if (!json)return NULL;
    sj_object_insert(json,"name",sj_new_str(section->name));
    sj_object_insert(json,"id",sj_new_uint32(section->id));
    sj_object_insert(json,"hull",sj_new_uint32(section->hull));
    if (section->parent)
    {
        sj_object_insert(json,"parent",sj_new_uint32(section->parent->id));
        sj_object_insert(json,"slot",sj_new_uint8(section->slot));
    }
    return json;
}

void station_save_data(StationData *data,const char *filename)
{
    int i,c;
    StationSection *section;
    SJson *json,*station,*sections;
    if ((!data)||(!filename))return;
    json = sj_object_new();
    if (!json)return;
    station = sj_object_new();
    sj_object_insert(station,"idPool",sj_new_uint32(data->idPool));
    sj_object_insert(station,"hull",sj_new_float(data->hull));
    sections = sj_array_new();
    c = gfc_list_get_count(data->sections);
    for (i = 0;i < c; i++)
    {
        section = gfc_list_get_nth(data->sections,i);
        if (!section)continue;
        sj_array_append(sections,station_section_to_json(section));
    }
    sj_object_insert(station,"sections",sections);
    sj_object_insert(json,"station",station);
    sj_save(json,filename);
    sj_free(json);
}

StationData *station_load_data(const char *filename)
{
    const char *name;
    Uint32 id;
    int parentId;
    StationSection *section;
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
    sj_object_get_value_as_int(station,"idPool",(int *)&data->idPool);
    
    list = sj_object_get_value(station,"sections");
    c = sj_array_get_count(list);
    for (i = 0; i < c;i++)
    {
        item = sj_array_get_nth(list,i);
        if (!item)continue;
        name = sj_object_get_value_as_string(item,"name");
        sj_object_get_value_as_uint32(item,"id",&id);
        if (sj_object_get_value(item,"parent"))
        {
            sj_get_integer_value(sj_object_get_value(item,"parent"),&parentId);
            parent = station_get_section_by_id(data,parentId);
            if (!parent)
            {
                slog("failed to find parent section with id %i",parentId);
            }
            sj_get_integer_value(sj_object_get_value(item,"slot"),&slot);
        }
        else parent = NULL;
        section = station_add_section(data,name,id,parent,slot);
        if (section)
        {
            sj_object_get_value_as_float(item,"hull",&section->hull);
        }
    }
    if (!sj_object_get_value_as_float(station,"hull",&data->hull))
    {
        data->hull = data->hullMax;
    }
    sj_free(json);
    return data;
}

StationSection *station_add_section(StationData *data,const char *sectionName,int id,StationSection *parent,Uint8 slot)
{
    Matrix4 mat;
    float tempf;
    const char *str;
    Vector3D offsetPosition = {0},offsetRotation = {0};
    StationSection *section;
    SJson *sectionDef,*parentDef,*extension,*stats;
    if (!sectionName)return NULL;
    sectionDef = config_def_get_by_name("sections",sectionName);
    if (!sectionDef)
    {
        slog("could not load section named %s",sectionName);
        return NULL;
    }
    section = gfc_allocate_array(sizeof(StationSection),1);
    if (!section)return NULL;
    gfc_matrix_identity(mat);

    section->children = gfc_list_new();
    data->sections = gfc_list_append(data->sections,section);// add us to the station sections list
    if (parent)
    {
        section->slot = slot;
        //get offset from parent
        parentDef = config_def_get_by_name("sections",parent->name);
        if (parentDef)
        {
            extension = station_def_get_extension_by_index(parentDef,slot);
            if (extension)
            {
                //get the offset for the extension
                vector3d_set(offsetPosition,0,0,0);
                sj_value_as_vector3d(sj_object_get_value(extension,"offset"),&offsetPosition);
                vector3d_set(offsetRotation,0,0,0);
                sj_value_as_vector3d(sj_object_get_value(extension,"rotation"),&offsetRotation);
                vector3d_scale(offsetRotation,offsetRotation,GFC_DEGTORAD);
                gfc_matrix4_from_vectors(mat,offsetPosition,offsetRotation,vector3d(1,1,1));
            }
            else
            {
                slog("no extension found for slot %i",slot);
            }
        }
        //set myself up as a child of the parent
        section->parent = parent;
        parent->children = gfc_list_append(parent->children,section);
        
        gfc_matrix_multiply(mat,mat,parent->mat.mat);
        
    }
    gf3d_model_mat_reset(&section->mat);
    gf3d_model_mat_parse(&section->mat,sectionDef);
    gf3d_model_mat_set_matrix(&section->mat);
    gfc_matrix_multiply(section->mat.mat,section->mat.mat,mat);
    
    sj_object_get_value_as_float(sectionDef,"rotates",&section->rotates);
    
    stats = sj_object_get_value(sectionDef,"stats");
    if (stats)
    {
        if (sj_object_get_value_as_float(stats,"hull",&tempf))
        {
            section->hull = section->hullMax = tempf;
            data->hullMax += tempf;
        }
    }
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
    
    station_save_data(data,"saves/testsave.save");
    data->sectionHighlight = -1;
    ent->mat.scale.x = ent->mat.scale.y = ent->mat.scale.z = 100;
//    ent->mat.rotation.y = -GFC_HALF_PI;
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
    Vector3D rotation;
    if ((!self)||(!self->data))return;
    data = (StationData *)self->data;
    
    c = gfc_list_get_count(data->sections);
    for ( i = 0; i< c; i++)
    {
        section = gfc_list_get_nth(data->sections,i);
        if (!section)continue;
        
        vector3d_copy(rotation,self->mat.rotation);
        rotation.x += (data->sectionRotation * section->rotates);

        gfc_matrix4_from_vectors(
            mat,
            self->mat.position,
            rotation,
            self->mat.scale);
        
        gfc_matrix_multiply(mat,section->mat.mat,mat);

        gf3d_model_draw(section->mat.model,0,mat,gfc_color_to_vector4f(self->color),vector4d(1,1,1,1));
        if (data->sectionHighlight == section->id)
        {
            gf3d_model_draw_highlight(section->mat.model,0,mat,vector4d(0,1,0,1));
        }
        
    }
}

void station_think(Entity *self)
{
    if (!self)return;
    // do maintenance
}

/*eol@eof*/
