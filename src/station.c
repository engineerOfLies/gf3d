#include "simple_logger.h"

#include "gfc_list.h"

#include "gf3d_draw.h"

#include "station.h"

typedef struct
{
    Model *model;
    ModelMat mat;
}StationSection;

typedef struct
{
    List *sections;
}StationData;

void station_update(Entity *self);
void station_draw(Entity *self);
void station_think(Entity *self);
void station_free(Entity *self);

void station_add_section(StationData *data,Model *segment, Vector3D offset)
{
    StationSection *section;
    if (!data)return;
    section = gfc_allocate_array(sizeof(StationSection),1);
    section->model = segment;
    gf3d_model_mat_reset(&section->mat);
    gf3d_model_mat_set_position(&section->mat,offset);
    data->sections = gfc_list_append(data->sections,section);
}

Entity *station_new(Vector3D position)
{
    Entity *ent = NULL;
    StationData *data;
    
    ent = entity_new();
    if (!ent)
    {
        return NULL;
    }
    data = gfc_allocate_array(sizeof(StationData),1);
    ent->data = data;
    data->sections = gfc_list_new();
    station_add_section(data,gf3d_model_load("models/station/station_core.model"), vector3d(0,0,0));
    station_add_section(data,gf3d_model_load("models/station/station_habitat.model"), vector3d(-2,0,0));

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
            gf3d_model_free(section->model);
            free(section);
        }
        free(data);
    }
}

void station_update(Entity *self)
{
    if (!self)
    {
        slog("self pointer not provided");
        return;
    }
    vector3d_add(self->mat.position,self->mat.position,self->velocity);
    self->mat.rotation.y += 0.001;
}

void station_draw(Entity *self)
{
    int i,c;
    Matrix4 mat;
    StationSection *section;
    StationData *data;
    if ((!self)||(!self->data))return;
    data = (StationData *)self->data;
    
    c = gfc_list_get_count(data->sections);
    for ( i = 0; i< c; i++)
    {
        section = gfc_list_get_nth(data->sections,i);
        if (!section)continue;
        
        
        gfc_matrix4_from_vectors(
            mat,
            vector3d(self->mat.position.x + section->mat.position.x,
                     self->mat.position.y + section->mat.position.y,
                     self->mat.position.z + section->mat.position.z),
            self->mat.rotation,
            self->mat.scale);
        gf3d_model_draw(section->model,mat,gfc_color_to_vector4f(self->color),vector4d(1,1,1,1));
        
    }
}

void station_think(Entity *self)
{
    if (!self)return;
    switch(self->state)
    {
        case ES_idle:
            //look for player
            break;
        case ES_hunt:
            // set move towards player
            break;
        case ES_dead:
            // remove myself from the system
            break;
        case ES_attack:
            // run through attack animation / deal damage
            break;
    }
}

/*eol@eof*/
