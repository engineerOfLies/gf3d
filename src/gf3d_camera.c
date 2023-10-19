#include <string.h>

#include "gfc_matrix.h"

#include "gf3d_camera.h"

static Camera gf3d_camera = {0};


void gf3d_camera_get_view_mat4(Matrix4 *view)
{
    if (!view)return;
    memcpy(view,gf3d_camera.cameraMat,sizeof(Matrix4));
}

void gf3d_camera_set_view_mat4(Matrix4 *view)
{
    if (!view)return;
    memcpy(gf3d_camera.cameraMat,view,sizeof(Matrix4));
}

void gf3d_camera_look_at(
    Vector3D position,
    Vector3D target,
    Vector3D up
)
{
    gfc_matrix_view(
        gf3d_camera.cameraMat,
        position,
        target,
        up
    );
}


void gf3d_camera_update_view()
{
    Vector3D eye, target = {0,1,0}, up = {0,0,1};
    Vector3D zaxis,xaxis,yaxis;
    
    Matrix4 orientation;
    Matrix4 translation;

    vector3d_copy(eye,gf3d_camera.position);
    vector3d_rotate_about_x(&target,-gf3d_camera.rotation.x);
    vector3d_rotate_about_z(&target,-gf3d_camera.rotation.z);
    vector3d_add(target,target,eye);
    
    
    // The "forward" vector.
    vector3d_sub(zaxis,eye,target);
    vector3d_normalize(&zaxis);
    
    // The "right" vector.
    vector3d_cross_product(&xaxis, up, zaxis);
    vector3d_normalize(&xaxis);
    
    // The "up" vector.
    vector3d_cross_product(&yaxis, zaxis, xaxis);
    vector3d_normalize(&yaxis);

    
    // Create a 4x4 orientation matrix from the right, up, and forward vectors
    // This is transposed which is equivalent to performing an inverse 
    // if the matrix is orthonormalized (in this case, it is).
    gfc_matrix_identity(orientation);
    orientation[0][0] = xaxis.x;
    orientation[0][1] = yaxis.x;
    orientation[0][2] = zaxis.x;
    orientation[0][3] = 0;
    orientation[1][0] = xaxis.y;
    orientation[1][1] = yaxis.y;
    orientation[1][2] = zaxis.y;
    orientation[1][3] = 0;
    orientation[2][0] = xaxis.z;
    orientation[2][1] = yaxis.z;
    orientation[2][2] = zaxis.z;
    orientation[2][3] = 0;
    
    // Create a 4x4 translation matrix.
    // The eye position is negated which is equivalent
    // to the inverse of the translation matrix. 
    // T(v)^-1 == T(-v)
    gfc_matrix_identity(translation);
    translation[3][0] = eye.x;
    translation[3][1] = eye.y;
    translation[3][2] = eye.z;

    // Combine the orientation and translation to compute 
    // the final view matrix. Note that the order of 
    // multiplication is reversed because the matrices
    // are already inverted.    
    gfc_matrix_multiply(
        gf3d_camera.cameraMat,
        translation,
        orientation);
    
    //gfc_matrix4_slog(gf3d_camera.cameraMat);

}

void gf3d_camera_update_view_old()
{
    /**
     * Adapted from tutorial:
     * https://www.3dgep.com/understanding-the-view-matrix/
     */
    Vector3D xaxis,yaxis,zaxis,position;
    float cosPitch = cos(gf3d_camera.rotation.x);
    float sinPitch = sin(gf3d_camera.rotation.x);
    float cosYaw = cos(gf3d_camera.rotation.z);
    float sinYaw = sin(gf3d_camera.rotation.z); 

    position.x = gf3d_camera.position.x;
    position.y = -gf3d_camera.position.z;        //inverting for Z-up
    position.z = gf3d_camera.position.y;
    gfc_matrix_identity(gf3d_camera.cameraMat);

    vector3d_set(xaxis, cosYaw,                     0,  -sinYaw);
    vector3d_set(yaxis, sinYaw * sinPitch,   cosPitch,   cosYaw * sinPitch);
    vector3d_set(zaxis, sinYaw * cosPitch,  -sinPitch,   cosPitch * cosYaw);
    
    gf3d_camera.cameraMat[0][0] = xaxis.x;
    gf3d_camera.cameraMat[0][1] = yaxis.x;
    gf3d_camera.cameraMat[0][2] = zaxis.x;

    gf3d_camera.cameraMat[1][0] = xaxis.z;
    gf3d_camera.cameraMat[1][1] = yaxis.z;
    gf3d_camera.cameraMat[1][2] = zaxis.z;

    gf3d_camera.cameraMat[2][0] = -xaxis.y;
    gf3d_camera.cameraMat[2][1] = -yaxis.y;
    gf3d_camera.cameraMat[2][2] = -zaxis.y;

    gf3d_camera.cameraMat[3][0] = vector3d_dot_product(xaxis, position);
    gf3d_camera.cameraMat[3][1] = vector3d_dot_product(yaxis, position);
    gf3d_camera.cameraMat[3][2] = vector3d_dot_product(zaxis, position);
    
}

void gf3d_camera_set_position(Vector3D position)
{
    gf3d_camera.position.x = -position.x;
    gf3d_camera.position.y = -position.y;
    gf3d_camera.position.z = -position.z;
}

void gf3d_camera_set_rotation(Vector3D rotation)
{
    gf3d_camera.rotation.x = -rotation.x;
    gf3d_camera.rotation.y = -rotation.y;
    gf3d_camera.rotation.z = -rotation.z;
}

void gf3d_camera_set_scale(Vector3D scale)
{
    if (!scale.x)gf3d_camera.scale.x = 0;
    else gf3d_camera.scale.x = 1/scale.x;
    if (!scale.y)gf3d_camera.scale.y = 0;
    else gf3d_camera.scale.y = 1/scale.y;
    if (!scale.z)gf3d_camera.scale.z = 0;
    else gf3d_camera.scale.z = 1/scale.z;
}

/*eol@eof*/
