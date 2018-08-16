#include "gf3d_extensions.h"
#include "gf3d_vector.h"

#include "simple_logger.h"

#include <vulkan/vulkan.h>

typedef struct
{
    Uint32                      enabled_extension_count;
    const char                **enabled_extension_names;
    
    Uint32                      available_extension_count;
    VkExtensionProperties      *available_extensions;
}vExtensions;

static vExtensions gf3d_extensions = {0};

void gf3d_extensions_close();

void gf3d_extensions_init()
{
    int i;
    
    vkEnumerateInstanceExtensionProperties(NULL, &gf3d_extensions.available_extension_count, NULL);
    slog("Total available extensions: %i",gf3d_extensions.available_extension_count);
    if (!gf3d_extensions.available_extension_count)return;

    gf3d_extensions.available_extensions = (VkExtensionProperties*)gf3d_allocate_array(sizeof (VkExtensionProperties),gf3d_extensions.available_extension_count);    
    if (!gf3d_extensions.available_extensions)return;

    gf3d_extensions.enabled_extension_names = gf3d_allocate_array(sizeof(const char *),gf3d_extensions.available_extension_count);
    if (!gf3d_extensions.enabled_extension_names)return;

    vkEnumerateInstanceExtensionProperties(NULL, &gf3d_extensions.available_extension_count, gf3d_extensions.available_extensions);
    
    for (i = 0;i < gf3d_extensions.available_extension_count; i++)
    {
        slog("available extension: %s",gf3d_extensions.available_extensions[i].extensionName);
    }
    atexit(gf3d_extensions_close);
}

void gf3d_extensions_close()
{
    if (gf3d_extensions.available_extensions)
    {
        free(gf3d_extensions.available_extensions);
    }
    if (gf3d_extensions.enabled_extension_names)
    {
        free(gf3d_extensions.enabled_extension_names);
    }

}

Bool gf3d_extensions_check_available(const char *extensionName)
{
    int i;
    for (i = 0; i < gf3d_extensions.available_extension_count;i++)
    {
        if (strcmp(gf3d_extensions.available_extensions[i].extensionName,extensionName) == 0)
        {
            return true;
        }
    }
    slog("Extension '%s' not available",extensionName);
    return false;
}

Bool gf3d_extensions_enable(const char *extensionName)
{
    int i;
    for (i = 0; i < gf3d_extensions.enabled_extension_count;i++)
    {
        if (strcmp(gf3d_extensions.enabled_extension_names[i],extensionName) == 0)
        {
            slog("Extension '%s' already enabled",extensionName);
            return false;
        }
    }
    if (!gf3d_extensions_check_available(extensionName))return false;
    if (gf3d_extensions.enabled_extension_count >= gf3d_extensions.available_extension_count)
    {
        slog("cannot enable extension '%s' no more space",extensionName);
        return false;
    }
    
    gf3d_extensions.enabled_extension_names[gf3d_extensions.enabled_extension_count++] = extensionName;
    return true;
}

const char* const* gf3d_extensions_get_enabled_names(Uint32 *count)
{
    if (count != NULL)*count = gf3d_extensions.enabled_extension_count;
    return gf3d_extensions.enabled_extension_names;
}


/*eol@eof*/
