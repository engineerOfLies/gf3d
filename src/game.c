#include <SDL.h>            

#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_mesh.h"
#include "gf3d_matrix.h"
#include "gf3d_camera.h"
#include "gf3d_vector.h"
#include "gf3d_texture.h"

int main(int argc,char *argv[])
{
    int done = 0;
    const Uint8 * keys;
//    Texture *texture;
    Mesh *testMesh = NULL;
    Mesh *mesh = NULL;
    Uint32 bufferFrame =0;
    VkCommandBuffer commandBuffer;

    static Vertex vertices[] = 
    {
        {
            {-1,-1,0},
            {1,0,0}
        },
        {
            {1,-1,0},
            {0,1,0}
            
        },
        {
            {1,1,0},
            {0,0,1}
        },
        {
            {-1,1,0},
            {0.5,0,0.5}
        }
    };
    
    static Face faces[] = 
    {
        {{0,1,2}},
        {{2,3,0}}
    };

    
    init_logger("gf3d.log");
    
    
    slog("gf3d begin");
    gf3d_vgraphics_init(
        "gf3d",                 //program name
        1200,                   //screen width
        700,                    //screen height
        vector4d(0.51,0.75,1,1),//background color
        0,                      //fullscreen
        1                       //validation
    );
    testMesh = gf3d_mesh_create_vertex_buffer_from_vertices(vertices,4,faces,2);
    gf3d_word_cpy(testMesh->filename,"testMesh");
    
    vertices[0].vertex.z = 1;
    vertices[1].vertex.z = 1;
    vertices[2].vertex.z = 1;
    vertices[3].vertex.z = 1;

    vertices[0].normal.x = 1;
    vertices[1].normal.x = 1;
    vertices[2].normal.y = 1;
    vertices[3].normal.y = 1;
    
    mesh = gf3d_mesh_create_vertex_buffer_from_vertices(vertices,4,faces,2);
    gf3d_word_cpy(mesh->filename,"mesh");
//    texture = gf3d_texture_load("images/bg_flat.png");

    // main game loop
    slog("gf3d main loop begin");
    
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        //update game things here
        
        gf3d_vgraphics_rotate_camera(0.001);
        
        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        commandBuffer = gf3d_command_rendering_begin(bufferFrame);
        // TODO loop through desired meshes to render
                gf3d_mesh_render(testMesh,gf3d_vgraphics_get_graphics_command_pool(),gf3d_vgraphics_get_graphics_pipeline(),bufferFrame,commandBuffer);
                gf3d_mesh_render(mesh,gf3d_vgraphics_get_graphics_command_pool(),gf3d_vgraphics_get_graphics_pipeline(),bufferFrame,commandBuffer);
                
        gf3d_command_rendering_end(commandBuffer);
        
        bufferFrame = gf3d_vgraphics_render();
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
    }    
    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
