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
    gf3d_draw_manager.icylinder= gf3d_model_load("models/primitives/icylinder.model");
    
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
    gf3d_model_draw(gf3d_draw_manager.icylinder,modelMat,color,0);
}

void gf3d_draw_cube_solid(GFC_Box cube,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color)
{
    GFC_Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + cube.x,position.y + cube.y,position.z + cube.z),
        rotation,
        gfc_vector3d(scale.x * cube.w,scale.y * cube.h,scale.z * cube.d));
    gf3d_model_draw(gf3d_draw_manager.cube,modelMat,color,0);
}

void gf3d_draw_sphere_solid(GFC_Sphere sphere,GFC_Vector3D position,GFC_Vector3D rotation,GFC_Vector3D scale,GFC_Color color,GFC_Color ambient)
{
    GFC_Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        gfc_vector3d(position.x + sphere.x,position.y + sphere.y,position.z + sphere.z),
        rotation,
        gfc_vector3d(scale.x * sphere.r,scale.y * sphere.r,scale.z * sphere.r));
    gf3d_model_draw(gf3d_draw_manager.sphere,modelMat,color,0);
}


/*eol@eof*/
