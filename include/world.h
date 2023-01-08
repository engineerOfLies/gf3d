#ifndef __WORLD_H__
#define __WORLD_H__


#include "gfc_types.h"
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"

#include "gf3d_vgraphics.h"
#include "gf3d_particle.h"
#include "gf3d_model.h"

#include "entity.h"

typedef struct
{
    List    *model_list;
    Vector3D cameraPosition;
    Uint32   now;
    Uint32   hourTime;   //how many ticks must pass before a new day happens
    Uint32   lastHour;   //when did the last day begin
    Matrix4 skyMat;
    Particle theSun;
    Model *sky;
    Mix_Music *backgroundMusic;
    List    *entity_list;
    HashMap *sounds;
}World;

/**
 * @brief get the current date formatted to Y: yyyy D: ddd
 * @param output a TextLine or longer string
 */
void get_date(char *output);

/**
 * @brief get the current date and time formatted to Y: yyyy D: ddd T: hh:00 of a specific day
 * @param output a TextLine or longer string
 * @param day the day to use
 */
void get_date_of(char *output,Uint32 day);


/**
 * @brief convert a specific time to datetime
 * @param output a TextLine or longer string
 * @param day the day to use
 * @param hour the hour to use
 */
void get_datetime_of(char *output,Uint32 day,Uint32 hour);

/**
 * @brief play a sound from the world sound pack
 */
void world_play_sound(const char *sound);

/**
 * @brief get the current date and time formatted to Y: yyyy D: ddd T: hh:00
 * @param output a TextLine or longer string
 */
void get_datetime(char *output);

World *world_load(char *filename);

void world_draw(World *world);

void world_delete(World *world);

void world_run_updates(World *world);

void world_add_entity(World *world,Entity *entity);

ModelMat *world_get_model_mat(World *world,Uint32 index);

#endif
