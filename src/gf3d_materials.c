#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_text.h"
#include "gfc_config.h"

#include "gf2d_actor.h"
#include "gf3d_materials.h"

typedef struct
{
    GFC_List *materials;
}MaterialManager;

static MaterialManager material_manager = {0};

void gf3d_material_delete(GF3D_Material *material);

void gf3d_materials_close()
{
    if (!material_manager.materials)return;
    gf3d_materials_clear();
    gfc_list_delete(material_manager.materials);
}

void gf3d_materials_init()
{
    if (material_manager.materials)return;
    
    material_manager.materials = gfc_list_new();
}

void gf3d_materials_clear()
{
    int i,c;
    GF3D_Material *material;
    c = gfc_list_get_count(material_manager.materials);
    for (i = 0;i < c;i++)
    {
        material = gfc_list_get_nth(material_manager.materials,i);
        if (!material)continue;
        gf3d_material_delete(material);
    }    
}

GF3D_Material *gf3d_material_new()
{
    GF3D_Material *material;
    material = gfc_allocate_array(sizeof(GF3D_Material),1);
    if (!material)return 0;
    //setting defaults:
    material->_refCount = 1;
    material->diffuse = GFC_COLOR_GREY;
    material->specular = GFC_COLOR_WHITE;
    material->ambient = GFC_COLOR_WHITE;
    material->emissive = GFC_COLOR_BLACK;
    material->transparency = 1.0;
    material->shininess = 128;
    material->ior = 1.0;
    gfc_list_append(material_manager.materials,material);
    return material;
}

const char *chomp(const char *p)
{
    const char *c;
    c = strchr(p,'\n');//go to the end of line
    p = c + 1;//and then the next line
    return p;
}

void gf3d_material_load_mtl_file(const char *filename)
{
    FILE* file;
    const char *p;
    void *mem = NULL;
    GFC_TextLine buf;
    GFC_TextLine text;
    GF3D_Material *material = NULL;
    size_t size = 0;
    float x,y,z;
    int count = 0;
    size_t fileSize = 0;
    
    if (!filename)return;
    file = fopen(filename,"r");
    if (!file)
    {
        slog("failed to open material file %s",filename);
        return;
    }
    size = get_file_Size(file);
    if (!size)
    {
        slog("material file %s has size zero",filename);
        fclose(file);
        return;
    }
    mem = gfc_allocate_array(sizeof(char),size);
    if (!mem)
    {
        slog("failed to load material file %s data",filename);
        return;
    }
    fread(mem, size, 1, file);
    fclose(file);

    p = mem;
    while(sscanf(p, "%s", buf) != EOF)
    {
        p += strlen(buf);//skip what was checked so far
        if (buf[0] == '#')  //skip comments
        {
            p = chomp(p);
            continue;
        }
        if (gfc_strlcmp(buf,"newmtl") == 0)
        {
            p++;
            count++;
            material = gf3d_material_new();
            if (!material)
            {
                slog("out of space for new materials");
                return;
            }
            sscanf(p, "%s", text);
            slog("new material: %s",text);
            material->name = gfc_string(text);
            material->filename = gfc_string(filename);
            p = chomp(p);
            continue;
        }
        if (material == NULL)//skip everything until we start a newmtl
        {
            p = chomp(p);
            continue;            
        }
        if (gfc_strlcmp(buf,"illum") == 0)//illumination
        {
            sscanf(p,"%i",(int*)&material->illum);
            p = chomp(p);
            continue;            
        }
        switch(buf[0])
        {
            case 'd':
                sscanf(p,"%f",&material->transparency);
                break;
            case 'N':
                switch(buf[1])
                {
                    case 's':       //specular exponent
                        sscanf(p,"%f",&material->specularExponent);
                        break;
                    case 'i':       //index of refraction
                        sscanf(p,"%f",&material->ior);
                        break;
                }                
                break;
            case 'K':
                switch(buf[1])
                {
                    case 'a':       //ambient
                        sscanf(
                            p,
                            "%f %f %f",
                            &x,
                            &y,
                            &z
                        );
                        material->ambient = gfc_color(x,y,z,1.0);
                        break;
                    case 'd':       //diffuse
                        sscanf(
                            p,
                            "%f %f %f",
                            &x,
                            &y,
                            &z
                        );
                        material->diffuse = gfc_color(x,y,z,1.0);
                        break;
                    case 's':       //specular
                        sscanf(
                            p,
                            "%f %f %f",
                            &x,
                            &y,
                            &z
                        );
                        material->specular = gfc_color(x,y,z,1.0);
                        break;
                    case 'e':       //emissive
                        sscanf(
                            p,
                            "%f %f %f",
                            &x,
                            &y,
                            &z
                        );
                        material->emissive = gfc_color(x,y,z,1.0);
                        break;
                }
                break;
        }
        p = chomp(p);
    }
    free(mem);
}

GF3D_Material *gf3d_material_duplicate(GF3D_Material *from)
{
    GF3D_Material *to = NULL;
    if (!from)return NULL;
    to = gf3d_material_new();
    if (!to)return NULL;
    memcpy(to,from,sizeof(GF3D_Material));
    to->_refCount = 1;
    gfc_string_append(to->name,".dup");//to make sure the search item is unique
    return to;
}

GF3D_Material *gf3d_material_parse_js(SJson *json,const char *filename)
{
    GF3D_Material *material = NULL;
    if (!json)return NULL;
    material = gf3d_material_new();
    if (!material)return NULL;
    material->filename = gfc_string(filename);
    material->name = sj_object_get_gfc_string(json,"name");
    material->ambient = sj_object_get_color(json,"ambient");
    material->diffuse = sj_object_get_color(json,"diffuse");
    sj_object_get_color_value(json,"specular",&material->specular);
    sj_object_get_color_value(json,"emit",&material->emissive);
    sj_object_get_value_as_float(json,"transparency",&material->transparency);
    sj_object_get_value_as_float(json,"shininess",&material->shininess);
    return material;
}

GF3D_Material *gf3d_material_get_by_name(const char *name)
{
    int i,c;
    GF3D_Material *material;
    c = gfc_list_get_count(material_manager.materials);
    for (i = 0;i < c;i++)
    {
        material = gfc_list_get_nth(material_manager.materials,i);
        if (!material)continue;
        if (gfc_string_l_strcmp(material->name,name)== 0)
        {
            return material;
        }
    }
    return NULL;
}

GF3D_Material *gf3d_material_get_by_file_name(const char *filename)
{
    int i,c;
    GF3D_Material *material;
    c = gfc_list_get_count(material_manager.materials);
    for (i = 0;i < c;i++)
    {
        material = gfc_list_get_nth(material_manager.materials,i);
        if (!material)continue;
        if (gfc_string_l_strcmp(material->filename,filename)!= 0)continue;
        return material;
    }
    return NULL;
}

GF3D_Material *gf3d_material_load(const char *filename)
{
    SJson *json;
    GF3D_Material *material;
    if (!filename)return NULL;
    material = gf3d_material_get_by_file_name(filename);
    if (material)
    {
        material->_refCount++;
        return material;
    }
    json = sj_load(filename);
    if (!json)return NULL;
    material = gf3d_material_parse_js(sj_object_get_value(json,"material"),filename);
    sj_free(json);
    return material;
}

void gf3d_material_free(GF3D_Material *material)
{
    if (!material)return;
    material->_refCount--;
    if (material->_refCount == 0)
    {
        gfc_list_delete_data(material_manager.materials,material);
        gf3d_material_delete(material);
    }
}

void gf3d_material_delete(GF3D_Material *material)
{
    if (!material)return;
    gfc_string_free(material->filename);
    free(material);
}

MaterialUBO gf3d_material_get_ubo(GF3D_Material *material)
{
    MaterialUBO ubo = {0};
    if (!material)return ubo;
    
    ubo.ambient = gfc_color_to_vector4f(material->ambient);
    ubo.diffuse = gfc_color_to_vector4f(material->diffuse);
    ubo.specular = gfc_color_to_vector4f(material->specular);
    ubo.emission = gfc_color_to_vector4f(material->emissive);
    ubo.shininess = material->shininess;
    ubo.transparency = material->transparency;
    
    return ubo;
}

MaterialUBO gf3d_material_make_basic_ubo(GFC_Color diffuse)
{
    MaterialUBO material = {0};
    material.ambient = gfc_vector4d(1,1,1,1);
    material.specular = gfc_vector4d(1,1,1,1);
    material.diffuse = gfc_color_to_vector4f(diffuse);
    material.emission = gfc_vector4d(0,0,0,0);
    material.shininess = 128;
    material.transparency = 1.0;
    return material;
}

/*eol@eof*/
