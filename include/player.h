#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"


// typedef struct rlPlayer_S
// {
//     Entity      playerEntity;
//     Vector3D    playerPosition, playerRotation;
//     int         playerNumber;
//     Uint8       inUse;
//
//
//     // void       (*think)(struct rlPlayer_S *self);
//     // void       (*update)(struct rlPlayer_S *self);
//     // void       (*free)(struct rlPlayer_S *self);
// }rlPlayer;

// typedef struct rlStatuses_S
// {
//     int hydration;
//     int saturation;
//     int defication;
//     int sanityation;
//     float calefaction;
// }rlStatuses;

/**
 * @brief Create a new player entity
 * @param position where to spawn the aguman at
 * @return NULL on error, or an player entity pointer on success
 */
Entity *player_new(Vector3D position);

//void player_free(RLPlayer *self);

//void player_think(RLPlayer *self);

//void player_update(RLPlayer *self);




#endif
