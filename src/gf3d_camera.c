#include <string.h>

#include "gfc_matrix.h"

#include "gf3d_camera.h"

static Camera gf3d_camera = {0};

void gf3d_camera_get_view_mat4(GFC_Matrix4 *view)
{
    if (!view)return;
    memcpy(view,gf3d_camera.cameraMat,sizeof(GFC_Matrix4));
}

void gf3d_camera_set_view_mat4(GFC_Matrix4 *view)
{
    if (!view)return;
    memcpy(gf3d_camera.cameraMat,view,sizeof(GFC_Matrix4));
}

void gf3d_camera_look_at(GFC_Vector3D target,const GFC_Vector3D *position)
{
    GFC_Vector3D angles,pos;
    GFC_Vector3D delta;
    if (position)
    {
        gfc_vector3d_copy(pos,(*position));
        gf3d_camera_set_position(pos);
    }
    else
    {
        pos = gf3d_camera_get_position();
    }
    gfc_vector3d_sub(delta,target,pos);
    gfc_vector3d_angles (delta, &angles);
    angles.z -= GFC_HALF_PI;
    angles.x -= GFC_PI;
    gf3d_camera_set_rotation(angles);
}

void gf3d_camera_update_view()
{
    /**
     * Adapted from tutorial:
     * https://www.3dgep.com/understanding-the-view-matrix/
     */
    
    GFC_Vector3D xaxis,yaxis,zaxis,position;
    float cosPitch = cos(gf3d_camera.rotation.x);
    float sinPitch = sin(gf3d_camera.rotation.x);
    float cosYaw = cos(gf3d_camera.rotation.z);
    float sinYaw = sin(gf3d_camera.rotation.z); 

    position.x = gf3d_camera.position.x;
    position.y = -gf3d_camera.position.z;        //inverting for Z-up
    position.z = gf3d_camera.position.y;
    gfc_matrix4_identity(gf3d_camera.cameraMat);

    gfc_vector3d_set(xaxis, cosYaw,                     0,  -sinYaw);
    gfc_vector3d_set(yaxis, sinYaw * sinPitch,   cosPitch,   cosYaw * sinPitch);
    gfc_vector3d_set(zaxis, sinYaw * cosPitch,  -sinPitch,   cosPitch * cosYaw);
    
    gf3d_camera.cameraMat[0][0] = xaxis.x;
    gf3d_camera.cameraMat[0][1] = yaxis.x;
    gf3d_camera.cameraMat[0][2] = zaxis.x;

    gf3d_camera.cameraMat[1][0] = xaxis.z;
    gf3d_camera.cameraMat[1][1] = yaxis.z;
    gf3d_camera.cameraMat[1][2] = zaxis.z;

    gf3d_camera.cameraMat[2][0] = -xaxis.y;
    gf3d_camera.cameraMat[2][1] = -yaxis.y;
    gf3d_camera.cameraMat[2][2] = -zaxis.y;

    gf3d_camera.cameraMat[3][0] = gfc_vector3d_dot_product(xaxis, position);
    gf3d_camera.cameraMat[3][1] = gfc_vector3d_dot_product(yaxis, position);
    gf3d_camera.cameraMat[3][2] = gfc_vector3d_dot_product(zaxis, position);
}

GFC_Vector3D gf3d_camera_get_position()
{
    GFC_Vector3D position;
    gfc_vector3d_negate(position,gf3d_camera.position);
    return position;
}

void gf3d_camera_set_position(GFC_Vector3D position)
{
    gf3d_camera.position.x = -position.x;
    gf3d_camera.position.y = -position.y;
    gf3d_camera.position.z = -position.z;
}

void gf3d_camera_move(GFC_Vector3D translation)
{
    gfc_vector3d_sub(gf3d_camera.position,gf3d_camera.position,translation);
}

void gf3d_camera_walk_forward(float magnitude)
{
    GFC_Vector2D w;
    GFC_Vector3D forward = {0};
    w = gfc_vector2d_from_angle(-gf3d_camera.rotation.z);
    forward.x = w.x;
    forward.y = w.y;
    gfc_vector3d_set_magnitude(&forward,magnitude);
    gf3d_camera_move(forward);

}

void gf3d_camera_walk_right(float magnitude)
{
    GFC_Vector2D w;
    GFC_Vector3D right = {0};
    w = gfc_vector2d_from_angle(-gf3d_camera.rotation.z - GFC_HALF_PI);
    right.x = w.x;
    right.y = w.y;
    gfc_vector3d_set_magnitude(&right,magnitude);
    gf3d_camera_move(right);

}

void gf3d_camera_move_up(float magnitude)
{
    GFC_Vector3D up = {0,0,magnitude};
    gf3d_camera_move(up);
}

void gf3d_camera_yaw(float magnitude)
{
    gf3d_camera.rotation.z -= magnitude;
}

void gf3d_camera_pitch(float magnitude)
{
    gf3d_camera.rotation.x -= magnitude;
    if (gf3d_camera.rotation.x >= -GFC_HALF_PI)
    {
        gf3d_camera.rotation.x = -GFC_HALF_PI - GFC_EPSILON;
    }
    if (gf3d_camera.rotation.x  <= -GFC_PI_HALFPI)
    {
        gf3d_camera.rotation.x = -GFC_PI_HALFPI + GFC_EPSILON;
    }
}

void gf3d_camera_roll(float magnitude)
{
    gf3d_camera.rotation.y -= magnitude;
}


void gf3d_camera_fly_forward(float magnitude)
{
    GFC_Vector3D forward;
    gf3d_camera_get_view_vectors(&forward, NULL, NULL);
    gfc_vector3d_set_magnitude(&forward,magnitude);
    gf3d_camera_move(forward);
}

void gf3d_camera_fly_right(float magnitude)
{
    GFC_Vector3D right;
    gf3d_camera_get_view_vectors(NULL, &right, NULL);
    gfc_vector3d_set_magnitude(&right,magnitude);
    gf3d_camera_move(right);
}

void gf3d_camera_fly_up(float magnitude)
{
    GFC_Vector3D up;
    gf3d_camera_get_view_vectors(NULL, NULL, &up);
    gfc_vector3d_set_magnitude(&up,magnitude);
    gf3d_camera_move(up);
}

void gf3d_camera_get_view_vectors(GFC_Vector3D *forward, GFC_Vector3D *right, GFC_Vector3D *up)
{
    gfc_vector3d_angle_vectors(gf3d_camera_get_angles(), forward, right, up);
}

GFC_Vector3D gf3d_camera_get_angles()
{
    return gfc_vector3d(-gf3d_camera.rotation.x,-gf3d_camera.rotation.y,-gf3d_camera.rotation.z);
}

void gf3d_camera_set_rotation(GFC_Vector3D rotation)
{
    gfc_angle_clamp_radians(&rotation.x);
    gf3d_camera.rotation.x = -rotation.x;
    gf3d_camera.rotation.y = -rotation.y;
    gf3d_camera.rotation.z = -rotation.z;
}

void gf3d_camera_set_scale(GFC_Vector3D scale)
{
    if (!scale.x)gf3d_camera.scale.x = 0;
    else gf3d_camera.scale.x = 1/scale.x;
    if (!scale.y)gf3d_camera.scale.y = 0;
    else gf3d_camera.scale.y = 1/scale.y;
    if (!scale.z)gf3d_camera.scale.z = 0;
    else gf3d_camera.scale.z = 1/scale.z;
}

/*eol@eof*/
