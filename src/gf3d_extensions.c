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

static vExtensions gf3d_instance_extensions = {0};
static vExtensions gf3d_device_extensions = {0};

void gf3d_extensions_instance_close();
void gf3d_extensions_device_close();

void gf3d_extensions_device_init(VkPhysicalDevice device)
{
    int i;
    
    vkEnumerateDeviceExtensionProperties(device,NULL, &gf3d_device_extensions.available_extension_count, NULL);
    slog("Total available device extensions: %i",gf3d_device_extensions.available_extension_count);
    if (!gf3d_device_extensions.available_extension_count)return;

    gf3d_device_extensions.available_extensions = (VkExtensionProperties*)gf3d_allocate_array(sizeof (VkExtensionProperties),gf3d_device_extensions.available_extension_count);    

    if (!gf3d_device_extensions.available_extensions)return;

    gf3d_device_extensions.enabled_extension_names = gf3d_allocate_array(sizeof(const char *),gf3d_device_extensions.available_extension_count);
    if (!gf3d_device_extensions.enabled_extension_names)return;

    vkEnumerateDeviceExtensionProperties(device,NULL, &gf3d_device_extensions.available_extension_count, gf3d_device_extensions.available_extensions);
    
    for (i = 0;i < gf3d_device_extensions.available_extension_count; i++)
    {
        slog("available device extension: %s",gf3d_device_extensions.available_extensions[i].extensionName);
    }
    atexit(gf3d_extensions_device_close);
}

void gf3d_extensions_device_close()
{
    if (gf3d_device_extensions.available_extensions)
    {
        free(gf3d_device_extensions.available_extensions);
    }
    if (gf3d_device_extensions.enabled_extension_names)
    {
        free(gf3d_device_extensions.enabled_extension_names);
    }

}

void gf3d_extensions_instance_init()
{
    int i;
    
    vkEnumerateInstanceExtensionProperties(NULL, &gf3d_instance_extensions.available_extension_count, NULL);
    slog("Total available instance extensions: %i",gf3d_instance_extensions.available_extension_count);
    if (!gf3d_instance_extensions.available_extension_count)return;

    gf3d_instance_extensions.available_extensions = (VkExtensionProperties*)gf3d_allocate_array(sizeof (VkExtensionProperties),gf3d_instance_extensions.available_extension_count);    
    if (!gf3d_instance_extensions.available_extensions)return;

    gf3d_instance_extensions.enabled_extension_names = gf3d_allocate_array(sizeof(const char *),gf3d_instance_extensions.available_extension_count);
    if (!gf3d_instance_extensions.enabled_extension_names)return;

    vkEnumerateInstanceExtensionProperties(NULL, &gf3d_instance_extensions.available_extension_count, gf3d_instance_extensions.available_extensions);
    
    for (i = 0;i < gf3d_instance_extensions.available_extension_count; i++)
    {
        slog("available instance extension: %s",gf3d_instance_extensions.available_extensions[i].extensionName);
    }
    atexit(gf3d_extensions_instance_close);
}

void gf3d_extensions_instance_close()
{
    if (gf3d_instance_extensions.available_extensions)
    {
        free(gf3d_instance_extensions.available_extensions);
    }
    if (gf3d_instance_extensions.enabled_extension_names)
    {
        free(gf3d_instance_extensions.enabled_extension_names);
    }

}

Bool gf3d_extensions_instance_check_available(const char *extensionName)
{
    int i;
    for (i = 0; i < gf3d_instance_extensions.available_extension_count;i++)
    {
        if (strcmp(gf3d_instance_extensions.available_extensions[i].extensionName,extensionName) == 0)
        {
            return true;
        }
    }
    slog("Extension '%s' not available",extensionName);
    return false;
}

Bool gf3d_extensions_instance_enable(const char *extensionName)
{
    int i;
    for (i = 0; i < gf3d_instance_extensions.enabled_extension_count;i++)
    {
        if (strcmp(gf3d_instance_extensions.enabled_extension_names[i],extensionName) == 0)
        {
            slog("Extension '%s' already enabled",extensionName);
            return false;
        }
    }
    if (!gf3d_extensions_instance_check_available(extensionName))return false;
    if (gf3d_instance_extensions.enabled_extension_count >= gf3d_instance_extensions.available_extension_count)
    {
        slog("cannot enable extension '%s' no more space",extensionName);
        return false;
    }
    
    gf3d_instance_extensions.enabled_extension_names[gf3d_instance_extensions.enabled_extension_count++] = extensionName;
    return true;
}

const char* const* gf3d_extensions_get_instance_enabled_names(Uint32 *count)
{
    if (count != NULL)*count = gf3d_instance_extensions.enabled_extension_count;
    return gf3d_instance_extensions.enabled_extension_names;
}


/*eol@eof*/
