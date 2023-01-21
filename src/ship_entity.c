#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_list.h"

#include "gf3d_draw.h"

#include "config_def.h"
#include "player.h"
#include "station.h"
#include "ship.h"
#include "ship_entity.h"


void ship_entity_update(Entity *self);
void ship_entity_draw(Entity *self);
void ship_entity_think(Entity *self);
void ship_entity_think_moving(Entity *self);
void ship_entity_free(Entity *self);

Entity *ship_entity_new(Vector3D position,Ship *data,Color detailColor)
{
    const char *str;
    SJson *def;
    Entity *ent = NULL;    
    if (!data)
    {
        slog("ship_entity: null ship data provided");
        return NULL;
    }
    def = config_def_get_by_name("ships",data->name);
    if (!def)return NULL;
    ent = gf3d_entity_new();
    if (!ent)return NULL;
    
    str = sj_object_get_value_as_string(def,"model");
    if (str)
    {
        ent->mat.model = gf3d_model_load(str);
        if (!ent->mat.model)
        {
            slog("SHIP MODEL DID NOT LOAD");
        }
    }
    else
    {
        slog("no model specified for ship %s",data->name);
    }

    gfc_line_cpy(ent->name,data->displayName);
    sj_value_as_vector3d(sj_object_get_value(def,"scale"),&ent->mat.scale);
    sj_value_as_vector3d(sj_object_get_value(def,"rotation"),&ent->mat.rotation);
    vector3d_copy(ent->mat.position,position);
    
    ent->selectedColor = gfc_color(0.9,0.7,0.1,1);
    ent->color = gfc_color(1,1,1,1);
    gfc_color_copy(ent->detailColor,detailColor);
    ent->think = ship_entity_think;
    ent->draw = ship_entity_draw;
    ent->update = ship_entity_update;
    ent->free = ship_entity_free;
    ent->data = data;
    vector3d_copy(ent->mat.position,position);
    return ent;
}

void ship_entity_move_to(Entity *ent,Uint32 pathIndex,const char *dockName)
{
    Vector3D *v;
    Ship *ship;
    if ((!ent)||(!ent->data))return;
    ship = ent->data;
    v = gfc_list_get_nth(ship->flightPath,pathIndex);
    if (!v)
    {
        slog("Path Complete");
        gfc_line_sprintf(ship->location,"docked");
        ent->think = ship_entity_think;
        return;
    }
    if ((dockName)&&(strlen(dockName)))gfc_line_cpy(ship->dockName,dockName);
    else gfc_line_clear(ship->dockName);
    ship->flightStep = pathIndex;
    vector3d_copy(ent->targetPosition,(*v));
    ent->targetComplete = 0;
    ent->counter = pathIndex;
    ent->think = ship_entity_think_moving;
}

void ship_entity_free(Entity *self)
{
    // the ship entity doesn't own the Ship data, ship data owns this
}

void ship_entity_update(Entity *self)
{
    Ship *ship;
    if ((!self)||(!self->data))
    {
        slog("self pointer not provided");
        return;
    }
    ship = self->data;
    //sync to data
    vector3d_copy(ship->position,self->mat.position);
    ship->flightStep = self->counter;
}

void ship_entity_draw(Entity *self)
{
    int i,c;
    ModelMat *mat;
    Matrix4 thrustMatrix;
    Ship *data;
    if ((!self)||(!self->data))return;
    data = self->data;
    if ((strcmp(data->location,"docked")==0)||
        (strcmp(data->location,"interstellar")==0))
    {
        self->hidden = 1;
        return;// inside the station
    }
    self->hidden = 0;
    if (!vector3d_is_zero(self->velocity))
    {
        //draw thrust
        c = gfc_list_get_count(data->thrust);
        for (i = 0;i < c; i++)
        {
            mat = gfc_list_get_nth(data->thrust,i);
            if (!mat)continue;
            mat_from_parent(
                thrustMatrix,
                self->mat.mat,
                mat->position,
                mat->rotation,
                mat->scale);            
            gf3d_model_draw(mat->model,0,thrustMatrix,vector4d(1,.9,.5,1),vector4d(1,1,1,1),vector4d(1,1,1,1));
        }
    }
}

void ship_entity_think_moving(Entity *self)
{
    StationSection *section;
    Vector3D dir;
    Ship *ship;
    StationData *station;
    if (!self)return;
    ship = self->data;
    // do maintenance
    ship_check(ship);
    if (vector3d_distance_between_less_than(self->mat.position,self->targetPosition,1))
    {
        self->think = ship_entity_think;
        self->targetComplete = 1;
        vector3d_clear(self->velocity);
        ship_entity_move_to(self,++self->counter,ship->dockName);
        slog("at waypoint");
        return;
    }
    if (vector3d_distance_between_less_than(self->mat.position,self->targetPosition,self->speed *0.01))
    {
        self->think = ship_entity_think;
        self->targetComplete = 1;
        slog("closing distance");
        vector3d_clear(self->velocity);
        vector3d_set_magnitude(&self->velocity,vector3d_magnitude_between(self->mat.position,self->targetPosition));
        return;
    }
    vector3d_sub(dir,self->targetPosition,self->mat.position);
    vector3d_normalize(&dir);
    vector3d_scale(self->velocity,dir,(self->speed*0.01));

    //check if we need to set our rotations
    if (strlen(ship->dockName)>0)
    {
        station = player_get_station_data();
        section = station_get_section_by_display_name(station,ship->dockName);
        if ((section)&&(section->rotates))
        {
            if (vector3d_compare(self->targetPosition,station_section_get_docking_position(section)))
            {
                //if our next position is the docking position, lets match the station rotation
                self->roll = -station->sectionRotation * section->rotates;
                //hack: if our y position is positive, we are approaching from opposite, reverse the rotation
                if (self->mat.position.y > 0)self->roll *= -1;
            }
        }
    }
}

void ship_entity_think(Entity *self)
{
    if (!self)return;
    // do maintenance
}

/*eol@eof*/
