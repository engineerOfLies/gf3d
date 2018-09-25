#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gf3d_matrix.h"
#include "simple_logger.h"

void gf3d_matrix_slog(Matrix4 mat)
{
    slog("%f,%f,%f,%f",mat[0][0],mat[0][1],mat[0][2],mat[0][3]);
    slog("%f,%f,%f,%f",mat[1][0],mat[1][1],mat[1][2],mat[1][3]);
    slog("%f,%f,%f,%f",mat[2][0],mat[2][1],mat[2][2],mat[2][3]);
    slog("%f,%f,%f,%f",mat[3][0],mat[3][1],mat[3][2],mat[3][3]);
}

void gf3d_matrix_copy(
    Matrix4 d,
    Matrix4 s
  )
{
    if ((!d)||(!s))return;
    if (d == s)return;
    memcpy(d,s,sizeof(Matrix4));
}


void gf3d_matrix_multiply(
    Matrix4 out,
    Matrix4 m1,
    Matrix4 m2
  )
{

  out[0][0] = m2[0][0]*m1[0][0] + m2[0][1]*m1[1][0] + m2[0][2]*m1[2][0] + m2[0][3]*m1[3][0];
  out[0][1] = m2[0][0]*m1[0][1] + m2[0][1]*m1[1][1] + m2[0][2]*m1[2][1] + m2[0][3]*m1[3][1];
  out[0][2] = m2[0][0]*m1[0][2] + m2[0][1]*m1[1][2] + m2[0][2]*m1[2][2] + m2[0][3]*m1[3][2];
  out[0][3] = m2[0][0]*m1[0][3] + m2[0][1]*m1[1][3] + m2[0][2]*m1[2][3] + m2[0][3]*m1[3][3];

  out[1][0] = m2[1][0]*m1[0][0] + m2[1][1]*m1[1][0] + m2[1][2]*m1[2][0] + m2[1][3]*m1[3][0];
  out[1][1] = m2[1][0]*m1[0][1] + m2[1][1]*m1[1][1] + m2[1][2]*m1[2][1] + m2[1][3]*m1[3][1];
  out[1][2] = m2[1][0]*m1[0][2] + m2[1][1]*m1[1][2] + m2[1][2]*m1[2][2] + m2[1][3]*m1[3][2];
  out[1][3] = m2[1][0]*m1[0][3] + m2[1][1]*m1[1][3] + m2[1][2]*m1[2][3] + m2[1][3]*m1[3][3];

  out[2][0] = m2[2][0]*m1[0][0] + m2[2][1]*m1[1][0] + m2[2][2]*m1[2][0] + m2[2][3]*m1[3][0];
  out[2][1] = m2[2][0]*m1[0][1] + m2[2][1]*m1[1][1] + m2[2][2]*m1[2][1] + m2[2][3]*m1[3][1];
  out[2][2] = m2[2][0]*m1[0][2] + m2[2][1]*m1[1][2] + m2[2][2]*m1[2][2] + m2[2][3]*m1[3][2];
  out[2][3] = m2[2][0]*m1[0][3] + m2[2][1]*m1[1][3] + m2[2][2]*m1[2][3] + m2[2][3]*m1[3][3];

  out[3][0] = m2[3][0]*m1[0][0] + m2[3][1]*m1[1][0] + m2[3][2]*m1[2][0] + m2[3][3]*m1[3][0];
  out[3][1] = m2[3][0]*m1[0][1] + m2[3][1]*m1[1][1] + m2[3][2]*m1[2][1] + m2[3][3]*m1[3][1];
  out[3][2] = m2[3][0]*m1[0][2] + m2[3][1]*m1[1][2] + m2[3][2]*m1[2][2] + m2[3][3]*m1[3][2];
  out[3][3] = m2[3][0]*m1[0][3] + m2[3][1]*m1[1][3] + m2[3][2]*m1[2][3] + m2[3][3]*m1[3][3];
}

void gf3d_matrix_multiply_vector4d(
  Vector4D * out,
  Matrix4    mat,
  Vector4D   vec
)
{
  double x,y,z,w;
  double ox,oy,oz,ow;
  if (!out)return;
  x=vec.x;
  y=vec.y;
  z=vec.z;
  w=vec.w;
  ox=x*mat[0][0] + y*mat[1][0] + mat[2][0]*z + mat[3][0]*w;
  oy=x*mat[0][1] + y*mat[1][1] + mat[2][1]*z + mat[3][1]*w;
  oz=x*mat[0][2] + y*mat[1][2] + mat[2][2]*z + mat[3][2]*w;
  ow=x*mat[0][3] + y*mat[1][3] + mat[2][3]*z + mat[3][3]*w;
  out->x = ox;
  out->y = oy;
  out->z = oz;
  out->w = ow;
}

void gf3d_matrix_zero(Matrix4 zero)
{
    memset(zero,0,sizeof(Matrix4));
}

void gf3d_matrix_identity(Matrix4 one)
{
    gf3d_matrix_zero(one);
    one[0][0] = 1;
    one[1][1] = 1;
    one[2][2] = 1;
    one[3][3] = 1;
}


void gf3d_matrix_perspective(
    Matrix4     out,
    double      fov,
    double      aspect,
    double      near,
    double      far
)
{
    double halftanfov = tan(fov * 0.5);
    gf3d_matrix_zero(out);

    if (aspect == 0)
    {
        slog("gf3d_matrix_perspective: aspect ratio cannot be zero");
        return;
    }
    if (halftanfov == 0)
    {
        slog("gf3d_matrix_perspective: bad fov");
        return;
    }
    if (near == far)
    {
        slog("gf3d_matrix_perspective: near plane and far plane cannot be the same");
        return;
    }

    gf3d_matrix_zero(out);
    out[0][0] = 1 / (aspect * halftanfov);
    out[1][1] = 1 / (halftanfov);
    out[2][2] = - ((far + near) / (far - near));
    out[2][3] = -1;
    if ((far - near) == 0)
    {
        out[3][2] = 0;
    }
    else
    out[3][2] = -(2 * far * near) / (far - near);
    return;
}

void gf3d_matrix_view(
    Matrix4  out,
    Vector3D position,
    Vector3D target,
    Vector3D up
)
{
    Vector3D f,s,u;
    vector3d_sub(f,target,position);
    vector3d_normalize(&f);
    
    vector3d_cross_product(&s,f,up);
    vector3d_normalize(&s);
    
    vector3d_cross_product(&u,s,f);
 
    gf3d_matrix_identity(out);
    out[0][0] = s.x;
    out[1][0] = s.y;
    out[2][0] = s.z;
    out[0][1] = u.x;
    out[1][1] = u.y;
    out[2][1] = u.z;
    out[0][2] = -f.x;
    out[1][2] = -f.y;
    out[2][2] = -f.z;
    out[3][0] = vector3d_dot_product(s, position)?-vector3d_dot_product(s, position):0;
    out[3][1] = vector3d_dot_product(u, position)?-vector3d_dot_product(u, position):0;
    out[3][2] = vector3d_dot_product(f, position)?vector3d_dot_product(f, position):0;
    
}

void gf3d_matrix_make_translation(
    Matrix4 out,
    Vector3D move
)
{
    if (!out)return;
    gf3d_matrix_identity(out);
    out[0][3] = move.x;
    out[1][3] = move.y;
    out[2][3] = move.z;
}

void gf3d_matrix_translate(
    Matrix4 out,
    Vector3D move
)
{
    Matrix4 translate,temp;
    gf3d_matrix_make_translation(translate,move);
    gf3d_matrix_multiply(temp,translate,out);
    gf3d_matrix_copy(out,temp);
}
