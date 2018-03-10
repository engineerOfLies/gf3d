#ifndef __GF3D_VECTOR_H__
#define __GF3D_VECTOR_H__

/**
 * gf3d vector
 * @license The MIT License (MIT)
   @copyright Copyright (c) 2018 EngineerOfLies
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#include "gf3d_types.h"

/*
 * The Vector Types
 * Not to be confused with the vector lists from STL
 */
typedef struct
{
  double x;
  double y;
}Vector2D;

typedef struct
{
  double x;
  double y;
  double z;
}Vector3D;

typedef struct
{
  double x;
  double y;
  double z;
  double w;
}Vector4D;

#define vector_in_rect(v, r)  ((v.x >= r.x)&&(v.x < (r.x + r.w))&&(v.y >= r.y)&&(v.y < (r.y + r.h)))

/**
 * @brief create and return an Vector2D
 */
Vector2D vector2d(double x, double y);

/**
 * @brief create and return an Vector3D
 */
Vector3D vector3d(double x, double y, double z);

/**
 * @brief create and return an Vector4D
 */
Vector4D vector4d(double x, double y, double z, double w);

/**
 * @brief sets the outvector to a unit vector pointing at the angle specified
 * @param out output.  Vector will be set to a unit vector rotated by radians
 * @param radians specify the angle of the vector to be set.
 */
void vector3d_set_angle_by_radians(Vector3D *out,float radians);

void vector2d_set_angle_by_radians(Vector2D *out,float radians);

/**
 * @brief returns the angle of a vector in 2D space.
 * @param x the x component of the vector
 * @param y the y component of the vector
 * @return the angle of the vector in radians.
 */
float vector_angle(float x,float y);

/**
 * @brief returns the angle of a vector in 2D space.
 * @param v the vector
 * @return the angle of the vector in radians.
 */
float vector2d_angle(Vector2D v);

/**
 * @brief returns the angle between the two angles.
 * Takes into account wraping around 0 and 360
 * units are degrees
 * @Param a one of the angles in question
 * @Param b one of the angles in question
 * @return the angle, in degrees, between the two angles
 */
float angle_between_degrees(float a, float b);

void angle_clamp_degrees(float *a);

/**
 * 3D Vector Math
 * These macros handle most of the common operations for vector math.
 */

#define vector4d_into_array(ar,vec)   (ar[0] = vec.x,ar[1] = vec.y,ar[2] = vec.z,ar[3] = vec.w)
#define vector3d_into_array(ar,vec)   (ar[0] = vec.x,ar[1] = vec.y,ar[2] = vec.z)
#define vector2d_into_array(ar,vec)   (ar[0] = vec.x,ar[1] = vec.y)

/**
 * @brief calculates the cross product of the two input vectors
 * @param out the resultant cross product
 * @param v1 one of the components
 * @param v2 one of the components
 */
void vector3d_cross_product(Vector3D *out, Vector3D v1, Vector3D v2);

/**
 * @brief Macro to get the dot product from a vector.
 *
 * @param a MglVect3D component of the dot product
 * @param b MglVect3D component of the dot product
 *
 * @return the calculated dot product
 */
#define vector4d_dot_product(a,b)      (a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w)
#define vector3d_dot_product(a,b)      (a.x*b.x+a.y*b.y+a.z*b.z)
#define vector2d_dot_product(a,b)      (a.x*b.x+a.y*b.y)

/**
 * @brief copies the data from one vector into another
 *
 * @param dst the destination vector
 * @param src the source vector
 */
#define vector2d_copy(dst,src)  (dst.x = src.x,dst.y = src.y)
#define vector3d_copy(dst,src)  (dst.x = src.x,dst.y = src.y,dst.z = src.z)
#define vector4d_copy(dst,src)  (dst.x = src.x,dst.y = src.y,dst.z = src.z,dst.w=src.w)

/**
 * @brief checks if vectors are exactly matches of each other
 *
 * @param a one vector to check
 * @param b other vector to check
 */
#define vector2d_equal(a,b) ((a.x == b.x)&&(a.y == b.y))
#define vector3d_equal(a,b) ((a.x == b.x)&&(a.y == b.y)&&(a.z == b.z))
#define vector4d_equal(a,b) ((a.x == b.x)&&(a.y == b.y)&&(a.z == b.z)&&(a.w == b.w))

/**
 * @brief Macro to subtract two vectors
 * varient ending in p takes a pointer to MglVect3D instead.
 * Varients ending with 2D only operate on the x an y components of vectors
 *
 * @param dst result MglVect3D output
 * @param a MglVect3D input
 * @param b MglVect3D input
 */
#define vector2d_sub(dst,a,b)     (dst.x=a.x-b.x,dst.y=a.y-b.y)
#define vector3d_sub(dst,a,b)     (dst.x=a.x-b.x,dst.y=a.y-b.y,dst.z=a.z-b.z)
#define vector4d_sub(dst,a,b)     (dst.x=a.x-b.x,dst.y=a.y-b.y,dst.z=a.z-b.z,dst.w=a.w-b.w)

/**
 * @brief Macro to add two vectors
 * varient ending in p takes a pointer to MglVect3D instead.
 * Varients ending with 2D only operate on the x an y components of vectors
 *
 * @param dst result MglVect3D output
 * @param a MglVect3D input
 * @param b MglVect3D input
 */
#define vector2d_add(dst,a,b)   (dst.x = a.x+b.x,dst.y = a.y+b.y)
#define vector3d_add(dst,a,b)   (dst.x = a.x+b.x,dst.y = a.y+b.y,dst.z = a.z+b.z)
#define vector4d_add(dst,a,b)   (dst.x = a.x+b.x,dst.y = a.y+b.y,dst.z = a.z+b.z,dst.w = a.w+b.w)

/**
 * @brief Macro to scale a vector by a scalar value
 * varient ending in p takes a pointer to MglVect3D instead.
 * Varients ending with 2D only operate on the x an y components of vectors
 *
 * @param dst result MglVect3D output
 * @param src MglVect3D input
 * @Param factpr the scalar value to scale the vector by.
 */
#define vector2d_scale(dst,src,factor) (dst.x = src.x *factor,\
                                         dst.y = src.y *factor)
#define vector3d_scale(dst,src,factor) (dst.x = src.x *factor,\
                                         dst.y = src.y *factor,\
                                         dst.z = src.z *factor)
#define vector4d_scale(dst,src,factor) (dst.x = src.x *factor,\
                                         dst.y = src.y *factor,\
                                         dst.z = src.z *factor,\
                                         dst.w = src.w *factor)
/**
 * @brief Macro that sets vector to zero.
 * @param a MglVect[2D|3D|4D] input
 */

#define vector2d_clear(a)       (a.x=0,a.y=0)
#define vector3d_clear(a)       (a.x=0,a.y=0,a.z=0)
#define vector4d_clear(a)       (a.x=0,a.y=0,a.z=0,a.w=0)

/**
 * @brief Macro to get the negative of a vector
 *
 * @param src MglVect[2D|3D|4D] input
 * @param dst MglVect[2D|3D|4D] negated output
 */
#define vector2d_negate(dst,src)      (dst.x = -1 * src.x,dst.y = -1 * src.y)
#define vector3d_negate(dst,src)      (dst.x = -src.x,dst.y = -src.y,dst.z = -src.z)
#define vector4d_negate(dst,src)      (dst.x = -src.x,dst.y = -src.y,dst.z = -src.z,dst.w = -src.w)

/**
 * @brief Macro to set the components of the vector
 *
 * @param v MglVect3D output
 * @param a float x component
 * @param b float y component
 * @param c float z component (only in 3D & 4D version)
 * @param d float w component (only in 4D version)
 */
#define vector2d_set(v, a, b)  (v.x=(a), v.y=(b))
#define vector3d_set(v, a, b, c)  (v.x=(a), v.y=(b), v.z=(c))
#define vector4d_set(v, a, b, c,d)  (v.x=(a), v.y=(b), v.z=(c), v.w=(d))

Vector2D *vector2d_new();
Vector3D *vector3d_new();
Vector4D *vector4d_new();

Vector2D *vector2d_dup(Vector2D old);
Vector3D *vector3d_dup(Vector3D old);
Vector4D *vector4d_dup(Vector4D old);

/**
 * @brief reflect a vector about a normal.  Simulates "Bounce"
 * @param out the resultant vector
 * @param normal the normal vector for the surface that is providing the bounce
 * @param in the input vector that will bounce off of the normal
 */
void vector2d_reflect(Vector2D *out, Vector2D normal,Vector2D in);
void vector3d_reflect(Vector3D *out, Vector3D normal,Vector3D in);
void vector4d_reflect(Vector4D *out, Vector4D normal,Vector4D in);


/**
 * @brief normalizes the vector passed.  does nothing for a zero length vector.
 *
 * @param v pointer to the vector to be normalized.
 */
float vector2d_magnitude(Vector2D V);
float vector3d_magnitude(Vector3D V);
float vector4d_magnitude(Vector4D V);

void vector2d_normalize (Vector2D *V);
void vector3d_normalize (Vector3D *V);
void vector4d_normalize (Vector4D *V);

/**
 * @brief returns the magnitude squared, which is faster than getting the magnitude
 * which would involve taking the square root of a floating point number.
 * @param V the vector to get the magnitude for
 * @return the square of the magnitude of V
 */
float vector2d_magnitude_squared(Vector2D V);
float vector3d_magnitude_squared(Vector3D V);
float vector4d_magnitude_squared(Vector4D V);

/**
 * @brief checks if the magnitude of V against size.  It does this without
 * doing square roots, which are costly.  It will still do floating point multiplication
 * @param V the vector to check
 * @param size the magnitude to check against
 * @return -1 f V is less than size, 0 if equal or 1 if size is greater than V
 */
int  vector2d_magnitude_compare(Vector2D V,float size);
int  vector3d_magnitude_compare(Vector3D V,float size);
int  vector4d_magnitude_compare(Vector4D V,float size);

/**
 * @brief scales the vector to the specified length without changing direction
 * No op is magnitude is 0 or V is NULL
 * @param V a pointer to the vector to scale
 * @param magnitude the new length for the vector
 */
void vector2d_set_magnitude(Vector2D * V,float magnitude);
void vector3d_set_magnitude(Vector3D * V,float magnitude);
void vector4d_set_magnitude(Vector4D * V,float magnitude);

/**
 * @brief checks if the distance between the two points provided is less than size.
 * @param p1 one point for the distance check
 * @param p2 another point for the distance check
 * @param size the value to check against
 * @return MglTrue if the distance between P1 and P2 is less than size, false otherwise
 */
Bool mgl_distance_between_less_than2d(Vector3D p1,Vector3D p2,float size);
Bool mgl_distance_between_less_than3d(Vector3D p1,Vector3D p2,float size);
Bool mgl_distance_between_less_than4d(Vector3D p1,Vector3D p2,float size);

/**
 * @brief given a rotation, get the component vectors
 * @param angles the input rotation
 * @param forward output optional calculated forward vector
 * @param right output optional calculated right vector
 * @param up output optional calculated up vector
 */
void vector3d_angle_vectors(Vector3D angles, Vector3D *forward, Vector3D *right, Vector3D *up);

/**
 * @brief rotate a 2D vector by the angle specified (in radians)
 * @param in the input vector
 * @param angle the angle to rotate by
 * @return the new vector
 */
Vector2D vector2d_rotate(Vector2D in, float angle);

/**
 * @brief rotate an Vector3D about another vector
 * @param dst output result
 * @param dir the vector to rotate about
 * @param point the point to rotate
 * @param degrees how far to rotate
 */
void vector3d_rotate_about_vector(Vector3D *dst, Vector3D dir, Vector3D point, float degrees);

void vector3d_rotate_about_x(Vector3D *vect, float angle);
void vector3d_rotate_about_y(Vector3D *vect, float angle);
void vector3d_rotate_about_z(Vector3D *vect, float angle);

#endif
