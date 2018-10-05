#ifndef __GF3D_MATRIX_H__
#define __GF3D_MATRIX_H__

#include "gf3d_vector.h"

typedef float Matrix4[4][4];


/**
 * @brief copy the contents of one matrix into another
 * @param d the destination matrix
 * @param s the source matrix
 */
void gf3d_matrix_copy(
    Matrix4 d,
    Matrix4 s
  );

/**
 * @brief set the matrix to an identity matrix
 * @param one the matrix to become an identity
 */
void gf3d_matrix_identity(Matrix4 one);

/**
 * @brief set the matrix to a zero matrix
 * @param zero the matrix to be set to zero
 */
void gf3d_matrix_zero(Matrix4 zero);

/**
 * @brief create a translation matrix given the vector
 * @param out the output matrix, the contents of this matrix are overwritten
 * @param move the vector describing the translation
 */
void gf3d_matrix_make_translation(
    Matrix4 out,
    Vector3D move
);

/**
 * @brief setup a view matrix for a frustum centered at position, pointed at target, with up as the up direction
 * @note adapted from glm
 * @param out output matrix
 * @param position position of the "camera"
 * @param target location to look
 * @param up the direction considered "up"
 */
void gf3d_matrix_view(
    Matrix4  out,
    Vector3D position,
    Vector3D target,
    Vector3D up
);

void gf3d_matrix_slog(Matrix4 mat);

/**
 * @brief setup a perspective projection matrix
 * @note adapted from glm
 * @param out the output matrix
 * @param fov the field of view
 * @param aspect aspect ration (screen width / screen height)
 * @param near the near z plane
 * @param far the far z plane
 */
void gf3d_matrix_perspective(
    Matrix4     out,
    float      fov,
    float      aspect,
    float      near,
    float      far
);


/**
 * @brief multiply the two input matrices together and save the result into out
 * @note this is not safe if out is one of the inputs
 * @param out the output matrix
 * @param a one multiplicand matrix
 * @param b another multiplicand matrix
 */
void gf3d_matrix_multiply(
    Matrix4 out,
    Matrix4 a,
    Matrix4 b
  );

/**
 * @brief multiply a vector by the matrix, saving the result in an vector
 * @param out a pointer to the vector that will hold the result
 * @param mat input matrix to multiply by
 * @param vec input matrix to multiply by
 */
void gf3d_matrix_multiply_vector4d(
    Vector4D * out,
    Matrix4    mat,
    Vector4D   vec
);

/**
 * @brief multiply a matrix by the rotation matrix
 * @param out the output matrix
 * @param in  the input matrix
 * @param degree the amount, in radians, to rotate by
 * @param axis the axis about which to rotate
 */
void gf3d_matrix_rotate(
    Matrix4     out,
    Matrix4     in,
    float       degree,
    Vector3D    axis
);

#endif
