#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_pak.h"
#include "gfc_audio.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"
#include "gf2d_windows.h"
#include "gf2d_windows_common.h"
#include "gf2d_message_buffer.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_particle.h"
#include "gf3d_draw.h"
#include "gf3d_lights.h"
#include "gf3d_entity.h"

#include "gf3d_camera_entity.h"

extern int __DEBUG;

int freeBuildMode = 0;

static int _done = 0;
static Window *_quit = NULL;

void parse_arguments(int argc,char *argv[]);

void onCancel(void *data)
{
    _quit = NULL;
}

void onExit(void *data)
{
    _done = 1;
    _quit = NULL;
}

void exitGame()
{
    _done = 1;
}

void exitCheck()
{
    if (_quit)return;
    _quit = window_yes_no("Exit?",onExit,onCancel,NULL);
}

void draw_origin()
{
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(vector3d(-100,0,0),vector3d(100,0,0)),
        vector3d(0,0,0),vector3d(0,0,0),vector3d(1,1,1),3,gfc_color(1,0,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(vector3d(0,-100,0),vector3d(0,100,0)),
        vector3d(0,0,0),vector3d(0,0,0),vector3d(1,1,1),3,gfc_color(0,1,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(vector3d(0,0,-100),vector3d(0,0,100)),
        vector3d(0,0,0),vector3d(0,0,0),vector3d(1,1,1),3,gfc_color(0,0,1,1));
}


int main(int argc,char *argv[])
{
    Model      *sky;
    Matrix4     skyMat;
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    gfc_pak_manager_init();
    gfc_input_init("config/input.cfg");
    slog("gf3d begin");
    gf3d_vgraphics_init("config/setup.cfg");
    gfc_audio_init(256,16,4,1,1,1);
    gf2d_font_init("config/font.cfg");
    gf3d_draw_init();//3D
    gf2d_draw_manager_init(1000);//2D
    gf2d_actor_init(1024);
    gf2d_mouse_load("actors/mouse.actor");
    gf3d_entity_system_init(1024);
    gf2d_windows_init(128,"config/windows.cfg");
    
    slog_sync();
        
//    SDL_SetRelativeMouseMode(SDL_TRUE);
    slog_sync();
    gf3d_camera_set_scale(vector3d(1,1,1));
    //load game definitions

    //game setup
    sky = gf3d_model_load("models/sky.model");
    gfc_matrix_identity(skyMat);
    gfc_matrix_scale(skyMat,vector3d(100,100,100));

    gf3d_camera_entity_new(vector3d(-100,0,0),vector3d(0,0,0));
    gf3d_camera_entity_set_look_mode(CTT_Position);
    gf3d_camera_entity_enable_free_look(1);

    window_message_buffer(5, 1000, GFC_COLOR_GREEN);
    message_new("ALT+F4 to exit");
    
    // main game loop
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();
        gf2d_windows_update_all();
        gf3d_entity_think_all();
        gf3d_entity_update_all();
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());

        gf3d_vgraphics_render_start();

            //3D draws
                gf3d_model_draw_sky(sky,skyMat,gfc_color(1,1,1,1));
                draw_origin();
                gf3d_entity_draw_all();
                //2D draws
                gf2d_windows_draw_all();
                gf2d_mouse_draw();

        gf3d_vgraphics_render_end();

        if ((gfc_input_command_down("exit"))&&(_quit == NULL))
        {
            exitCheck();
        }
    }    
        
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
        if (strcmp(argv[a],"--freeBuildMode") == 0)
        {
            freeBuildMode = 1;
        }
    }    
}

/*eol@eof*/
