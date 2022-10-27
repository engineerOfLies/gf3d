#include "simple_logger.h"

#include "gfc_text.h"
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_shape.h"

#include "gf3d_vgraphics.h"
#include "gf3d_texture.h"
#include "gf2d_sprite.h"
#include "gf2d_font.h"

typedef struct
{
    Sprite     *image;
    TextBlock   text;
    Color       color;
    Font       *font;
    Uint32      last_used;
}FontImage;

typedef struct
{
    Font *font_list;
    Font *font_tags[FT_MAX];
    Uint32 font_max;
    List *font_images;
    Uint32 ttl;         //time to live for font re-use
}FontManager;

static FontManager font_manager = {0};

void gf2d_fonts_load(const char *filename);
void gf2d_fonts_load_json(const char *filename);

void gf2d_font_close()
{
    int i,c;
    FontImage *image;
    for (i = 0;i < font_manager.font_max;i++)
    {
        if (font_manager.font_list[i].font != NULL)
        {
            TTF_CloseFont(font_manager.font_list[i].font);
        }
    }
    c = gfc_list_get_count(font_manager.font_images);
    for (i= 0; i < c; i++)
    {
        image = gfc_list_get_nth(font_manager.font_images,i);
        if (!image)continue;
        gf2d_sprite_free(image->image);
        free(image);
    }
    gfc_list_delete(font_manager.font_images);
    TTF_Quit();
    slog("text system closed");
}

void gf2d_font_image_free(FontImage *image)
{
    if (!image)return;
    gf2d_sprite_free(image->image);
    free(image);
}

void gf2d_font_image_new(
    Sprite     *sprite,
    TextBlock   text,
    Color       color,
    Font       *font
)
{
    FontImage *image;
    if (!sprite)return;
    image = gfc_allocate_array(sizeof(FontImage),1);
    if (!image)return;
    image->image = sprite;
    gfc_block_cpy(image->text,text);
    image->color = color;
    image->font = font;
    image->last_used = SDL_GetTicks();
    font_manager.font_images = gfc_list_append(font_manager.font_images,image);
}

FontImage *gf2d_font_image_get(
    TextBlock   text,
    Color       color,
    Font       *font)
{
    FontImage *image;
    int i,c;
    c = gfc_list_get_count(font_manager.font_images);
    for (i = 0;i < c;i++)
    {
        image = gfc_list_get_nth(font_manager.font_images,i);
        if (!image)continue;
        if (image->font != font)continue;
        if (!gfc_color_cmp(image->color, color))continue;
        if (gfc_block_cmp(image->text,text)!= 0)continue;
        return image;
    }
    return NULL;
}

void gf2d_font_init(const char *configFile)
{
    if (TTF_Init() == -1)
    {
        slog("TTF_Init: %s\n", TTF_GetError());
        return;
    }
    gf2d_fonts_load_json(configFile);
    font_manager.font_images = gfc_list_new();
    font_manager.ttl = 1000;// 1000 milliseconds
    slog("text system initialized");
    atexit(gf2d_font_close);
}

void gf2d_font_update()
{
    FontImage *image;
    Uint32 now = SDL_GetTicks();
    int i;
    for (i = 0;i < gfc_list_get_count(font_manager.font_images);i++)
    {
        image = gfc_list_get_nth(font_manager.font_images,i);
        if (!image)continue;
        if ((now - image->last_used) < font_manager.ttl)continue;
        gfc_list_delete_data(font_manager.font_images,image);
        gf2d_font_image_free(image);        
        i--;
    }
}

int gf2d_fonts_get_count(FILE *file)
{
    char buf[512];
    int count = 0;
    if (!file)return 0;
    rewind(file);
    while(fscanf(file, "%s", buf) != EOF)
    {
        if(strcmp(buf,"font:") == 0)
        {
            count++;
        }
        fgets(buf, sizeof(buf), file);
    }
    return count;
}

FontTypes gf2d_font_type_from_text(const char *buf)
{
    if (strcmp(buf,"normal")==0)
    {
        return FT_Normal;
    }
    else if (strcmp(buf,"small")==0)
    {
        return FT_Small;
    }
    else if (strcmp(buf,"H1")==0)
    {
        return FT_H1;
    }
    else if (strcmp(buf,"H2")==0)
    {
        return FT_H2;
    }
    else if (strcmp(buf,"H3")==0)
    {
        return FT_H3;
    }
    else if (strcmp(buf,"H4")==0)
    {
        return FT_H4;
    }
    else if (strcmp(buf,"H5")==0)
    {
        return FT_H5;
    }
    else if (strcmp(buf,"H6")==0)
    {
        return FT_H6;
    }
    return FT_MAX;
}

void gf2d_fonts_parse(FILE *file)
{
    FontTypes fontType;
    Font *font;
    char buf[512];
    if (!file)return;
    rewind(file);
    font = font_manager.font_list;
    font--;
    while(fscanf(file, "%s", buf) != EOF)
    {
        if(strcmp(buf,"font:") == 0)
        {
            font++;
            fscanf(file,"%s",(char*)&font->filename);
            continue;
        }
        if(strcmp(buf,"size:") == 0)
        {
            fscanf(file,"%ui",&font->pointSize);
            continue;
        }
        if(strcmp(buf,"tag:") == 0)
        {
            fscanf(file,"%s",buf);
            fontType = gf2d_font_type_from_text(buf);
            if (fontType == FT_MAX)
            {
                fgets(buf, sizeof(buf), file);
                continue;
            }
            font_manager.font_tags[fontType] = font;
            continue;
        }
        fgets(buf, sizeof(buf), file);
    }
}

void gf2d_fonts_load_json(const char *filename)
{
    int i;
    int count = 0;
    const char *str;
    int size = 10;
    FontTypes fontType;
    SJson *file,*fonts,*item;
    file = sj_load(filename);
    if (!file)return;
    fonts = sj_object_get_value(file,"fonts");
    if (!fonts)
    {
        slog("font config %s does not contain fonts list",filename);
        sj_free(file);
        return;
    }
    count = sj_array_get_count(fonts);
    if (!count)
    {
        slog("font config has no fonts");
        sj_free(file);
        return;
    }
    font_manager.font_list = (Font*)gfc_allocate_array(sizeof(Font),count);
    for (i = 0; i < count; i++)
    {
        item = sj_array_get_nth(fonts,i);
        if (!item)continue;
        str = sj_get_string_value(sj_object_get_value(item,"font"));
        if (str)
        {
            gfc_line_cpy(font_manager.font_list[i].filename,str);
        }
        str = sj_get_string_value(sj_object_get_value(item,"tag"));
        if (str)
        {
            fontType = gf2d_font_type_from_text(str);
            if (fontType < FT_MAX)
            font_manager.font_tags[fontType] = &font_manager.font_list[i];
        }
        sj_get_integer_value(sj_object_get_value(item,"size"),&size);
        font_manager.font_list[i].pointSize = size;
        font_manager.font_list[i].font = TTF_OpenFont(font_manager.font_list[i].filename, font_manager.font_list[i].pointSize);
    }
    sj_free(file);
}

void gf2d_fonts_load(const char *filename)
{
    FILE *file;
    int count;
    int i;
    file = fopen(filename,"r");
    if (!file)
    {
        slog("failed to open font config file %s",filename);
        return;
    }
    count = gf2d_fonts_get_count(file);
    if (!count)
    {
        slog("font config file %s contained no font information",filename);
        fclose(file);
        return;
    }
    font_manager.font_list = (Font*)gfc_allocate_array(sizeof(Font),count);
    if (!font_manager.font_list)
    {
        slog("failed to allocate memory for %i fonts",count);
        fclose(file);
        return;
    }
    memset(font_manager.font_list,0,sizeof(Font)*count);
    for (i = 0 ; i < FT_MAX; i++)
    {
        font_manager.font_tags[i] = font_manager.font_list;
    }
    gf2d_fonts_parse(file);
    for (i = 0; i < count; i++)
    {
        font_manager.font_list[i].font = TTF_OpenFont(font_manager.font_list[i].filename, font_manager.font_list[i].pointSize);
        if (!font_manager.font_list[i].font)
        {
            slog("failed to load font: %s\n", TTF_GetError());
        }
    }
    font_manager.font_max = count;
    slog("font library loaded with %i fonts",count);
    fclose(file);
}

Font *gf2d_font_get_by_filename(char *filename)
{
    int i;
    if (!filename)return NULL;
    for (i = 0; i < font_manager.font_max;i++)
    {
        if (gfc_line_cmp(font_manager.font_list[i].filename,filename) == 0)
        {
            return &font_manager.font_list[i];
        }
    }
    return NULL;// not found
}

Font *gf2d_font_get_by_tag(FontTypes tag)
{
    if ((tag < 0) ||(tag >= FT_MAX))
    {
        slog("bad font tag: %i",tag);
        return NULL;
    }
    return font_manager.font_tags[tag];
}

void gf2d_font_draw_line_named(char *text,char *filename,Color color, Vector2D position)
{
    gf2d_font_draw_line(text,gf2d_font_get_by_filename(filename),color, position);
}

void gf2d_font_draw_line_tag(char *text,FontTypes tag,Color color, Vector2D position)
{
    gf2d_font_draw_line(text,gf2d_font_get_by_tag(tag),color, position);
}

void gf2d_font_draw_line(char *text,Font *font,Color color, Vector2D position)
{
    SDL_Surface *surface;
    Sprite *sprite;
    FontImage *image;
    if (!text)
    {
        slog("cannot draw text, none provided");
        return;
    }
    if (!font)
    {
        slog("cannot draw text, no font provided");
        return;
    }
    
    image = gf2d_font_image_get(text,color,font);
    
    if (image != NULL)
    {
        image->last_used = SDL_GetTicks();
        gf2d_sprite_draw(image->image,position,vector2d(1,1),vector3d(0,0,0),gfc_color(1,1,1,1),0);
        return;
    }

    surface = TTF_RenderUTF8_Blended(font->font, text, gfc_color_to_sdl(color));
    if (!surface)
    {
        return;
    }
    
    sprite = gf2d_sprite_from_surface(surface,0,0,0);
    if (!sprite)
    {
        slog("cannot draw text '%s', failed to make overlay sprite",text);
        SDL_FreeSurface(surface);
        return;
    }
    gf2d_sprite_draw(sprite,position,vector2d(1,1),vector3d(0,0,0),gfc_color(1,1,1,1),0);
    gf2d_font_image_new(sprite,text,color,font);
}

Vector2D gf2d_font_get_bounds_tag(char *text,FontTypes tag)
{
    return gf2d_font_get_bounds(text,gf2d_font_get_by_tag(tag));
}

Vector2D gf2d_font_get_bounds(char *text,Font *font)
{
    int x = -1,y = -1;
    if (!text)
    {
        slog("cannot size text, none provided");
        return vector2d(-1,-1);
    }
    if (!font)
    {
        slog("cannot size text, no font provided");
        return vector2d(-1,-1);
    }
    
    TTF_SizeUTF8(font->font, text, &x,&y);
    return vector2d(x,y);
}

void gf2d_font_chomp(char *text,int length,int strl)
{
    int i;
    if (!text)return;
    for(i = 0;i < strl - length;i++)
    {
        text[i] = text[i + length];
    }
    if (i > 0)
    {
        text[i - 1] = '\0';/*null terminate in case its overwritten*/
    }
    else
    {
        text[0] = '\0';
    }
}

Rect gf2d_font_get_text_wrap_bounds_tag(
    char       *thetext,
    FontTypes   tag,
    Uint32      w,
    Uint32      h
)
{
    return gf2d_font_get_text_wrap_bounds(thetext,gf2d_font_get_by_tag(tag),w,h);
}

Rect gf2d_font_get_text_wrap_bounds(
    char    *thetext,
    Font    *font,
    Uint32   w,
    Uint32   h
)
{
    Rect r = {0,0,0,0};
    TextBlock textline;
    TextBlock temptextline;
    TextBlock tempBuffer;
    TextBlock text;
    TextLine word;
    Bool whitespace;
    int tw = 0, th = 0;
    int drawheight = 0;
    int i = 0;
    int space = 0;
    int lindex = 0;
    if((thetext == NULL)||(thetext[0] == '\0'))
    {
        return r;
    }
    if (font == NULL)
    {
        slog("no font provided for draw.");
        return r;
    }
    
    gfc_block_cpy(text,thetext);
    temptextline[0] = '\0';
    do
    {
        space = 0;
        i = 0;
        whitespace = false;
        do
        {
            if(sscanf(&text[i],"%c",&word[0]) == EOF)break;
            if(word[0] == ' ')
            {
                space++;
                whitespace = true;
            }
            if(word[0] == '\t')
            {
                space+=2;
                whitespace = true;
            }
            i++;
        }while(whitespace);
        
        if(sscanf(text,"%s",word) == EOF)
        {
            break;
        }
        gf2d_font_chomp(text,strlen(word) + 1,GFCTEXTLEN);
        strncpy(textline,temptextline,GFCTEXTLEN);/*keep the last line that worked*/
        for(i = 0;i < (space - 1);i++)
        {
            gfc_block_sprintf(tempBuffer,"%s%c",temptextline,' '); /*add spaces*/
            gfc_block_cpy(temptextline,tempBuffer);
        }
        gfc_block_sprintf(tempBuffer,"%s %s",temptextline,word); /*add a word*/
        gfc_block_cpy(temptextline,tempBuffer);
        TTF_SizeText(font->font, temptextline, &tw, &th); /*see how big it is now*/
        lindex += strlen(word);
        if(tw > w)         /*see if we have gone over*/
        {
            drawheight += th;
            if (h != 0)
            {
                if ((drawheight + th) > h)
                {
                    break;
                }
            }
            gfc_block_sprintf(temptextline,"%s",word); /*add a word*/
        }
        else if (tw > r.w)
        {
            r.w = tw;
        }
    }while(1);
    r.h = drawheight + th;
    return r;
}



void gf2d_font_draw_text_wrap_tag(char *text,FontTypes tag,Color color, Rect block)
{
    gf2d_font_draw_text_wrap(text,block,color, gf2d_font_get_by_tag(tag));
}


void gf2d_font_draw_text_wrap(
    char    *thetext,
    Rect     block,
    Color    color,
    Font    *font
)
{
    TextBlock textline;
    TextBlock temptextline;
    TextBlock tempBuffer;
    TextBlock text;
    TextLine word;
    Bool whitespace;
    int drawheight = block.y;
    int w,h = 0;
    int row = 0;
    int i;
    int space;
    int lindex = 0;
    if ((thetext == NULL)||(thetext[0] == '\0'))
    {
        slog("no text provided for draw.");
        return;
    }
    if (font == NULL)
    {
        slog("no font provided for draw.");
        return;
    }
    if (font->font == NULL)
    {
        slog("bad Font provided for draw.");
        return;
    }

    gfc_block_cpy(text,thetext);
    temptextline[0] = '\0';
    do
    {
        space = 0;
        i = 0;
        do
        {
            whitespace = false;
            if(sscanf(&text[i],"%c",&word[0]) == EOF)break;
            if(word[0] == ' ')
            {
                space++;
                whitespace = true;
            }
            if(word[0] == '\t')
            {
                space+=2;
                whitespace = true;
            }
            i++;
        }while (whitespace);
        if (sscanf(text,"%s",word) == EOF)
        {
            block.y=drawheight + (h*row);
            gf2d_font_draw_line(temptextline,font,color, vector2d(block.x,block.y));
            return;
        }
        
        gf2d_font_chomp(text,strlen(word) + space,GFCTEXTLEN);
        strncpy(textline,temptextline,GFCTEXTLEN);/*keep the last line that worked*/
        for (i = 0;i < (space - 1);i++)
        {
            gfc_block_sprintf(tempBuffer,"%s%c",temptextline,' '); /*add spaces*/
            gfc_block_cpy(temptextline,tempBuffer);
        }
        gfc_block_sprintf(tempBuffer,"%s %s",temptextline,word); /*add a word*/
        gfc_block_cpy(temptextline,tempBuffer);
        TTF_SizeText(font->font, temptextline, &w, &h); /*see how big it is now*/
        lindex += strlen(word);
        if(w > block.w)         /*see if we have gone over*/
        {
            block.y=drawheight + (h*row);
            gf2d_font_draw_line(textline,font,color, vector2d(block.x,block.y));
            row++;
            /*draw the line and get ready for the next line*/
            if (block.h != 0)
            {
                if ((drawheight + (h*row)) > (block.y + block.h))
                {
                    break;
                }
            }
            gfc_block_sprintf(temptextline,"%s",word); /*add a word*/
        }
    }while(1);
    
}


/*eol@eof*/
