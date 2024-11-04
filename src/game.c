#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_draw.h"

#include "entity.h"
#include "player.h"
#include "ent_obj.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}

void draw_origin()
{
    // x axes
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(-100,0,0),gfc_vector3d(100,0,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(1,0,0,1));
    // y axes
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,-100,0),gfc_vector3d(0,100,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,1,0,1));
    // z axes
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,0,-100),gfc_vector3d(0,0,100)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,0,1,1));
}


int main(int argc,char *argv[])
{
    //local variables
    Model *sky,*dino, *cylinder;
    GFC_Matrix4 skyMat,dinoMat;
    Mix_Music *music;
    int menu = 1;
    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    gfc_audio_init(16,8,0,1,1,0);
    entity_system_init(32);                 // Entity limit 32
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf3d_materials_init();
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    gf3d_draw_init();//3D
    gf2d_draw_manager_init(1000);//2D

    //game init
    srand(SDL_GetTicks());
    slog_sync();

    //game setup
    gf2d_mouse_load("actors/mouse.actor");
    //dog = gf3d_model_load_full();
    sky = gf3d_model_load("models/sky.model");
    gfc_matrix4_identity(skyMat);
    dino = gf3d_model_load("models/dino.model");
    gfc_matrix4_identity(dinoMat);
    cylinder = gf3d_model_load("models/primitives/icylinder.model");
    gfc_matrix4_identity(cylinder);
    music = gfc_sound_load_music("music/Persona 3 Reload - It's Going Down Now (Extended Version).mp3");

    // Player
    GFC_Vector3D playerSpawn = gfc_vector3d(0, 0, 0);
    Entity* player = player_new("player", dino, playerSpawn);
    

    // Object
    GFC_Vector3D spawn = gfc_vector3d(0, -20, 0);
    Entity* cylind = obj_new("cylinder", cylinder, spawn);

    float offset = 45.0f;
    entity_set_radius(player, &offset);
    
    //camera
    gf3d_camera_set_scale(gfc_vector3d(1,1,1));
    SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    GFC_Vector3D offsetPos = gfc_vector3d(0.0f, offset, 20.0f);
    GFC_Vector3D offsetLook = gfc_vector3d(0.0f, 0.0f, 10.0f);
    gf3d_camera_set_offset_position(player->position, offsetPos);
    gf3d_camera_set_offset_rotation(player->position, offsetLook);
    
    //GFC_Box testBox = gfc_box(0, -10, 0, 5, 5, 5);

    //gf3d_camera_look_at(gfc_vector3d(player->position.x, player->position.y, player->position.z + 10), NULL);
    //gf3d_camera_set_move_step(1.0);
    //gf3d_camera_set_rotate_step(0.1);
    
    //gf3d_camera_enable_free_look(1);
    gf3d_camera_enable_player_look(1);
    //gf3d_camera_set_auto_pan(1);
    entity_set_camera(player, gf3d_camera_get_mode());
    //windows

    // Music
    gfc_music_play(music, -1);      // Play Music

    // main game loop    
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();

        //camera updates
        GFC_Vector2D mousePos = gf2d_mouse_get_movement();

        entity_system_think();
        entity_system_collision();
        entity_system_update();

        gf3d_camera_move_mouse(mousePos, player->position, offset);
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());

        gf3d_vgraphics_render_start();

            //3D draws
        
                gf3d_model_draw_sky(sky,skyMat,GFC_COLOR_WHITE);

                //gf3d_model_draw();
                
                
                entity_system_draw();
                entity_system_collision_visible(1);
                
                
                
                //gf3d_model_draw();

                /*gf3d_model_draw(
                    dino,
                    dinoMat,
                    GFC_COLOR_WHITE,
                    0);*/
                draw_origin();
            //2D draws
                //gf2d_mouse_draw();
                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));

                if (menu == -1) {
                    gf2d_draw_menu();
                   // gf2d_font_draw_text_wrap_tag("This text is in a box!", FT_Large, GFC_COLOR_WHITE, gfc_rect(100, 100, 200, 200));
                    //gf2d_font_draw_line_tag("Menu Opened", FT_Large, GFC_COLOR_RED, gfc_vector2d(100, 100));
                    //gf2d_font_draw_line_tag("This is a line", FT_Large, GFC_COLOR_WHITE, gfc_vector2d(100, 150));
                    //gf2d_draw_shape(gfc_shape_rect(0, 0, 200,200), GFC_COLOR_BLUE, gfc_vector2d(0, 0));
                    //gf2d_draw_rect_filled(gfc_rect(0,0,200,200), GFC_COLOR_BLUE);
                }

        gf3d_vgraphics_render_end();

        entity_set_camera(player,gf3d_camera_get_mode());
        //gf3d_camera_third_person(mousePos, player->position);
        //gf3d_camera_update_view();

        // Open menu when pressing ESCAPE
        if (gfc_input_command_pressed("cancel")) menu = -menu;

        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay();
    }    
    
    entity_system_close();
    gfc_music_free(music);
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
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
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}
/*eol@eof*/
