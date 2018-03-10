#include <SDL.h>
#include <stdlib.h>

#include "gf3d_graphics.h"
#include "simple_logger.h"

/*local types*/
typedef struct
{
    SDL_Window   *   main_window;
    SDL_Renderer *   renderer;
    SDL_Texture  *   texture;
    SDL_Surface  *   surface;
    SDL_Surface  *   temp_buffer;

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
    Bool fullscreen
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
    
    
    
    gf3d_graphics.renderer = SDL_CreateRenderer(gf3d_graphics.main_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!gf3d_graphics.renderer)
    {
        slog("failed to create renderer: %s",SDL_GetError());
        gf3d_graphics_close();
        return;
    }
    
    SDL_SetRenderDrawColor(gf3d_graphics.renderer, 0, 0, 0, 255);
    SDL_RenderClear(gf3d_graphics.renderer);
    SDL_RenderPresent(gf3d_graphics.renderer);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(gf3d_graphics.renderer, renderWidth, renderHeight);

    gf3d_graphics.texture = SDL_CreateTexture(
        gf3d_graphics.renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        renderWidth, renderHeight);
    if (!gf3d_graphics.texture)
    {
        slog("failed to create screen texture: %s",SDL_GetError());
        gf3d_graphics_close();
        return;
    }
    
    SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ARGB8888,
                                    &gf3d_graphics.bitdepth,
                                    &gf3d_graphics.rmask,
                                    &gf3d_graphics.gmask,
                                    &gf3d_graphics.bmask,
                                    &gf3d_graphics.amask);

    
    gf3d_graphics.surface = SDL_CreateRGBSurface(0, renderWidth, renderHeight, gf3d_graphics.bitdepth,
                                        gf3d_graphics.rmask,
                                    gf3d_graphics.gmask,
                                    gf3d_graphics.bmask,
                                    gf3d_graphics.amask);
    
    if (!gf3d_graphics.surface)
    {
        slog("failed to create screen surface: %s",SDL_GetError());
        gf3d_graphics_close();
        return;
    }
    
    gf3d_graphics.background_color = SDL_MapRGB(gf3d_graphics.surface->format, bgcolor.x,bgcolor.y,bgcolor.z);
    vector4d_set(gf3d_graphics.background_color_v,bgcolor.x,bgcolor.y,bgcolor.z,bgcolor.w);
    SDL_SetRenderDrawBlendMode(gf3d_graphics_get_renderer(),SDL_BLENDMODE_BLEND);

    srand(SDL_GetTicks());
    atexit(gf3d_graphics_close);
    slog("graphics initialized");
}

void gf3d_graphics_close()
{
    if (gf3d_graphics.texture)
    {
        SDL_DestroyTexture(gf3d_graphics.texture);
    }
    if (gf3d_graphics.renderer)
    {
        SDL_DestroyRenderer(gf3d_graphics.renderer);
    }
    if (gf3d_graphics.main_window)
    {
        SDL_DestroyWindow(gf3d_graphics.main_window);
    }
    if (gf3d_graphics.surface)
    {
        SDL_FreeSurface(gf3d_graphics.surface);
    }
    if (gf3d_graphics.temp_buffer)
    {
        SDL_FreeSurface(gf3d_graphics.temp_buffer);
    }
    gf3d_graphics.surface = NULL;
    gf3d_graphics.main_window = NULL;
    gf3d_graphics.renderer = NULL;
    gf3d_graphics.texture = NULL;
    gf3d_graphics.temp_buffer = NULL;

    slog("graphics closed");
}

SDL_Renderer *gf3d_graphics_get_renderer()
{
    return gf3d_graphics.renderer;
}

SDL_Texture *gf3d_graphics_get_screen_texture()
{
    return gf3d_graphics.texture;
}

SDL_Surface *gf3d_graphics_get_screen_surface()
{
    return gf3d_graphics.surface;
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

void gf3d_grahics_next_frame()
{
    SDL_RenderPresent(gf3d_graphics.renderer);
    gf3d_graphics_frame_delay();
}

void gf3d_graphics_clear_screen()
{
    if (!gf3d_graphics.surface)
    {
        return;
    }
    SDL_SetRenderDrawColor(
        gf3d_graphics.renderer,
        gf3d_graphics.background_color_v.x,
        gf3d_graphics.background_color_v.y,
        gf3d_graphics.background_color_v.z,
        gf3d_graphics.background_color_v.w);
    SDL_FillRect(gf3d_graphics.surface,NULL,gf3d_graphics.background_color);
    SDL_RenderClear(gf3d_graphics.renderer);
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

void gf3d_graphics_render_texture_to_screen(SDL_Texture *texture,const SDL_Rect * srcRect,SDL_Rect * dstRect)
{
    if (!texture)return;
    if (!gf3d_graphics.renderer)
    {
        slog("no graphics rendering context");
        return;
    }
    if (SDL_RenderCopy(gf3d_graphics.renderer,
                   texture,
                   srcRect,
                   dstRect))
    {
        slog("failed to render:%s",SDL_GetError());
    }

}

void gf3d_graphics_blit_surface_to_screen(SDL_Surface *surface,const SDL_Rect * srcRect,SDL_Rect * dstRect)
{
    if (!surface)return;
    if (!gf3d_graphics.surface)
    {
        slog("no screen surface loaded");
        return;
    }
    SDL_BlitSurface(surface,
                    srcRect,
                    gf3d_graphics.surface,
                    dstRect);
}

SDL_Surface *gf3d_graphics_screen_convert(SDL_Surface **surface)
{
    SDL_Surface *convert;
    if (!(*surface))
    {
        slog("surface provided was NULL");
        return NULL;
    }
    if (!gf3d_graphics.surface)
    {
        slog("graphics not yet initialized");
        return NULL;
    }
    convert = SDL_ConvertSurface(*surface,
                       gf3d_graphics.surface->format,
                       0);
    if (!convert)
    {
        slog("failed to convert surface: %s",SDL_GetError());
        return NULL;
    }
    SDL_FreeSurface(*surface);
    *surface = NULL;
    return convert;
}

/*eol@eof*/
