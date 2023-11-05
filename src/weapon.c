#include "simple_logger.h"
#include "gfc_types.h"

#include "weapon.h"

void weapon_think(Entity *self);
void weapon_update(Entity *self);
void weapon_free(Entity *self);

Entity *weapon_new(Vector3D position){

    Entity *wep = NULL;

    wep = entity_new();

    if(!wep){
        slog("You should not own a waepon");
        return NULL;
    }

    wep->selectedColor = gfc_color(0,0,0,1);
    wep->color = gfc_color(1,1,1,1);
    wep->model = gf3d_model_load("models/ak47.model");
    wep->think = weapon_think;
    wep->update = weapon_update;
    wep->free = weapon_free;
    wep->isWeapon = 1;
    vector3d_copy(wep->position, position);
    return wep;
}

void weapon_think(Entity *self){

    if(!self)return;


}

void weapon_update(Entity *self){
    if(!self)return;
}

void weapon_free(Entity *self){
    if(!self)return;
}
