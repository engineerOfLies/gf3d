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
#include "gf2d_draw.h"

#include "entity.h"
#include "agumon.h"
#include "player.h"
#include "world.h"
#include "weapon.h"
#include "resource.h"

//#include "player.c"

extern int __DEBUG;

int main(int argc,char *argv[])
{
    int done = 0;
    int a;
    TextBlock playerStatuses;
    TextBlock gatheredResources;
    
    Sprite *mouse = NULL;
    int mousex,mousey;
    //Uint32 then;
    float mouseFrame = 0;
    World *w;
    Entity *agu;
    Entity *player;
    Entity *collisionPartner;
    Entity *weapon;
    Entity *woodLog;
    Entity *cementBlock;
    Entity *metalBarrel;
    Entity *jerryCan;
    Entity *waterWell;
    //Entity *gun;
    //Particle particle[100];
    Matrix4 skyMat;
    Model *sky;

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
    gf2d_draw_manager_init(1000);
    
    slog_sync();
    
    entity_system_init(1024);
    
    mouse = gf2d_sprite_load("images/pointer.png",32,32, 16);
    

    agu = agumon_new(vector3d(0,0,50));
    if (agu)agu->selected = 1;
    w = world_load("config/testworld.json");
    
    SDL_SetRelativeMouseMode(SDL_TRUE);
    slog_sync();
    gf3d_camera_set_scale(vector3d(1,1,1));
    player = player_new(vector3d(-50,0,0));
    weapon = weapon_new();
    woodLog = resource_new(vector3d(100,0,0), "models/log.model", "log");
    cementBlock = resource_new(vector3d(125,0,0), "models/cement.model", "concrete");
    metalBarrel = resource_new(vector3d(150,0,0), "models/metal.model", "metal");
    metalBarrel->scale = vector3d(1,1,1);
    jerryCan = resource_new(vector3d(175,0,0), "models/fuel.model", "fuel");
    jerryCan->scale = vector3d(5,5,5);
    waterWell = resource_new(vector3d(200,0,0), "models/water.model", "water");
    waterWell->scale = vector3d(1,1,1);

    //test_string = sj_string_new_text("test: ", 0);


    /*
    for (a = 0; a < 100; a++)
    {
        particle[a].position = vector3d(gfc_crandom() * 100,gfc_crandom() * 100,gfc_crandom() * 100);
        particle[a].color = gfc_color(0,0,0,1);
        particle[a].color = gfc_color(gfc_random(),gfc_random(),gfc_random(),1);
        particle[a].size = 100 * gfc_random();
    }
    a = 0;
    */
    sky = gf3d_model_load("models/sky.model");
    gfc_matrix_identity(skyMat);
    gfc_matrix_scale(skyMat,vector3d(100,100,100));
    
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

        //rlStatuses workPlease = *((rlStatuses*)player->customData);

        //slog("calefaction check: %f", workPlease.calefaction);

        gfc_block_sprintf(playerStatuses
        ,"Calefaction: %f | Hydration: %i | Saturation: %i | Sanityation: %i | Defication: %i"
        ,player->calefaction
        ,player->hydration
        ,player->saturation
        ,player->sanityation
        ,player->defication);

        gfc_block_sprintf(gatheredResources
        ,"Wood: %i | Concrete: %i | Metal: %i | Fuel: %i | Water: %i"
        ,player->wood
        ,player->concrete
        ,player->metal
        ,player->fuel
        ,player->water);


        collisionPartner = entity_get_collision_partner(player);
        if (collisionPartner != NULL) {
            if(collisionPartner->isEnemy == 1){
                SDL_Delay(1);
                player->sanityation -= 1;
            }else if(collisionPartner->isResource == 1){
                if(gfc_stricmp(collisionPartner->entityName, "log") == 0){
                    player->wood++;
                    entity_free(woodLog);
                }else if(gfc_stricmp(collisionPartner->entityName, "concrete") == 0){
                    player->concrete++;
                    entity_free(cementBlock);
                }else if(gfc_stricmp(collisionPartner->entityName, "metal") == 0){
                    player->metal++;
                    entity_free(metalBarrel);
                }else if(gfc_stricmp(collisionPartner->entityName, "fuel") == 0){
                    player->fuel += 10;
                    entity_free(jerryCan);
                }else if(gfc_stricmp(collisionPartner->entityName, "water") == 0){
                    player->water += 25;
                    entity_free(waterWell);
                }
            }
        } else {
            if(player->sanityation < 100)
            {
                player->sanityation +=1;
                if(player->sanityation == 100)continue;
            }
        }




        gf3d_vgraphics_render_start();

            //3D draws
                gf3d_model_draw_sky(sky,skyMat,gfc_color(1,1,1,1));
                world_draw(w);
                entity_draw_all();
                /*
                for (a = 0; a < 100; a++)
                {
                    gf3d_particle_draw(&particle[a]);
                }
                */
            //2D draws
                //gf2d_draw_rect_filled(gfc_rect(10 ,10,1000,32),gfc_color8(128,128,128,255));
                gf2d_font_draw_line_tag(playerStatuses,FT_H1,gfc_color(1,0,1,1), vector2d(10,10));
                gf2d_font_draw_line_tag(gatheredResources,FT_Normal,gfc_color(0,1,0,1), vector2d(10,30));

                //gf2d_font_draw_line_tag(hydrationValue,FT_H1,gfc_color(1,1,1,1), vector2d(10,30));
                
                //gf2d_draw_rect(gfc_rect(10 ,10,1000,32),gfc_color8(255,255,255,255));
                
                gf2d_sprite_draw(mouse,vector2d(mousex,mousey),vector2d(2,2),vector3d(8,8,0),gfc_color(0.3,.9,1,0.9),(Uint32)mouseFrame);
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
