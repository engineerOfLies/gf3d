#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

//typedef struct PlayerEntityData_s
//{
//	Uint8	jump_buffer;			//gives the player a #frame jump buffer so they are not frame perfect
//	Uint8	wall_jump_buffer;		//gives the player a #frame wall jump buffer so they are not frame perfect
//	int		last_x_collision_dir;	// holds the last world collision x direction, + for right, - for left
//
//	Uint8	ground_pound_delay;		//freezes the player's movement to perform the ground pound
//	Uint8	ground_pounding;		//is the player ground pounding?
//	Uint8	ground_pound_recovery;	//freezes the player's movement while recovering from the ground
//
//	Uint8	crouching;				//is the player crouching?
//
//	int		coin_count;
//	int		lives_count;
//}PlayerEntityData;


/**
* @brief spawn a new player entity
* @param position where to spawn it
* @return NULL on error, or a pointer to the spawned player entity
*/
Entity* player_new(GFC_Vector3D position);

#endif