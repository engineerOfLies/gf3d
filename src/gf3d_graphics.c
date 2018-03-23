#include <SDL.h>
#include <GL/glew.h>
#include <stdlib.h>

#include "gf3d_graphics.h"
#include "simple_logger.h"
#include "gf3d_shaders.h"

/*local types*/
typedef struct
{
    SDL_Window   *   main_window;
    SDL_GLContext    gl_context;
    SDL_Texture  *   texture;

    Uint32 frame_delay;
    Uint32 now;
    Uint32 then;
    Bool print_fps;
    float fps; 

    Uint32 background_color;
    Vector4D background_color_v;

    Sint32 bitdepth;
    Uint32 rmask;
    Uint32 gmask;
    Uint32 bmask;
    Uint32 amask;
    
    GLuint program;                 /**<shader program*/
    Matrix4 projection;
}Graphics;

/*local gobals*/
static Graphics gf3d_graphics;

/*forward declarations*/
void gf3d_graphics_close();

void gf3d_graphics_initialize(
    char *windowName,
    int viewWidth,
    int viewHeight,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    int major,
    int minor
)
{
    Uint32 flags = SDL_WINDOW_OPENGL;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        slog("Unable to initilaize SDL system: %s",SDL_GetError());
        return;
    }
    atexit(SDL_Quit);
    if (fullscreen)
    {
        if (renderWidth == 0)
        {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            flags |= SDL_WINDOW_FULLSCREEN;
        }
    }
    gf3d_graphics.main_window = SDL_CreateWindow(windowName,
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             renderWidth, renderHeight,
                             flags);

    if (!gf3d_graphics.main_window)
    {
        slog("failed to create main window: %s",SDL_GetError());
        gf3d_graphics_close();
        return;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    gf3d_graphics.gl_context = SDL_GL_CreateContext(gf3d_graphics.main_window);
    if (!gf3d_graphics.gl_context)
    {
        slog("failed to create gl context: %s",SDL_GetError());
        gf3d_graphics_close();
        exit(0);
        return;
    }
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    glClearColor(bgcolor.x,bgcolor.y,bgcolor.z,bgcolor.w);
    
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(gf3d_graphics.main_window);
    
    vector4d_set(gf3d_graphics.background_color_v,bgcolor.x,bgcolor.y,bgcolor.z,bgcolor.w);
    
    gf3d_matrix_perspective(
        gf3d_graphics.projection,
        45,
        (float)renderWidth/(float)renderHeight,
        0.1,
        100.0
    );

    gf3d_graphics.program = gf3d_shader_program_load("shaders/basic.vert", "shaders/basic.frag");

    srand(SDL_GetTicks());
    atexit(gf3d_graphics_close);
    slog("graphics initialized");
}

void gf3d_graphics_close()
{
    if (gf3d_graphics.main_window)
    {
        SDL_DestroyWindow(gf3d_graphics.main_window);
    }
    gf3d_graphics.main_window = NULL;

    slog("graphics closed");
}

void gf3d_graphics_set_frame_delay(Uint32 frameDelay)
{
    gf3d_graphics.frame_delay = frameDelay;
}

float gf3d_graphics_get_frames_per_second()
{
    return gf3d_graphics.fps;
}

void gf3d_graphics_frame_delay()
{
    Uint32 diff;
    gf3d_graphics.then = gf3d_graphics.now;
    slog_sync();// make sure logs get written when we have time to write it
    gf3d_graphics.now = SDL_GetTicks();
    diff = (gf3d_graphics.now - gf3d_graphics.then);
    if (diff < gf3d_graphics.frame_delay)
    {
        SDL_Delay(gf3d_graphics.frame_delay - diff);
    }
    gf3d_graphics.fps = 1000.0/MAX(SDL_GetTicks() - gf3d_graphics.then,0.001);
}

void gf3d_graphics_next_frame()
{
    SDL_GL_SwapWindow(gf3d_graphics.main_window);
    gf3d_graphics_frame_delay();
    slog_sync();
}

void gf3d_graphics_clear_screen()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

SDL_Surface *gf3d_graphics_create_surface(Uint32 w,Uint32 h)
{
    SDL_Surface *surface;
    surface = SDL_CreateRGBSurface(
        0,w, h,
        gf3d_graphics.bitdepth,
        gf3d_graphics.rmask,
        gf3d_graphics.gmask,
        gf3d_graphics.bmask,
        gf3d_graphics.amask);
    return surface;
}

void gf3d_graphics_get_projection(Matrix4 projection)
{
    if (!projection)return;
    gf3d_matrix_copy(
        projection,
        gf3d_graphics.projection
    );
}

void gf3d_graphics_set_projection(Matrix4 projection)
{
    if (!projection)return;
    gf3d_matrix_copy(
        gf3d_graphics.projection,
        projection
    );
}

GLuint gf3d_graphics_get_shader_program_id()
{
    return gf3d_graphics.program;
}

/*eol@eof*/
