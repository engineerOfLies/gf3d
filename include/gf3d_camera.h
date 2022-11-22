#ifndef __GF3D_CAMERA_H__
#define __GF3D_CAMERA_H__

#include "gfc_matrix.h"

typedef struct
{
    Matrix4 cameraMat;      //final matrix to become the view matrix
    Matrix4 cameraMatInv;   //final matrix to become the inverse view matrix
    Vector3D scale;
    Vector3D position;
    Vector3D rotation;      // pitch, roll, yaw
}Camera;

/**
 * @brief take the position,scale, and rotation to calculate the view matrix
 * @note: Do not use if you are tailoring the camera matrix by hand
 */
void gf3d_camera_update_view();

/**
 * @brief get the current camera view
 * @param view output, the matrix provided will be populated with the current camera information
 */
void gf3d_camera_get_view_mat4(Matrix4 *view);

/**
 * @brief set the current camera based on the matrix provided
 */
void gf3d_camera_set_view_mat4(Matrix4 *view);

/**
 * @brief set the camera properties based on position and direction that the camera should be looking
 * @param position the location for the camera
 * @param target the point the camera should be looking at
 * @param up the direction considered to be "up"
 */
void gf3d_camera_look_at(
    Vector3D position,
    Vector3D target,
    Vector3D up
);

/**
 * @brief move the camera "FORWARD" relative to the view angles of the camera
 * @param magnitude how much to move
 */
void gf3d_camera_fly_forward(float magnitude);

/**
 * @brief move the camera "RIGHT" relative to the view angles of the camera
 * @param magnitude how much to move
 */
void gf3d_camera_fly_right(float magnitude);

/**
 * @brief move the camera "UP" relative to the view angles of the camera
 * @param magnitude how much to move
 */
void gf3d_camera_fly_up(float magnitude);

/**
 * @brief move the camera forward relative to the camera view angle
 * @note does not move along the z axis
 * @param magnitude how far to move
 */
void gf3d_camera_walk_forward(float magnitude);

/**
 * @brief move the camera right relative to the camera view angle
 * @note does not move along the z axis
 * @param magnitude how far to move
 */
void gf3d_camera_walk_right(float magnitude);

/**
 * @brief move the camera up along the z axis.
 * @note does not consider view angles.
 * @param magnitude how far to move
 */
void gf3d_camera_move_up(float magnitude);

/**
 * @brief rotate the camera's yaw
 * @param magnitude how far, in radians, to rotate
 */
void gf3d_camera_yaw(float magnitude);

/**
 * @brief rotate the camera's pitch
 * @param magnitude how far, in radians, to rotate
 */
void gf3d_camera_pitch(float magnitude);

/**
 * @brief rotate the camera's roll
 * @param magnitude how far, in radians, to rotate
 */
void gf3d_camera_roll(float magnitude);

/**
 * @brief get the current camera position in world space;
 * @return the camera position.
 */
Vector3D gf3d_camera_get_position();


/**
 * @brief explicitely set the camera positon, holding all other parameters the same
 * @param position the new position for the camera
 */
void gf3d_camera_set_position(Vector3D position);

/**
 * @brief explicitely set the camera scale (to be applied to the entire scene)
 * @param scale the new scale for the camera
 */
void gf3d_camera_set_scale(Vector3D scale);

/**
 * @brief explicitely set the camera positon, holding all other parameters the same
 * @param rotation the new rotation for the camera (pitch[x], roll[y], yaw[z])
 */
void gf3d_camera_set_rotation(Vector3D rotation);

/**
 * @brief get the rotation angles of the camera that can be used with vector3d_angle_vectors()
 * @return the angle, in radians of the camera
 */
Vector3D gf3d_camera_get_angles();

/**
 * @brief get the vectors for forward, right, and up based on the camera view angles
 * @param forward   [output] if provided, it will be set with the normal vector in the direction the camera is looking
 * @param right     [output] if provided, it will be set with the normal vector to the right of where the camera is looking
 * @param up        [output] if provided, it will be set with the normal vector directly up from where the camera is looking
 */
void gf3d_camera_get_view_vectors(Vector3D *forward, Vector3D *right, Vector3D *up);

#endif
