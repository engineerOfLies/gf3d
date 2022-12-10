#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_config.h"

#include "gf3d_draw.h"

#include "config_def.h"
#include "station_def.h"
#include "resources.h"
#include "station.h"


void station_update(Entity *self);
void station_draw(Entity *self);
void station_think(Entity *self);
void station_free(Entity *self);

void station_section_free(StationSection *section);
StationSection *station_add_section(StationData *data,const char *sectionName,int id,StationSection *parent,Uint8 slot);

void station_facility_free(StationFacility *facility);
StationFacility *station_facility_new();
StationFacility *station_facility_load(SJson *config);
SJson *station_facility_save(StationFacility *facility);
void station_facility_free_list(List *list);
StationFacility *station_facility_new_by_name(const char *name);

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
    int i,c;
    SJson *json,*facilities;
    StationFacility *facility;
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
    c = gfc_list_get_count(section->facilities);
    if (c)
    {
        facilities = sj_array_new();
        for (i = 0; i < c; i++)
        {
            facility = gfc_list_get_nth(section->facilities,i);
            if (!facility)continue;
            sj_array_append(facilities,station_facility_save(facility));
        }
        sj_object_insert(json,"facilities",facilities);
    }
    return json;
}

SJson *station_save_data(StationData *data)
{
    int i,c;
    StationSection *section;
    SJson *station,*sections;
    if (!data)return NULL;
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
    return station;
}

StationData *station_load_data(SJson *station)
{
    const char *name;
    Uint32 id;
    int parentId;
    StationSection *section;
    StationSection *parent;
    StationFacility *stationFacility;
    int slot;
    StationData *data;
    int i,c;
    int j,d;
    SJson *list,*item;
    SJson *facilities,*facility;
    
    if (!station)
    {
        slog("no station object provided");
        return NULL;
    }
    data = gfc_allocate_array(sizeof(StationData),1);
    if (!data)
    {
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
            //here is where we parse out specific stats
            facilities = sj_object_get_value(item,"facilities");
            if (facilities)
            {
                station_facility_free_list(section->facilities);
                section->facilities = gfc_list_new();
                d = sj_array_get_count(facilities);
                for (j = 0;j < d; j++)
                {
                    facility = sj_array_get_nth(facilities,j);
                    if (!facility)continue;
                    stationFacility = station_facility_load(facility);
                    if (!stationFacility)continue;
                    section->facilities = gfc_list_append(section->facilities,stationFacility);
                }
            }
            sj_object_get_value_as_float(item,"hull",&section->hull);
        }
    }
    if (!sj_object_get_value_as_float(station,"hull",&data->hull))
    {
        data->hull = data->hullMax;
    }
    return data;
}

StationSection *station_section_get_child_by_slot(StationSection *section,Uint8 slot)
{
    int i,c;
    StationSection *child;
    if (!section)return NULL;
    c = gfc_list_get_count(section->children);
    for (i = 0; i < c;i++)
    {
        child = gfc_list_get_nth(section->children,i);
        if (!child)continue;
        if (child->slot == slot)return child;
    }
    return NULL;
}

StationSection *station_add_section(StationData *data,const char *sectionName,int id,StationSection *parent,Uint8 slot)
{
    Matrix4 mat;
    int i,c;
    float tempf;
    const char *str;
    StationFacility *facility;
    Vector3D offsetPosition = {0},offsetRotation = {0};
    StationSection *section;
    SJson *sectionDef,*parentDef,*extension,*stats,*list,*item;
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
    section->expansionSlots = sj_array_get_count(sj_object_get_value(sectionDef,"extensions"));
    
    list = sj_object_get_value(sectionDef,"default_facilities");
    if (list)
    {
        c = sj_array_get_count(list);
        for (i = 0; i < c; i++)
        {
            item = sj_array_get_nth(list,i);
            if (!item)continue;
            str = sj_get_string_value(item);
            if (!str)continue;
            facility = station_facility_new_by_name(str);
            if (!facility)continue;
            section->facilities = gfc_list_append(section->facilities,facility);
        }
    }
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



Entity *station_new(Vector3D position,SJson *config)
{
    Entity *ent = NULL;
    StationData *data = NULL;
    StationSection *section;
    
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
    
    if (config)
    {
        data = station_load_data(config);
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
    data->mat = &ent->mat;
    return ent;
}

void station_remove_section(StationData *station,StationSection *section)
{
    if ((!station)||(!section))return;
    if (section->parent)
    {
        gfc_list_delete_data(section->parent->children,section);
    }
    gfc_list_delete_data(station->sections,section);
    station_section_free(section);
}

void station_section_free(StationSection *section)
{
    if (!section)return;
    gf3d_model_free(section->mat.model);
    gfc_list_delete(section->children);
    gfc_list_foreach(section->facilities,(void (*)(void *))station_facility_free);
    gfc_list_delete(section->facilities);
    free(section);
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
            station_section_free(section);
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
    Vector4D color;
    Matrix4 mat,mat1;
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
            mat1,
            vector3d(0,0,0),
            rotation,
            vector3d(1,1,1));

        gfc_matrix4_from_vectors(
            mat,
            self->mat.position,
            self->mat.rotation,
            self->mat.scale);

        gfc_matrix_multiply(mat1,mat1,section->mat.mat);
        
        gfc_matrix_multiply(mat,mat1,mat);
        
        color = gfc_color_to_vector4f(self->color);

        if ((data->sectionHighlight != -1)&&(data->sectionHighlight != section->id))
        {
            color.w = 0.5;
        }
        else
        {
            color.w = 1.0;
        }
        gf3d_model_draw(section->mat.model,0,mat,color,vector4d(1,1,1,1));
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

void station_facility_free(StationFacility *facility)
{
    if (!facility)return;
    resources_list_free(facility->upkeep);
    resources_list_free(facility->produces);
    free(facility);
}

SJson *station_facility_save(StationFacility *facility)
{
    SJson *json;
    if (!facility)return NULL;
    json = sj_object_new();
    sj_object_insert(json,"name",sj_new_str(facility->name));
    sj_object_insert(json,"staff",sj_new_int(facility->staffAssigned));
    sj_object_insert(json,"inactive",sj_new_bool(facility->inactive));
    sj_object_insert(json,"disabled",sj_new_bool(facility->disabled));
    if (strlen(facility->officer))
    {
        sj_object_insert(json,"officer",sj_new_str(facility->officer));
    }
    return json;
}

StationFacility *station_facility_load(SJson *config)
{
    const char *str;
    StationFacility *facility;
    if (!config)
    {
        slog("no config provided");
        return NULL;
    }
    str = sj_object_get_value_as_string(config,"name");
    if (!str)
    {
        slog("facility has no name");
        return NULL;
    }
    facility = station_facility_new_by_name(str);
    if (!facility)
    {
        slog("failed to make facility %s",str);
        return NULL;
    }
    sj_object_get_value_as_int(config,"staff",&facility->staffAssigned);
    sj_object_get_value_as_bool(config,"inactive",(short int*)&facility->inactive);
    sj_object_get_value_as_bool(config,"disabled",(short int*)&facility->disabled);
    str = sj_object_get_value_as_string(config,"officer");
    if (str)gfc_line_cpy(facility->officer,str);
    return facility;
}

StationFacility *station_facility_new_by_name(const char *name)
{
    const char *str;
    StationFacility *facility;
    SJson *facilityDef,*res;
    if (!name)
    {
        slog("no name provided");
        return NULL;
    }
    facilityDef = config_def_get_by_name("facilities",name);
    if (!facilityDef)
    {
        slog("facility %s not found",name);
        return NULL;
    }
    facility = station_facility_new();
    if (!facility)
    {
        return NULL;
    }
    str = sj_object_get_value_as_string(facilityDef,"name");
    if (str)gfc_line_cpy(facility->name,str);
    str = sj_object_get_value_as_string(facilityDef,"type");
    if (str)gfc_line_cpy(facility->facilityType,str);
    res = sj_object_get_value(facilityDef,"produces");
    if (res)facility->produces = resources_list_parse(res);
    res = sj_object_get_value(facilityDef,"upkeep");
    if (res)facility->upkeep = resources_list_parse(res);
    sj_object_get_value_as_int(facilityDef,"staff",&facility->staffRequired);
    return facility;
}

void station_facility_free_list(List *list)
{
    if (!list)return;
    gfc_list_foreach(list,(void (*)(void *))station_facility_free);
    gfc_list_delete(list);
}

StationFacility *station_facility_new()
{
    StationFacility *facility;
    facility = gfc_allocate_array(sizeof(StationFacility),1);
    if (!facility)return NULL;
    facility->upkeep = resources_list_new();
    facility->produces = resources_list_new();
    return facility;
}
/*eol@eof*/
