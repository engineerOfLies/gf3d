#include "simple_logger.h"

#include "gf3d_model.h"

#include "gf3d_draw.h"

typedef struct
{
    Model *cube;
    Model *icube;
    Model *sphere;
    Model *isphere;
    Model *edge;
}GF3D_DrawManager;

static GF3D_DrawManager gf3d_draw_manager = {0};

void gf3d_draw_close()
{
    gf3d_model_free(gf3d_draw_manager.cube);
    gf3d_model_free(gf3d_draw_manager.icube);
    gf3d_model_free(gf3d_draw_manager.sphere);
    gf3d_model_free(gf3d_draw_manager.isphere);
}

void gf3d_draw_init()
{
    gf3d_draw_manager.cube = gf3d_model_load_full("models/primitives/cube.obj","models/primitives/flatwhite.png");
    gf3d_draw_manager.icube = gf3d_model_load_full("models/primitives/icube.obj","models/primitives/flatwhite.png");
    gf3d_draw_manager.sphere = gf3d_model_load_full("models/primitives/sphere.obj","models/primitives/flatwhite.png");
    gf3d_draw_manager.isphere = gf3d_model_load_full("models/primitives/isphere.obj","models/primitives/flatwhite.png");
    
    atexit(gf3d_draw_close);
}

void gf3d_draw_cube_wireframe(Box cube,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + cube.x,position.y + cube.y,position.z + cube.z),
        rotation,
        vector3d(scale.x * cube.w,scale.y * cube.h,scale.z * cube.d));
    gf3d_model_draw_highlight(gf3d_draw_manager.icube,modelMat,gfc_color_to_vector4f(color));
}

void gf3d_draw_cube_solid(Box cube,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + cube.x,position.y + cube.y,position.z + cube.z),
        rotation,
        vector3d(scale.x * cube.w,scale.y * cube.h,scale.z * cube.d));
    gf3d_model_draw(gf3d_draw_manager.cube,modelMat,gfc_color_to_vector4f(color),vector4d(1,1,1,1));
}


void gf3d_draw_sphere_wireframe(Sphere sphere,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + sphere.x,position.y + sphere.y,position.z + sphere.z),
        rotation,
        vector3d(scale.x * sphere.r,scale.y * sphere.r,scale.z * sphere.r));
    gf3d_model_draw_highlight(gf3d_draw_manager.isphere,modelMat,gfc_color_to_vector4f(color));
}

void gf3d_draw_sphere_solid(Sphere sphere,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + sphere.x,position.y + sphere.y,position.z + sphere.z),
        rotation,
        vector3d(scale.x * sphere.r,scale.y * sphere.r,scale.z * sphere.r));
    gf3d_model_draw(gf3d_draw_manager.sphere,modelMat,gfc_color_to_vector4f(color),vector4d(1,1,1,1));
}


void gf3d_draw_circle(Circle circle,Vector3D position,Vector3D rotation,Vector3D scale,Color color)
{
    Matrix4 modelMat;
    
    gfc_matrix4_from_vectors(
        modelMat,
        vector3d(position.x + circle.x,position.y + circle.y,position.z),
        rotation,
        vector3d(scale.x * circle.r,scale.y * circle.r,scale.z * circle.r));
    gf3d_model_draw(gf3d_draw_manager.sphere,modelMat,vector4d(0,0,0,0),vector4d(0,0,0,0));
    gf3d_model_draw_highlight(gf3d_draw_manager.sphere,modelMat,gfc_color_to_vector4f(color));
}



/*eol@eof*/
