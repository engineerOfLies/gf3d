#include "simple_logger.h"
#include "gfc_types.h"

#include "weapon.h"

void weapon_think(Entity *self);
void weapon_update(Entity *self);
void weapon_free(Entity *self);

Entity *weapon_new(void){

    Entity *wep = NULL;
    Entity *player = NULL;

    wep = entity_new();
    player = entity_get_player();

    if(!wep){
        slog("You should not own a waepon");
        return NULL;
    }

    wep->selectedColor = gfc_color(0,0,0,1);
    wep->color = gfc_color(1,1,1,1);
    wep->model = gf3d_model_load("models/ak47.model");
    wep->scale = vector3d(0.25,0.25,0.25);
    wep->think = weapon_think;
    wep->update = weapon_update;
    wep->free = weapon_free;
    wep->isWeapon = 1;
    vector3d_copy(wep->position, player->position);
    return wep;
}

void weapon_think(Entity *self){

    Entity *player;
    player = entity_get_player();

    if(!self)return;
    if(!player)return;
    self->rotation.z = player->rotation.z;
    self->position.z = -1;
}

void weapon_update(Entity *self){
    Entity *player;
    player = entity_get_player();

    if(!self)return;
    if(!player)return;

    vector3d_copy(self->position, player->position);
}

void weapon_free(Entity *self){
    if(!self)return;
    entity_free(self);
}
