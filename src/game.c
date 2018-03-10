#include "gf3d_graphics.h"
#include "simple_logger.h"

int main(int argc,char *argv[])
{
    int done = 0;
    const Uint8 * keys;

    init_logger("gf2d.log");
    slog("gf3d begin");
    
    gf3d_graphics_initialize(
    "gametest3D",
    1200,
    700,
    1200,
    700,
    vector4d(1,1,1,1),
    0);
    
    // main game loop
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
    }    
    
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
