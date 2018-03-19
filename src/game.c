#include <SDL.h>            
#include <GL/glew.h>

#include "simple_logger.h"
#include "gf3d_graphics.h"
#include "gf3d_model.h"
#include "gf3d_matrix.h"
#include "gf3d_camera.h"

int main(int argc,char *argv[])
{
    int done = 0;
    const Uint8 * keys;
    GLuint program = 0;
    Model *testmodel;
    Matrix4 model;
    
    init_logger("gf2d.log");
    slog("gf3d begin");
    
    gf3d_graphics_initialize(
        "gametest3D",
        1200,
        700,
        1200,
        700,
        vector4d(135.0/255,196.0/255,245.0/255,1),  // background color
        0,
        3,1 // opengl major,minor 
    );

    gf3d_model_manager_init(1024);

    testmodel = gf3d_model_load_from_json_file("models/cube.json");
//    testmodel = gf3d_model_new_triangle();

    gf3d_camera_look_at(
        vector3d(4,3,3),
        vector3d(0,0,0),
        vector3d(0,1,0)
    );
    gf3d_matrix_identity(model);
    
    program = gf3d_graphics_get_shader_program_id();
    // main game loop
    while(!done)
    {
        gf3d_graphics_clear_screen();
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        
//        slog("using shader program %i",program);
        gf3d_model_render(testmodel,model,program);
        
        gf3d_grahics_next_frame();
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
    }    
    
    //cleanup
    gf3d_model_free(testmodel);
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
