#include <string.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_vector.h"
#include "gfc_list.h"
#include "gfc_pak.h"

#include "gf3d_validation.h"

typedef struct
{
    const char *name;
    VkLayerProperties *properties;
    Bool enabled;
}ValidationLayer;

// validation layers
typedef struct
{
    Uint32 layerCount;
    VkLayerProperties *availableLayers;
    const char **enabledLayers;
    Uint32 enabledCount;
    GFC_List *layers;
}GF3D_Validation_Manager;

static GF3D_Validation_Manager gf3d_validation = {0};

void gf3d_validation_layer_enable(const char *name,Bool enable);


void gf3d_validation_query_layer_properties()
{
    int i;
    ValidationLayer *newLayer;
    vkEnumerateInstanceLayerProperties(&gf3d_validation.layerCount, NULL);
    slog("discovered %i validation layers",gf3d_validation.layerCount);
    
    if (!gf3d_validation.layerCount)return;
    
    gf3d_validation.availableLayers = (VkLayerProperties *)gfc_allocate_array(sizeof(VkLayerProperties),gf3d_validation.layerCount);
    vkEnumerateInstanceLayerProperties(&gf3d_validation.layerCount, gf3d_validation.availableLayers);
    
    gf3d_validation.layers = gfc_list_new();
    for (i = 0; i < gf3d_validation.layerCount;i++)
    {
        newLayer = gfc_allocate_array(sizeof(ValidationLayer),1);
        if (!newLayer)continue;
        newLayer->properties = &gf3d_validation.availableLayers[i];
        newLayer->name = newLayer->properties->layerName;
        newLayer->enabled = 1;//enabled by default
        slog("Validation layer available: %s",gf3d_validation.availableLayers[i].layerName);
        gfc_list_append(gf3d_validation.layers,newLayer);
    }
}

void gf3d_validation_close()
{
    if (gf3d_validation.enabledLayers)
    {
        free(gf3d_validation.enabledLayers);// data pointed to by this is owned elsewhere
    }
    if (gf3d_validation.availableLayers)
    {
        free(gf3d_validation.availableLayers);
        gf3d_validation.availableLayers = NULL;
    }
    gfc_list_foreach(gf3d_validation.layers,free);
    gfc_list_delete(gf3d_validation.layers);
    memset(&gf3d_validation,0,sizeof(GF3D_Validation_Manager));
    slog("validation layers closed");

}


void gf3d_validation_config_layers(const char *config)
{
    SJson *json,*layers,*layer;
    const char *name;
    int i,c;
    if (!config)return;
    json = gfc_pak_load_json(config);
    if (!json)
    {
        return;
    }
    layers = sj_object_get_value(json,"disabled_layers");
    if (!layers)return;
    c = sj_array_get_count(layers);
    for (i = 0; i < c; i++)
    {
        layer = sj_array_get_nth(layers,i);
        if (!layer)continue;
        name = sj_get_string_value(layer);
        if (!name)continue;
        slog("disabling validation layer '%s'",name);
        gf3d_validation_layer_enable(name,0);//disbale the layer
    }
}

void gf3d_validation_build_enabled_layer_list()
{
    ValidationLayer *layer;
    int i,c;
    int index,count = 0;
    c = gfc_list_get_count(gf3d_validation.layers);
    //first pass to get count
    for (i = 0; i < c; i++)
    {
        layer = gfc_list_get_nth(gf3d_validation.layers,i);
        if (!layer)continue;
        if (layer->enabled)count++;
    }
    if (!count)return;// nothing to do
    gf3d_validation.enabledCount = count;
    gf3d_validation.enabledLayers = gfc_allocate_array(sizeof(char *),count);
    for (i = 0,index = 0; i < c; i++)
    {
        layer = gfc_list_get_nth(gf3d_validation.layers,i);
        if (!layer)continue;
        if (!layer->enabled)continue;
        gf3d_validation.enabledLayers[index++] = layer->name;
    }
}

void gf3d_validation_init(const char *config)
{
    slog("initializing validation layers");
    gf3d_validation_query_layer_properties();
    
    gf3d_validation_config_layers(config);
    gf3d_validation_build_enabled_layer_list();
    
    atexit(gf3d_validation_close);
}

Bool gf3d_validation_check_layer_support(char *layerName)
{
    int i;
    for (i = 0; i < gf3d_validation.layerCount;i++)
    {
        if (strcmp(layerName,gf3d_validation.availableLayers[i].layerName) == 0)
        {
            return true;
        }
    }
    return false;
}

void gf3d_validation_layer_enable(const char *name,Bool enable)
{
    ValidationLayer *layer;
    int i,c;
    c = gfc_list_get_count(gf3d_validation.layers);
    for (i = 0; i < c; i++)
    {
        layer = gfc_list_get_nth(gf3d_validation.layers,i);
        if (!layer)continue;
        if (strcmp(name,layer->name)==0)
        {
            layer->enabled = enable;
            return;
        }
    }
}

Uint32 gf3d_validation_get_available_layer_count()
{
    return gf3d_validation.layerCount;
}

Uint32 gf3d_validation_get_enabled_layer_count()
{
    return gf3d_validation.enabledCount;
}

VkLayerProperties *gf3d_validation_get_validation_layer_data()
{
    return gf3d_validation.availableLayers;
}

const char* const* gf3d_validation_get_enabled_layer_names()
{
    return (const char* const* )gf3d_validation.enabledLayers;
}

/*eol@eof*/
