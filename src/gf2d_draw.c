#include "simple_logger.h"

#include "gfc_list.h"

#include "gf3d_vgraphics.h"

#include "gf2d_sprite.h"
#include "gf2d_draw.h"

typedef struct
{
    Sprite     *image;
    Shape       shape;
    Uint8       filled;
    Uint32      last_used;
}DrawImage;

typedef struct
{
    Uint32 ttl;
    List *draw_images;
}DrawManager;

static DrawManager draw_manager = {0};

void gf2d_draw_manager_close()
{
    int i,c;
    DrawImage *image;
    c = gfc_list_get_count(draw_manager.draw_images);
    for (i = 0;i < c; i++)
    {
        image = gfc_list_get_nth(draw_manager.draw_images,i);
        if (!image)continue;
        gf2d_sprite_free(image->image);
        free(image);
    }
    gfc_list_delete(draw_manager.draw_images);
    memset(&draw_manager,0,sizeof(DrawManager));
}

void gf2d_draw_manager_init(Uint32 ttl)
{
    draw_manager.ttl = ttl;
    draw_manager.draw_images = gfc_list_new();
    atexit (gf2d_draw_manager_close);
}

void gf2d_draw_image_free(DrawImage *image)
{
    if (!image)return;
    gf2d_sprite_free(image->image);
    free(image);
}

void gf2d_draw_manager_update()
{
    DrawImage *image;
    Uint32 now = SDL_GetTicks();
    int i;
    for (i = 0;i < gfc_list_get_count(draw_manager.draw_images);i++)
    {
        image = gfc_list_get_nth(draw_manager.draw_images,i);
        if (!image)continue;
        if ((now - image->last_used) < draw_manager.ttl)continue;
        gfc_list_delete_data(draw_manager.draw_images,image);
        gf2d_draw_image_free(image);
        i--;
    }
}

void gf2d_draw_image_new(
    Sprite     *sprite,
    Shape       shape,
    Uint8       filled
)
{
    DrawImage *image;
    if (!sprite)return;
    image = gfc_allocate_array(sizeof(DrawImage),1);
    if (!image)return;
    image->image = sprite;
    image->shape = shape;
    image->filled = filled;
    image->last_used = SDL_GetTicks();
    draw_manager.draw_images = gfc_list_append(draw_manager.draw_images,image);
}

DrawImage *gf2d_draw_image_get(
    Shape       shape,
    Uint8       filled)
{
    DrawImage *image;
    int i,c;
    c = gfc_list_get_count(draw_manager.draw_images);
    for (i = 0;i < c;i++)
    {
        image = gfc_list_get_nth(draw_manager.draw_images,i);
        if (!image)continue;
        if (image->filled != filled)continue;
        if (!gfc_shape_compare(image->shape, shape))continue;
        return image;
    }
    return NULL;
}


void gf2d_draw_rect(Rect rect,Color color)
{
    SDL_Surface *surface;
    Sprite *sprite;
    SDL_Rect rects[4];
    Shape shape;
    DrawImage *image = NULL;
    
    shape = gfc_shape_from_rect(gfc_rect(0,0,rect.w,rect.h));
    image = gf2d_draw_image_get(shape,0);
    if (image)
    {
        image->last_used = SDL_GetTicks();
        gf2d_sprite_draw(image->image,vector2d(rect.x,rect.y),vector2d(1,1),vector3d(0,0,0),color,0);
        return;
    }
    if ((!rect.w)||(!rect.h))
    {
        slog("cannot render a zero dimension rectangle");
    }
    surface = gf3d_vgraphics_create_surface((Uint32)rect.w + 1,(Uint32)rect.h + 1);
    if (!surface)
    {
        slog("failed to create surface for rectangle draw");
    }
    rects[0] = gfc_rect_to_sdl_rect(gfc_rect(0,0,rect.w,2));
    rects[1] = gfc_rect_to_sdl_rect(gfc_rect(0,0,2,rect.h));
    rects[2] = gfc_rect_to_sdl_rect(gfc_rect(0,rect.h - 2,rect.w,2));
    rects[3] = gfc_rect_to_sdl_rect(gfc_rect(rect.w - 2,0,2,rect.h));

    SDL_FillRects(surface, rects, 4, SDL_MapRGBA(surface->format,255,255,255,255));
    
    sprite = gf2d_sprite_from_surface(surface,0,0, 1);
    
    gf2d_sprite_draw(sprite,vector2d(rect.x,rect.y),vector2d(1,1),vector3d(0,0,0),color,0);
    gf2d_draw_image_new(sprite,shape,0);
}

void gf2d_draw_rect_filled(Rect rect,Color color)
{
    SDL_Surface *surface;
    Sprite *sprite;
    SDL_Rect rects;
    Shape shape;
    DrawImage *image = NULL;
    
    shape = gfc_shape_from_rect(gfc_rect(0,0,rect.w,rect.h));
    image = gf2d_draw_image_get(shape,1);
    if (image)
    {
        image->last_used = SDL_GetTicks();
        gf2d_sprite_draw(image->image,vector2d(rect.x,rect.y),vector2d(1,1),vector3d(0,0,0),color,0);
        return;
    }
    if ((!rect.w)||(!rect.h))
    {
        slog("cannot render a zero dimension rectangle");
    }
    surface = gf3d_vgraphics_create_surface((Uint32)rect.w + 1,(Uint32)rect.h + 1);
    if (!surface)
    {
        slog("failed to create surface for rectangle draw");
    }
    rects = gfc_rect_to_sdl_rect(shape.s.r);

    SDL_FillRect(surface, &rects,SDL_MapRGBA(surface->format,255,255,255,255));
    
    sprite = gf2d_sprite_from_surface(surface,0,0, 1);
    
    gf2d_sprite_draw(sprite,vector2d(rect.x,rect.y),vector2d(1,1),vector3d(0,0,0),color,0);
    gf2d_draw_image_new(sprite,shape,1);
}

#if 0

void gf2d_draw_shape(Shape shape,Color color,Vector2D offset)
{
    switch(shape.type)
    {
        case ST_RECT:
            vector2d_add(shape.s.r,shape.s.r,offset);
            gf2d_draw_rect(shape.s.r,color);
            break;
        case ST_CIRCLE:
            gf2d_draw_circle(vector2d(shape.s.c.x + offset.x,shape.s.c.y + offset.y), shape.s.c.r,color);
            break;
        case ST_EDGE:
            gf2d_draw_line(vector2d(shape.s.e.x1 + offset.x,shape.s.e.y1 + offset.y),vector2d(shape.s.e.x2 + offset.x,shape.s.e.y2 + offset.y),color);
            break;
    }
}

List *gf2d_draw_get_bezier_points(Vector2D p0, Vector2D p1, Vector2D p2)
{
    List *points;
    Vector2D *point;
    Vector2D qp,qp2,qpv; /*approximation line starting point and vector*/
    Vector2D p0v,p1v,temp; /*vectors from point to next point*/
    Vector2D dp; /*draw point*/
    float t = 0;  /*time segment*/
    float tstep;
    float totalLength;
    vector2d_sub(p0v,p1,p0);
    vector2d_sub(p1v,p2,p1);
    totalLength = vector2d_magnitude(p0v)+vector2d_magnitude(p1v);
    if (totalLength == 0)
    {
        return NULL;
    }
    tstep = fabs(1.0/(totalLength * 0.9));
    vector2d_sub(p0v,p1,p0);
    vector2d_sub(p1v,p2,p1);
    points = gfc_list_new();
    for (t = 0; t <= 1;t += tstep)
    {
        /*calculate Q*/
        vector2d_scale(temp,p0v,t);
        vector2d_add(qp,p0,temp);
        
        vector2d_scale(temp,p1v,t);
        vector2d_add(qp2,p1,temp);
        
        vector2d_sub(qpv,qp2,qp);
        
        vector2d_scale(temp,qpv,t);
        vector2d_add(dp,qp,temp);
        point = gfc_allocate_array(sizeof(Vector2D),1);
        if (!point)continue;
        vector2d_copy((*point),dp);
        points = gfc_list_append(points,point);
    }
    return points;
}

SDL_Point *gf2d_draw_point_list_to_array(List *points)
{
    int i,c;
    Vector2D *point;
    SDL_Point *array;
    if (!points)return NULL;
    c = gfc_list_get_count(points);
    if (!c)return NULL;
    array = gfc_allocate_array(sizeof(SDL_Point),c);
    for (i = 0; i < c; i++)
    {
        point = gfc_list_get_nth(points,i);
        vector2d_copy(array[i],(*point));
    }
    return array;
}

void gf2d_draw_polygon(List *points,Color color)
{
    int c;
    Color drawColor;
    SDL_Point *array;
    c = gfc_list_get_count(points);
    if (c < 2)return;

    array = gf2d_draw_point_list_to_array(points);
    if (!array)return;
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    SDL_RenderDrawLines(gf2d_graphics_get_renderer(),
                        array,
                        c);
    SDL_RenderDrawLine(gf2d_graphics_get_renderer(),
                    array[0].x,
                    array[0].y,
                    array[c-1].x,
                    array[c-1].y);

    free(array);
}

void gf2d_draw_free_points(void *data)
{
    Vector2D *vec;
    if (data)
    {
        vec = data;
        free(vec);
    }
}

void gf2d_draw_point_list(List *points,Color color)
{
    int c;
    SDL_Point *array;
    Color drawColor;
    if (!points)return;
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    array = gf2d_draw_point_list_to_array(points);
    if (!array)return;
    c = gfc_list_get_count(points);
    SDL_RenderDrawPoints(gf2d_graphics_get_renderer(),
                        array,
                        c);
    free(array);
}

void gf2d_draw_bezier_curve(Vector2D p0, Vector2D p1, Vector2D p2,Color color)
{
    List *points;
    points = gf2d_draw_get_bezier_points(p0, p1, p2);
    if (!points)return;
    gf2d_draw_point_list(points,color);
    gfc_list_foreach(points,gf2d_draw_free_points);
    gfc_list_delete(points);
}

void gf2d_draw_bezier4_curve(Vector2D p0,Vector2D r0,Vector2D r1,Vector2D p1,Color color)
{
    Vector2D *points;
    int length;
    int index = 0;
    float t,step;
    Color drawColor;
    //source: https://programmerbay.com/c-program-to-draw-bezier-curve-using-4-control-points/
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    length = vector2d_magnitude(vector2d(p1.x - r1.x,p1.y - r1.y))
            +vector2d_magnitude(vector2d(r1.x - r0.x,r1.y - r0.y))
            +vector2d_magnitude(vector2d(p0.x - r0.x,p0.y - r0.y));
    if (length == 0)return;
    points = gfc_allocate_array(sizeof(Vector2D),length);
    if (!points)return;
    step = 1/(float)length;
    for (t= 0,index = 0; index < length;t += step,index++)
    {
        points[index].x = pow(1-t,3)*p0.x + 3*t*pow(1-t,2)*r0.x + 3*t*t*(1-t)*r1.x + pow(t,3)*p1.x;
        points[index].y = pow(1-t,3)*p0.y + 3*t*pow(1-t,2)*r0.y + 3*t*t*(1-t)*r1.y + pow(t,3)*p1.y;
    }
    SDL_RenderDrawPointsF(gf2d_graphics_get_renderer(),
                          (SDL_FPoint*)points,
                          length);
    free(points);
}


void gf2d_draw_lines(Vector2D *p1,Vector2D *p2, Uint32 lines,Color color);

void gf2d_draw_diamond(Vector2D center, int radius, Color color)
{
    int i;
    Vector2D p1[4] = {0},p2[4] = {0};
    p1[0].y -= radius;
    p2[0].x += radius;

    p1[1].x += radius;
    p2[1].y += radius;
    
    p1[2].y += radius;
    p2[2].x -= radius;
    
    p1[3].x -= radius;
    p2[3].y -= radius;
    for (i = 0;i < 4;i++)
    {
        vector2d_add(p1[i],p1[i],center);
        vector2d_add(p2[i],p2[i],center);
    }
    gf2d_draw_lines(p1,p2, 4,color);
}

void gf2d_draw_lines(Vector2D *p1,Vector2D *p2, Uint32 lines,Color color)
{
    int i;
    Color drawColor;
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    for (i = 0; i < lines;i++)
    {
        SDL_RenderDrawLine(gf2d_graphics_get_renderer(),
                           p1[i].x,
                           p1[i].y,
                           p2[i].x,
                           p2[i].y);
    }
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           255,
                           255,
                           255,
                           255);
}

void gf2d_draw_line(Vector2D p1,Vector2D p2, Color color)
{
    Color drawColor;
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    SDL_RenderDrawLine(gf2d_graphics_get_renderer(),
                       p1.x,
                       p1.y,
                       p2.x,
                       p2.y);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           255,
                           255,
                           255,
                           255);
}

void gf2d_draw_rects(SDL_Rect *rects,Uint32 count,Color color)
{
    Color drawColor;
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    SDL_RenderDrawRects(gf2d_graphics_get_renderer(),rects,count);
}

void gf2d_draw_pixel(Vector2D pixel,Color color)
{
    Color drawColor;
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    SDL_RenderDrawPoint(gf2d_graphics_get_renderer(),
                        pixel.x,
                        pixel.y);
}

void gf2d_draw_pixel_list(SDL_Point * pixels,Uint32 count,Color color)
{
    Color drawColor;
    drawColor = gfc_color_to_int8(color);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    SDL_RenderDrawPoints(gf2d_graphics_get_renderer(),
                        pixels,
                        count);
}

/*
 * credit circle drawing algorith:
 * http://groups.csail.mit.edu/graphics/classes/6.837/F98/Lecture6/circle.html
 */

static int gf2d_draw_circle_points(SDL_Point *p,Vector2D center, Vector2D point)
{  
  if (point.x == 0)
  {
    vector2d_set(p[0],center.x, center.y + point.y);
    vector2d_set(p[1],center.x, center.y - point.y);
    vector2d_set(p[2],center.x + point.y, center.y);
    vector2d_set(p[3],center.x - point.y, center.y);
    return 4;
  }
  if (point.x == point.y)
  {
    vector2d_set(p[0],center.x + point.x, center.y + point.y);
    vector2d_set(p[1],center.x - point.x, center.y + point.y);
    vector2d_set(p[2],center.x + point.x, center.y - point.y);
    vector2d_set(p[3],center.x - point.x, center.y - point.y);
    return 4;
  }
  if (point.x < point.y)
  {
    vector2d_set(p[0],center.x + point.x, center.y + point.y);
    vector2d_set(p[1],center.x - point.x, center.y + point.y);
    vector2d_set(p[2],center.x + point.x, center.y - point.y);
    vector2d_set(p[3],center.x - point.x, center.y - point.y);
    vector2d_set(p[4],center.x + point.y, center.y + point.x);
    vector2d_set(p[5],center.x - point.y, center.y + point.x);
    vector2d_set(p[6],center.x + point.y, center.y - point.x);
    vector2d_set(p[7],center.x - point.y, center.y - point.x);
    return 8;
  }
  return 0;
}

void gf2d_draw_circle(Vector2D center, int radius, Color color)
{
    SDL_Point *pointArray;
    int i = 0;
    Vector2D point = {0,0};
    int p = (5 - radius*4)/4;
    Color drawColor;
    drawColor = gfc_color_to_int8(color);
    point.y = radius;
    pointArray = (SDL_Point*)malloc(sizeof(SDL_Point)*radius*8);
    if (!pointArray)
    {
        slog("gf2d_draw_circle: failed to allocate points for circle drawing");
        return;
    }
    i = gf2d_draw_circle_points(&pointArray[i],center, point);
    while (point.x < point.y)
    {
        point.x++;
        if (p < 0)
        {
            p += 2*point.x+1;
        }
        else
        {
            point.y--;
            p += 2*(point.x-point.y)+1;
        }
        i += gf2d_draw_circle_points(&pointArray[i],center, point);
        if (i + 8 >= radius*8)
        {
            break;
        }
    }
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                           drawColor.r,
                           drawColor.g,
                           drawColor.b,
                           drawColor.a);
    SDL_RenderDrawPoints(gf2d_graphics_get_renderer(),pointArray,i);
    SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(),
                            255,
                            255,
                            255,
                            255);
    free(pointArray);
}

List *gf2d_draw_get_bezier4_points(
    Vector2D ep1,
    Vector2D rp1,
    Vector2D rp2,
    Vector2D ep2)
{
/*    List *points;
    Vector2D *point;
    Vector2D qp,qp2,qpv; //approximation line starting point and vector
    Vector2D p0v,p1v,p2v,temp; //vectors from point to next point
    Vector2D dp; //draw point
    float t = 0;  //time segment
    float tstep;
    float totalLength;
    vector2d_sub(p0v,rp1,ep1);
    vector2d_sub(p1v,rp2,rp1);
    vector2d_sub(p2v,ep2,rp2);//segments between each
    totalLength = vector2d_magnitude(p0v)+vector2d_magnitude(p1v)+vector2d_magnitude(p2v);
    if (totalLength == 0)
    {
        return NULL;
    }
    tstep = fabs(1.0/(totalLength * 0.9));
    points = gfc_list_new();
    for (t = 0; t <= 1;t += tstep)
    {
        vector2d_scale(temp,p0v,t);
        vector2d_add(qp,ep0,temp);
        
        vector2d_scale(temp,p1v,t);
        vector2d_add(qp2,p1,temp);
        
        vector2d_sub(qpv,qp2,qp);
        
        vector2d_scale(temp,qpv,t);
        vector2d_add(dp,qp,temp);
        point = gfc_allocate_array(sizeof(Vector2D),1);
        if (!point)continue;
        vector2d_copy((*point),dp);
        points = gfc_list_append(points,point);
    }
    return points;
        */return NULL;
}

void gf2d_bezier4_draw(
    Vector2D ep1,
    Vector2D rp1,
    Vector2D rp2,
    Vector2D ep2,
    Color color)
{
    List *points;
    points = gf2d_draw_get_bezier4_points(ep1,rp1,rp2,ep2);
    if (!points)return;
    gf2d_draw_point_list(points,color);
    gfc_list_foreach(points,gf2d_draw_free_points);
    gfc_list_delete(points);
}
#endif

/*eol@eof*/
