#include <vulkan/vulkan.h>

#include "gf3d_validation.h"
#include "gf3d_types.h"
#include "gf3d_vector.h"

#include "simple_logger.h"

// validation layers
typedef struct
{
    Uint32 layerCount;
    VkLayerProperties *availableLayers;
}vValidation;

static vValidation gf3d_validation = {0};

void gf3d_validation_query_layer_properties()
{
    int i;
    vkEnumerateInstanceLayerProperties(&gf3d_validation.layerCount, NULL);
    slog("discovered %i validation layers",gf3d_validation.layerCount);
    
    if (!gf3d_validation.layerCount)return;
    
    gf3d_validation.availableLayers = (VkLayerProperties *)gf3d_allocate_array(sizeof(VkLayerProperties),gf3d_validation.layerCount);
    vkEnumerateInstanceLayerProperties(&gf3d_validation.layerCount, gf3d_validation.availableLayers);
    for (i = 0; i < gf3d_validation.layerCount;i++)
    {
        slog("Validation layer available: %s",gf3d_validation.availableLayers[i].layerName);
    }
}

void gf3d_validation_close()
{
    if (gf3d_validation.availableLayers)
    {
        free(gf3d_validation.availableLayers);
        gf3d_validation.availableLayers = NULL;
    }
}

void gf3d_validation_init()
{
    gf3d_validation_query_layer_properties();
    atexit(gf3d_validation_close);
}


/*eol@eof*/
