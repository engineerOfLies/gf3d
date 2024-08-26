#ifndef __GF3D_DRAW_H__
#define __GF3D_DRAW_H__

#include "gfc_primitives.h"
#include "gfc_shape.h"
#include "gfc_color.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"

/**
 * @brief setup the internals for drawing basic geometric shapes in 3D space
 */
void gf3d_draw_init();

/**
 * @brief draw an edge with the given orientation
 * @param edge the edge to draw.  (x,y,and z are offsets from center)
 * @param position where to move the sphere (final location is position + edge.a.xyz
 * @param rotation how to rotation the edge about point a
 * @param scale how much to scale the scale.  The size of the edge is also a factor
 * @param color the color to draw with
 */
void gf3d_draw_edge_3d(GFC_Edge3D edge,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,float radius,GFC_Color color);

/**
 * @brief draw a cube outline with the given orientation, centered on position
 * @param cube the cube dimensions to draw.  (x,y,and z are offsets from center)
 * @param position where to move the cube center 
 * @param rotation how to rotation the cube about its center
 * @param scale how much to scale the cube.  The size of the cube is also a factor
 * @param color the color to draw with
 */
void gf3d_draw_cube_wireframe(GFC_Box cube,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color);

/**
 * @brief draw an axis aligned box using the box's absolute coordinates
 * @param cube the cube to draw
 * @param color the color to draw with
 */
void gf3d_draw_box_wireframe(GFC_Box cube,GFC_Color color);

/**
 * @brief draw a solid cube with the given orientation, centered on position
 * @param cube the cube dimensions to draw.  (x,y,and z are offsets from center)
 * @param position where to move the cube center 
 * @param rotation how to rotation the cube about its center
 * @param scale how much to scale the cube.  The size of the cube is also a factor
 * @param color the color to draw with
 */
void gf3d_draw_cube_solid(GFC_Box cube,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color);

/**
 * @brief draw a wireframe sphere with the given orientation
 * @param sphere the sphere to draw
 * @param position where to draw the sphere.  final location is position + sphere.xyz
 * @param rotation how to rotate the sphere (makes no sense unless there is non-uniform scaling
 * @param scale how to scale / stretch the sphere.  radius is a factor
 * @param color the color to draw with
 */
void gf3d_draw_sphere_wireframe(GFC_Sphere sphere,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color);

/**
 * @brief draw a solid sphere with the given orientation
 * @param sphere the sphere to draw
 * @param position where to draw the sphere.  final location is position + sphere.xyz
 * @param rotation how to rotate the sphere (makes no sense unless there is non-uniform scaling
 * @param scale how to scale / stretch the sphere.  radius is a factor
 * @param color the color to draw with
 * @param ambient how much to adjust render due to global ambient light
 */
void gf3d_draw_sphere_solid(GFC_Sphere sphere,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color,GFC_Color ambient);

/**
 * @brief draw a circle outline in 3D space
 * @param circle the circle to draw
 * @param position where to draw the sphere.  final location is position + circle.xy
 * @param rotation how to rotate the sphere (makes no sense unless there is non-uniform scaling
 * @param scale how to scale / stretch the circle.  radius is a factor
 * @param color the color to draw with
 */
void gf3d_draw_circle(GFC_Circle circle,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color);

#endif
