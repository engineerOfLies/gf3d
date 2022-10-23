#include <SDL.h>            

#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_particle.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"

#include "entity.h"
#include "agumon.h"
#include "player.h"
#include "world.h"

extern int __DEBUG;

int main(int argc,char *argv[])
{
    int done = 0;
    int a;
    
    Sprite *mouse = NULL;
    int mousex,mousey;
    //Uint32 then;
    float mouseFrame = 0;
    World *w;
    Entity *agu;
    Particle particle[100];

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }
    
    init_logger("gf3d.log",0);    
    gfc_input_init("config/input.cfg");
    slog("gf3d begin");
    gf3d_vgraphics_init("config/setup.cfg");
    gf2d_font_init("config/font.cfg");

    slog_sync();
    
    entity_system_init(1024);
    
    mouse = gf2d_sprite_load("images/pointer.png",32,32, 16);
    
    
    agu = agumon_new(vector3d(0 ,0,0));
    if (agu)agu->selected = 1;
    w = world_load("config/testworld.json");
    
    SDL_SetRelativeMouseMode(SDL_TRUE);
    slog_sync();
    gf3d_camera_set_scale(vector3d(1,1,1));
    player_new(vector3d(-50,0,0));
    
    for (a = 0; a < 100; a++)
    {
        particle[a].position = vector3d(gfc_crandom() * 100,gfc_crandom() * 100,gfc_crandom() * 100);
        particle[a].color = gfc_color(gfc_random(),gfc_random(),gfc_random(),1);
        particle[a].size = 100 * gfc_random();
    }
    a = 0;
    // main game loop
    slog("gf3d main loop begin");
    while(!done)
    {
        gfc_input_update();
        gf2d_font_update();
        SDL_GetMouseState(&mousex,&mousey);
        
        mouseFrame += 0.01;
        if (mouseFrame >= 16)mouseFrame = 0;
        world_run_updates(w);
        entity_think_all();
        entity_update_all();
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());

        gf3d_vgraphics_render_start();

            //3D draws
                world_draw(w);
                entity_draw_all();
                
                for (a = 0; a < 100; a++)
                {
                    gf3d_particle_draw(&particle[a]);
                }
            //2D draws
                gf2d_font_draw_line_tag("Press ALT+F4 to exit",FT_H1,gfc_color(1,1,1,1), vector2d(10,10));
                gf2d_sprite_draw(mouse,vector2d(mousex,mousey),vector2d(1,1),vector3d(0,0,0),gfc_color(0.3,.9,1,1),(Uint32)mouseFrame);
        gf3d_vgraphics_render_end();

        if (gfc_input_command_down("exit"))done = 1; // exit condition
    }    
    
    world_delete(w);
    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
