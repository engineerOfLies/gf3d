#ifndef __GF2D_DRAW_H__
#define __GF2D_DRAW_H__

#include <SDL.h>
#include "gfc_list.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "gfc_color.h"


/**
 * @brief setup the draw manager internals
 * @param ttl set how long drawn shapes stay in memory for reuse
 */
void gf2d_draw_manager_init(Uint32 ttl);

/**
 * @brief draw the shape with the color specified
 * @param shape the shape to draw
 * @param color the color to draw with
 * @param offset a positional offset to draw the space at
 */
void gf2d_draw_shape(Shape shape,Color color,Vector2D offset);

/**
 * @brief draw a pixel to the current rendering context
 * @param pixel the screen position to draw at
 * @param color the color to draw with
 */
void gf2d_draw_pixel(Vector2D pixel,Color color);

/**
 * @brief draw a circle outline
 * @param center the center of the circle to draw
 * @param radius the radius of the circle
 * @param color the color to draw with
 */
void gf2d_draw_circle(Vector2D center, int radius, Color color);

/**
 * @brief draw a rectanlge outline
 * @param rect the rect to draw
 * @param color the color to draw with
 */
void gf2d_draw_rect(Rect rect,Color color);

/**
 * @brief draw a solid rectanlge
 * @param rect the rect to draw
 * @param color the color to draw with
 */
void gf2d_draw_rect_filled(Rect rect,Color color);

/**
 * @brief draw a diamon shape centered around a point
 * @param center the center of the diamond to draw
 * @param radius the distance from center to each point
 * @param color draw with this color
 */
void gf2d_draw_diamond(Vector2D center, int radius, Color color);

/**
 * @brief draw a line to the current rendering context
 * @param p1 starting point
 * @param p2 ending point
 * @param color the color to draw with
 */
void gf2d_draw_line(Vector2D p1,Vector2D p2, Color color);

/**
 * @brief use the bezier algorithm to render a curve
 * @param p0 starting point
 * @param p1 reference point
 * @param p2 ending point
 * @param color rendering color
 */
void gf2d_draw_bezier_curve(Vector2D p0, Vector2D p1, Vector2D p2,Color color);

/**
 * @brief draw a 4 point bezier curve given two end points and two reference points
 * @param p0 first end point
 * @param r0 first reference point
 * @param p1 second end point
 * @param r1 second reference point
 * @param color rendering color
 */
void gf2d_draw_bezier4_curve(Vector2D p0,Vector2D r0,Vector2D r1,Vector2D p1,Color color);

/**
 * @brief draw a closed polygon described by the list of Vector2D points provided
 * @param points a list containing Vector2D points describing a polygon
 * @param p2 ending point
 * @param color the color to draw with
 */
void gf2d_draw_polygon(List *points,Color color);

#endif
