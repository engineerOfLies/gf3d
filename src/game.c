#include <SDL.h>            
#include <GL/glew.h>

#include "simple_logger.h"
#include "gf3d_vgraphics.h"
#include "gf3d_model.h"
#include "gf3d_matrix.h"
#include "gf3d_camera.h"

int main(int argc,char *argv[])
{
    int done = 0;
    const Uint8 * keys;
    
    init_logger("gf2d.log");
    slog("gf3d begin");
    gf3d_vgraphics_init(
        "gf3d",
        1200,
        700,
        vector4d(0.51,0.75,1,1),
        0
    );
    
    // main game loop
    while(!done)
    {
        gf3d_vgraphics_clear();
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        
        
        gf3d_vgraphics_render();
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
    }    
    
    //cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
