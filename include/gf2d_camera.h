#ifndef __GF2D_CAMERA_H__
#define __GF2D_CAMERA_H__

#include <SDL.h>

#include "gfc_vector.h"
#include "gfc_shape.h"

/**
 * @brief set the position and size of the screen gf2d_camera
 * @param x the current left position of the gf2d_camera
 * @param y the current top position of the gf2d_camera
 * @param w the width of the gf2d_camera
 * @param h the height of the gf2d_camera
 */
void gf2d_camera_set_dimensions(Sint32 x,Sint32 y,Uint32 w,Uint32 h);

/**
 * @brief get the position and size of the gf2d_camera
 * @return the gf2d_camera view
 */
GFC_Rect gf2d_camera_get_dimensions();

/**
 * @brief get the width and height of the gf2d_camera view
 * @return the width and height as x and y respectively
 */
GFC_Vector2D gf2d_camera_get_size();

/**
 * @brief get the current gf2d_camera position
 * @return the x,y position of the gf2d_camera
 */
GFC_Vector2D gf2d_camera_get_position();

/**
 * @brief get the offset to draw something relative to the gf2d_camera
 * @return gfc_vector for the offset to draw at (negative gf2d_camera position)
 */
GFC_Vector2D gf2d_camera_get_offset();

/**
 * @brief limit the position of the gf2d_camera by these bounds
 * @param x the minimum x position for the gf2d_camera
 * @param y the minimum y position for the gf2d_camera
 * @param w the right bounds
 * @param w the bottom bounds
 */
void gf2d_camera_set_bounds(Sint32 x,Sint32 y,Uint32 w,Uint32 h);

/**
 * @brief get the current gf2d_camera bounds
 * @return the gf2d_camera bounding rect
 */
GFC_Rect gf2d_camera_get_bounds();

/**
 * @brief snap the gf2d_camera to within the set bounds
 */
void gf2d_camera_bind();

/**
 * @brief move the gf2d_camera, but limit by bounds
 * @param v the gfc_vector to move the gf2d_camera by
 */
void gf2d_camera_move(GFC_Vector2D v);

/**
 * @brief move the gf2d_camera to new position, but respect bounds
 * @param position the ideal new top left position of the gf2d_camera
 */
void gf2d_camera_set_position(GFC_Vector2D position);

/**
 * @brief move the gf2d_camera to center on the provided position, but respect bounds
 * @param position the ideal new center point of the gf2d_camera
 */
void gf2d_camera_set_focus(GFC_Vector2D position);

/**
 * @brief move the gf2d_camera to ne position, but do not respect bounds
 * @param position the new top left position of the gf2d_camera
 */
void gf2d_camera_set_position_absolute(GFC_Vector2D position);

/**
 * @brief center the camera based on the position
 * @param position the new center point for the camera
 */
void gf2d_camera_center_on(GFC_Vector2D position);

#endif
