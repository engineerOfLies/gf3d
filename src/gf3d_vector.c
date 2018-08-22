#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gf3d_vector.h"
#include "simple_logger.h"

Vector2D vector2d(double x, double y)
{
  Vector2D vec;
  vector2d_set(vec,x, y);
  return vec;
}

Vector3D vector3d(double x, double y, double z)
{
  Vector3D vec;
  vector3d_set(vec,x, y, z);
  return vec;
}

Vector4D vector4d(double x, double y, double z, double w)
{
  Vector4D vec;
  vector4d_set(vec,x, y, z, w);
  return vec;
}

float vector2d_magnitude (Vector2D V)
{
  return sqrt (V.x * V.x + V.y * V.y);
}

float vector3d_magnitude (Vector3D V)
{
  return sqrt (V.x * V.x + V.y * V.y + V.z * V.z);
}

float vector4d_magnitude (Vector4D V)
{
  return sqrt (V.x * V.x + V.y * V.y + V.z * V.z + V.w * V.w);
}

float vector2d_magnitude_squared(Vector2D V)
{
  return (V.x * V.x + V.y * V.y);
}

float vector3d_magnitude_squared(Vector3D V)
{
  return (V.x * V.x + V.y * V.y + V.z * V.z);
}

float vector4d_magnitude_squared(Vector4D V)
{
  return (V.x * V.x + V.y * V.y + V.z * V.z + V.w * V.w);
}

void vector2d_set_magnitude(Vector2D * V,float magnitude)
{
  if (!V)return;
  vector2d_normalize(V);
  V->x *= magnitude;
  V->y *= magnitude;
}

void vector3d_set_magnitude(Vector3D * V,float magnitude)
{
  if (!V)return;
  vector3d_normalize(V);
  V->x *= magnitude;
  V->y *= magnitude;
  V->z *= magnitude;
}

void vector4d_set_magnitude(Vector4D * V,float magnitude)
{
  if (!V)return;
  vector4d_normalize(V);
  V->x *= magnitude;
  V->y *= magnitude;
  V->z *= magnitude;
  V->w *= magnitude;
}

int vector2d_magnitude_compare(Vector2D V,float size)
{
    float ms = vector2d_magnitude_squared(V);
    float ss = size * size;
    if (ms < ss)return -1;
    if (ms == ss)return 0;
    return 1;
}

int vector3d_magnitude_compare(Vector3D V,float size)
{
    float ms = vector3d_magnitude_squared(V);
    float ss = size * size;
    if (ms < ss)return -1;
    if (ms == ss)return 0;
    return 1;
}

int vector4d_magnitude_compare(Vector4D V,float size)
{
    float ms = vector4d_magnitude_squared(V);
    float ss = size * size;
    if (ms < ss)return -1;
    if (ms == ss)return 0;
    return 1;
}


void vector3d_set_angle_by_radians(Vector3D *out,float radians)
{
  if(!out)return;
  out->x = cos(radians);
  out->y = sin(radians);
}

void vector4d_set_angle_by_radians(Vector4D *out,float radians)
{
  if(!out)return;
  out->x = cos(radians);
  out->y = sin(radians);
}

void vector2d_normalize (Vector2D *V)
{
  float M;
  if (!V)return;
  M = vector2d_magnitude (*V);
  if (M == 0.0f)
  {
    return;
  }
  M = 1/M;
  V->x *= M;
  V->y *= M;
}

void vector3d_normalize (Vector3D *V)
{
  float M;
  if (!V)return;
  M= vector3d_magnitude (*V);
  if (M == 0.0f)
  {
    return;
  }
  M = 1/M;
  V->x *= M;
  V->y *= M;
  V->z *= M;
}

void vector4d_normalize (Vector4D *V)
{
  float M;
  if (!V)return;
  M = vector4d_magnitude (*V);
  if (M == 0.0f)
  {
    return;
  }
  M = 1/M;
  V->x *= M;
  V->y *= M;
  V->z *= M;
  V->w *= M;
}

Vector2D *vector2d_dup(Vector2D old)
{
  Vector2D *duped = NULL;
  duped = vector2d_new();
  if (!duped)return NULL;
  vector2d_copy((*duped),old);
  return duped;
}

Vector3D *vector3d_dup(Vector3D old)
{
  Vector3D *duped = NULL;
  duped = vector3d_new();
  if (!duped)return NULL;
  vector3d_copy((*duped),old);
  return duped;
}

Vector4D *vector4d_dup(Vector4D old)
{
  Vector4D *duped = NULL;
  duped = vector4d_new();
  if (!duped)return NULL;
  vector4d_copy((*duped),old);
  return duped;
}

Vector2D *vector2d_new()
{
  Vector2D *vec = NULL;
  vec = (Vector2D *)malloc(sizeof(Vector2D));
  if (vec == NULL)
  {
    return NULL;
  }
  vector2d_clear((*vec));
  return vec;
}

Vector3D *vector3d_new()
{
  Vector3D *vec = NULL;
  vec = (Vector3D *)malloc(sizeof(Vector3D));
  if (vec == NULL)
  {
    return NULL;
  }
  vector3d_clear((*vec));
  return vec;
}

Vector4D *vector4d_new()
{
  Vector4D *vec = NULL;
  vec = (Vector4D *)malloc(sizeof(Vector4D));
  if (vec == NULL)
  {
    return NULL;
  }
  vector4d_clear((*vec));
  return vec;
}

void vector3d_project_to_plane( Vector3D *dst, Vector3D p, Vector3D normal )
{
  float d,f;
  Vector3D n;
  float inv_denom;
  if (!dst)return;
  f = vector3d_dot_product( normal, normal );
  if (!f)return;
  inv_denom = 1.0F / f;
  
  d = vector3d_dot_product( normal, p ) * inv_denom;
  
  n.x = normal.x * inv_denom;
  n.y = normal.y * inv_denom;
  n.z = normal.z * inv_denom;
  
  dst->x = p.z - d * n.x;
  dst->y = p.y - d * n.y;
  dst->z = p.x - d * n.z;
}


void vector3d_perpendicular( Vector3D *dst, Vector3D src )
{
  int pos;
  float minelem = 1.0F;
  Vector3D tempvec;
  if (!dst)return;
  /*
   * * find the smallest magnitude axially aligned vector
   */
  pos=0;
  
  if ( fabs( src.x ) < minelem )
  {
    pos=0;
    minelem = fabs( src.x );
  }
  if ( fabs( src.y ) < minelem )
  {
    pos=1;
    minelem = fabs( src.y );
  }
  if ( fabs( src.y ) < minelem )
  {
    pos=2;
    minelem = fabs( src.z );
  }
  
  tempvec.x=0;
  tempvec.y=0;
  tempvec.z=0;
  
  switch(pos)
  {
    case 0:
      tempvec.x=1;
      break;
    case 1:
      tempvec.y=1;
      break;
    case 2:
      tempvec.z=1;
      break;
  }
  
  /*
   * * project the point onto the plane defined by src
   */
  vector3d_project_to_plane( dst, tempvec, src );
  
  /*
   * * normalize the result
   */
  vector3d_normalize( dst );
}

void rotation_concacenate(float in1[3][3], float in2[3][3], float out[3][3])
{
  out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
  in1[0][2] * in2[2][0];
  out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
  in1[0][2] * in2[2][1];
  out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
  in1[0][2] * in2[2][2];
  out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
  in1[1][2] * in2[2][0];
  out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
  in1[1][2] * in2[2][1];
  out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
  in1[1][2] * in2[2][2];
  out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
  in1[2][2] * in2[2][0];
  out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
  in1[2][2] * in2[2][1];
  out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
  in1[2][2] * in2[2][2];
}

void vector3d_rotate_about_vector(Vector3D *dst, Vector3D dir, Vector3D point, float degrees)
{
  float m[3][3];
  float im[3][3];
  float zrot[3][3];
  float tmpmat[3][3];
  float rot[3][3];
  Vector3D vr, vup, vf;
  
  if (!dst)return;
  
  vf.x = dir.x;
  vf.y = dir.y;
  vf.z = dir.z;
  
  vector3d_perpendicular( &vr, dir );
  vector3d_cross_product(&vup, vr, vf);

  m[0][0] = vr.x;
  m[1][0] = vr.y;
  m[2][0] = vr.z;
  
  m[0][1] = vup.x;
  m[1][1] = vup.y;
  m[2][1] = vup.z;
  
  m[0][2] = vf.x;
  m[1][2] = vf.y;
  m[2][2] = vf.z;
  
  memcpy( im, m, sizeof( im ) );
  
  im[0][1] = m[1][0];
  im[0][2] = m[2][0];
  im[1][0] = m[0][1];
  im[1][2] = m[2][1];
  im[2][0] = m[0][2];
  im[2][1] = m[1][2];
  
  memset( zrot, 0, sizeof( zrot ) );
  zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;
  
  zrot[0][0] = cos( ( degrees*GF3D_DEGTORAD ) );
  zrot[0][1] = sin( ( degrees*GF3D_DEGTORAD ) );
  zrot[1][0] = -sin( ( degrees*GF3D_DEGTORAD ) );
  zrot[1][1] = cos( ( degrees*GF3D_DEGTORAD ) );
  
  rotation_concacenate( m, zrot, tmpmat );
  rotation_concacenate( tmpmat, im, rot );
  
  dst->x = rot[0][0] * point.x + rot[0][1] * point.y + rot[0][2] * point.z;
  dst->y = rot[1][0] * point.x + rot[1][1] * point.y + rot[1][2] * point.z;
  dst->z = rot[2][0] * point.x + rot[2][1] * point.y + rot[2][2] * point.z;
}

void vector3d_rotate_about_x(Vector3D *vect, float angle)
{
  Vector3D temp;
  if (!vect)return;
  
  angle=angle*GF3D_DEGTORAD;
  
  temp.x=vect->x;
  temp.y=(vect->y*cos(angle))-(vect->z*sin(angle));
  temp.z=(vect->y*sin(angle))+(vect->z*cos(angle));
  
  vect->x=temp.x;
  vect->y=temp.y;
  vect->z=temp.z;
}

void vector3d_rotate_about_y(Vector3D *vect, float angle)
{
  Vector3D temp;
  if (!vect)return;
  
  angle=angle*GF3D_DEGTORAD;
  
  temp.y=vect->y;
  temp.x=(vect->x*cos(angle))+(vect->z*sin(angle));
  temp.z=(vect->x*sin(angle)*(-1))+(vect->z*cos(angle));
  
  vect->x=temp.x;
  vect->y=temp.y;
  vect->z=temp.z;
}

void vector3d_rotate_about_z(Vector3D *vect, float angle)
{
  Vector3D temp;
  if (!vect)return;
  
  angle=angle*GF3D_DEGTORAD;
  
  temp.z=vect->z;
  temp.x=(vect->x*cos(angle))-(vect->y*sin(angle));
  temp.y=(vect->x*sin(angle))+(vect->y*cos(angle));
  
  vect->x=temp.x;
  vect->y=temp.y;
  vect->z=temp.z;
}


void vector3d_angle_vectors(Vector3D angles, Vector3D *forward, Vector3D *right, Vector3D *up)
{
  float angle;
  float sr, sp, sy, cr, cp, cy;
  
  angle = angles.x * (GF3D_DEGTORAD);
  sy = sin(angle);
  cy = cos(angle);
  angle = angles.y * (GF3D_DEGTORAD);
  sp = sin(angle);
  cp = cos(angle);
  angle = angles.z * (GF3D_DEGTORAD);
  sr = sin(angle);
  cr = cos(angle);
  
  if(forward)
  {
    forward->x = cp*cy;
    forward->y = cp*sy;
    forward->z = -sp;
  }
  if(right)
  {
    right->x = (-1*sr*sp*cy+-1*cr*-sy);
    right->y = (-1*sr*sp*sy+-1*cr*cy);
    right->z = -1*sr*cp;
  }
  if(up)
  {
    up->x = (cr*sp*cy+-sr*-sy);
    up->y = (cr*sp*sy+-sr*cy);
    up->z = cr*cp;
  }
}

float vector2d_angle(Vector2D v)
{
  return vector_angle(v.x,v.y);
}

Vector2D vector2d_rotate(Vector2D in, float angle)
{
    Vector2D out;
    out.x = in.x * cos(angle) - in.y * sin(angle); // now x is something different than original vector x
    out.y = in.x * sin(angle) + in.y * cos(angle);
    return out;
}

float vector_angle(float x,float y)
{
  float angle = atan2(y, x) + M_PI;
  float fraction = angle * 0.5 / M_PI;
  if (fraction >= 1.0)
  {
    fraction -= 1.0;
  }
  return (fraction * 360)-180;
}

void angle_clamp_degrees(float *a)
{
  if (!a)return;
  while (*a >= 360)*a -= 360;
  while (*a < 0)*a += 360;
}

float angle_between_degrees(float a, float b)
{
  float angle;
  angle = fabs(a - b);
  while (angle >= 360)angle -= 360;
  while (angle < 0)angle += 360;
  if (angle > 180)angle -= 180;
  return angle;
}

void vector2d_reflect(Vector2D *out, Vector2D normal,Vector2D in)
{
  float f;
  f = vector2d_dot_product(in,normal);
  out->x = in.x - (2 * normal.x * f);
  out->y = in.y - (2 * normal.y * f);
}

void vector3d_reflect(Vector3D *out, Vector3D normal,Vector3D in)
{
  float f;
  f = vector3d_dot_product(in,normal);
  out->x = in.x - (2 * normal.x * f);
  out->y = in.y - (2 * normal.y * f);
  out->z = in.z - (2 * normal.z * f);
}

void vector4d_reflect(Vector4D *out, Vector4D normal,Vector4D in)
{
  float f;
  f = vector4d_dot_product(in,normal);
  out->x = in.x - (2 * normal.x * f);
  out->y = in.y - (2 * normal.y * f);
  out->z = in.z - (2 * normal.z * f);
  out->w = in.w - (2 * normal.w * f);
}

void vector3d_cross_product(Vector3D *out, Vector3D v1, Vector3D v2)
{
  out->x = v1.y*v2.z - v1.z*v2.y;
  out->y = v1.z*v2.x - v1.x*v2.z;
  out->z = v1.x*v2.y - v1.y*v2.x;
}

/*eol@eof*/
