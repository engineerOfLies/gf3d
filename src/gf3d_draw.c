#include "simple_logger.h"

#include "gf3d_model.h"

#include "gf3d_draw.h"

typedef struct
{
    Model *cube;
    Model *icube;
    Model *sphere;
    Model *isphere;
    Model *icylinder;
}GF3D_DrawManager;

static GF3D_DrawManager gf3d_draw_manager = {0};

void gf3d_draw_close()
{
    gf3d_model_free(gf3d_draw_manager.cube);
    gf3d_model_free(gf3d_draw_manager.icube);
    gf3d_model_free(gf3d_draw_manager.sphere);
    gf3d_model_free(gf3d_draw_manager.isphere);
    gf3d_model_free(gf3d_draw_manager.icylinder);
}

void gf3d_draw_init()
{
    gf3d_draw_manager.cube = gf3d_model_load_full("models/primitives/cube.obj","models/primitives/flatwhite.png");
    gf3d_draw_manager.icube = gf3d_model_load_full("models/primitives/icube.obj","models/primitives/flatwhite.png");
    gf3d_draw_manager.sphere = gf3d_model_load_full("models/primitives/sphere.obj","models/primitives/flatwhite.png");
    gf3d_draw_manager.isphere = gf3d_model_load_full("models/primitives/isphere.obj","models/primitives/flatwhite.png");
    gf3d_draw_manager.icylinder= gf3d_model_load_full("models/primitives/icylinder.obj","models/primitives/flatwhite.png");
    
    atexit(gf3d_draw_close);
}

void gf3d_draw_edge_3d(GFC_Edge3D edge,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,float radius,GFC_Color color)
{
    Mesh *mesh;
    GFC_Matrix4 modelMat;
    GFC_Vector3D v,angles;
    float d;
    
    if (!radius)return;// zero radius doesn't draw anyway
    
    if (!gf3d_draw_manager.icylinder)return;
    //calculate rotation of the gfc_vector from a to b
    gfc_vector3d_sub(v,edge.b,edge.a);// gfc_vector from a to b
    gfc_vector3d_angles (v, &angles);//rotation from a to b
    //angles.y -= GFC_HALF_PI;
    
    //z scale based on gfc_vector length
    d = gfc_vector3d_magnitude(v);
    if (!d)return;// can't draw a zero length edge
    mesh = gfc_list_get_nth(gf3d_draw_manager.icylinder->mesh_list,0);
    scale.x *= d /mesh->bounds.w;
    // y and z scale based on radius
    scale.y *= radius / mesh->bounds.h;
    scale.z *= radius / mesh->bounds.d;

    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + edge.a.x,position.y + edge.a.y,position.z + edge.a.z),
        gfc_vector3d(rotation.x + angles.x,rotation.y + angles.y,rotation.z + angles.z),
        scale);
    gf3d_model_draw(gf3d_draw_manager.icylinder,modelMat,color,NULL,0);
}

void gf3d_draw_cube_wireframe(GFC_Box cube,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color)
{
    GFC_Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + cube.x,position.y + cube.y,position.z + cube.z),
        rotation,
        gfc_vector3d(scale.x * cube.w,scale.y * cube.h,scale.z * cube.d));
    gf3d_model_draw_highlight(gf3d_draw_manager.icube,0,modelMat,color);
}

void gf3d_draw_box_wireframe(GFC_Box cube,GFC_Color color)
{
    GFC_Vector3D center;
    GFC_Matrix4 modelMat;
    
    center.x = cube.x + (cube.w * 0.5);
    center.y = cube.y + (cube.h * 0.5);
    center.z = cube.z + (cube.d * 0.5);

    gfc_matrix4_from_vectors(
        modelMat,
        center,
        gfc_vector3d(0,0,0),
        gfc_vector3d(cube.w,cube.h,cube.d));
    gf3d_model_draw_highlight(gf3d_draw_manager.icube,0,modelMat,color);
}


void gf3d_draw_cube_solid(GFC_Box cube,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color)
{
    GFC_Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + cube.x,position.y + cube.y,position.z + cube.z),
        rotation,
        gfc_vector3d(scale.x * cube.w,scale.y * cube.h,scale.z * cube.d));
    gf3d_model_draw(gf3d_draw_manager.cube,modelMat,color,NULL,0);
}


void gf3d_draw_sphere_wireframe(GFC_Sphere sphere,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color)
{
    GFC_Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + sphere.x,position.y + sphere.y,position.z + sphere.z),
        rotation,
        gfc_vector3d(scale.x * sphere.r,scale.y * sphere.r,scale.z * sphere.r));
    gf3d_model_draw_highlight(gf3d_draw_manager.isphere,0,modelMat,color);
}

void gf3d_draw_sphere_solid(GFC_Sphere sphere,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color,GFC_Color ambient)
{
    GFC_Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + sphere.x,position.y + sphere.y,position.z + sphere.z),
        rotation,
        gfc_vector3d(scale.x * sphere.r,scale.y * sphere.r,scale.z * sphere.r));
    gf3d_model_draw(gf3d_draw_manager.sphere,modelMat,color,NULL,0);
}


void gf3d_draw_circle(GFC_Circle circle,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color)
{
    GFC_Matrix4 modelMat;
    Mesh *mesh;
    
    mesh  = gfc_list_get_nth(gf3d_draw_manager.sphere->mesh_list,0);
    
    circle.r /= mesh->bounds.w;
    
    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + circle.x,position.y + circle.y,position.z),
        rotation,
        gfc_vector3d(scale.x * circle.r,scale.y * circle.r,scale.z * circle.r));
    gf3d_model_draw(gf3d_draw_manager.sphere,modelMat,GFC_COLOR_BLACK,NULL,0);
    gf3d_model_draw_highlight(gf3d_draw_manager.sphere,0,modelMat,color);
}



/*eol@eof*/
