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

#include "gf3d_entity.h"

typedef struct
{
    TextLine name;
    Bool     draw_home;
    TextLine backgroundMusic;
    Vector3D center;
    Vector3D camera;
    Vector3D a_side;
    Vector3D b_side;
}CombatZone;

typedef struct
{
    List       *model_list;
    Vector3D    cameraPosition;
    Uint32      now;
    Uint32      hourTime;   //how many ticks must pass before a new day happens
    Uint32      lastHour;   //when did the last day begin
    Matrix4     skyMat;
    Particle    theSun;
    Model      *sky;
    Mix_Music  *backgroundMusic;
    List       *entity_list;
    HashMap    *sounds;
    List       *parking_spots;//places that have been assigned
    Vector3D    parkingStart;//starting location for parking spots
    Vector3D    parkingDelta;//how much space between spots
    Vector3D    parkingApproach;//relay point for going to parking
    Vector3D    parkingEgress;//relay point for leaving parking
    Vector3D    gateApproach;//relay point for going to the stargate
    Vector3D    gateEgress;//relay point for leaving the stargate
    List       *combatZones;
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

/**
 * @brief attempt to claim a spot by position (for loading purposes)
 * @param spot the location you want
 * @return the same location if you were successful, a different one if it was already taken
 */
Vector3D world_parking_claim_spot(Vector3D spot);

/**
 * @brief check if the spot is occupied
 * @param spot the spot to check
 * @return NULL if it is empty, or the pointer to the location otherwise
 */
Vector3D *world_parking_spot_get_by_location(Vector3D spot);

/**
 * @brief get an empty parking spot and claim it
 * @return (-1,-1,-1) if the parking lot is full, or the position of your spot otherwise
 */
Vector3D world_parking_get_spot();

/**
 * @brief get the approach vector for the parking lot
 */
Vector3D world_get_parking_approach();

/**
 * @brief get the exit vector for the parking lot
 */
Vector3D world_get_parking_egress();

/**
 * @brief get the approach vector for the FSD gate
 */
Vector3D world_get_gate_approach();

/**
 * @brief get the exit vector for the FSD Gate
 */
Vector3D world_get_gate_egress();

/**
 * @brief let the parking lot know that you are done with it
 * @param spot the spot you are giving up
 */
void world_parking_vacate_spot(Vector3D spot);

/**
 * @brief get combat zone information by its name
 * @param name the name of the combat zone (see the def file for options)
 * @return NULL if not found, the CombatZone data otherwise
 */
CombatZone *world_get_combat_zone_by_name(const char *name);


#endif
