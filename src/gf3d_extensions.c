#include <string.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#include "simple_logger.h"
#include "gfc_vector.h"

#include "gf3d_extensions.h"

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
    Uint32 i;
    
    vkEnumerateDeviceExtensionProperties(device,NULL, &gf3d_device_extensions.available_extension_count, NULL);
    slog("Total available device extensions: %i",gf3d_device_extensions.available_extension_count);
    if (!gf3d_device_extensions.available_extension_count)return;

    gf3d_device_extensions.available_extensions = (VkExtensionProperties*)gfc_allocate_array(sizeof (VkExtensionProperties),gf3d_device_extensions.available_extension_count);    

    if (!gf3d_device_extensions.available_extensions)return;

    gf3d_device_extensions.enabled_extension_names = gfc_allocate_array(sizeof(const char *),gf3d_device_extensions.available_extension_count);
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
    slog("cleaning up device extensions");
    if (gf3d_device_extensions.available_extensions)
    {
        free(gf3d_device_extensions.available_extensions);
    }
    if (gf3d_device_extensions.enabled_extension_names)
    {
        free(gf3d_device_extensions.enabled_extension_names);
    }
    memset(&gf3d_device_extensions,0,sizeof(vExtensions));
}

void gf3d_extensions_instance_init()
{
    int i;
    
    vkEnumerateInstanceExtensionProperties(NULL, &gf3d_instance_extensions.available_extension_count, NULL);
    slog("Total available instance extensions: %i",gf3d_instance_extensions.available_extension_count);
    if (!gf3d_instance_extensions.available_extension_count)return;

    gf3d_instance_extensions.available_extensions = (VkExtensionProperties*)gfc_allocate_array(sizeof (VkExtensionProperties),gf3d_instance_extensions.available_extension_count);    
    if (!gf3d_instance_extensions.available_extensions)return;

    gf3d_instance_extensions.enabled_extension_names = gfc_allocate_array(sizeof(const char *),gf3d_instance_extensions.available_extension_count);
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
    slog("cleaning up instance extentions");
    if (gf3d_instance_extensions.available_extensions)
    {
        free(gf3d_instance_extensions.available_extensions);
    }
    if (gf3d_instance_extensions.enabled_extension_names)
    {
        free(gf3d_instance_extensions.enabled_extension_names);
    }
    memset(&gf3d_instance_extensions,0,sizeof(vExtensions));
}

Bool gf3d_extensions_check_available(vExtensions *extensions,const char *extensionName)
{
    int i;
    
    if (!extensions)return false;
    
    for (i = 0; i < extensions->available_extension_count;i++)
    {
        if (strcmp(extensions->available_extensions[i].extensionName,extensionName) == 0)
        {
            return true;
        }
    }
    slog("Extension '%s' not available",extensionName);
    return false;
}

Bool gf3d_extensions_enable(ExtensionType extType, const char *extensionName)
{
    vExtensions *extensions;
    Uint32 i;
    switch(extType)
    {
        case ET_Instance:
            extensions = &gf3d_instance_extensions;
        break;
        case ET_Device:
            extensions = &gf3d_device_extensions;
        break;
        default:
            slog("unknown extension type");
            return false;
    }
    for (i = 0; i < extensions->enabled_extension_count;i++)
    {
        if (strcmp(extensions->enabled_extension_names[i],extensionName) == 0)
        {
            slog("Extension '%s' already enabled",extensionName);
            return false;
        }
    }
    if (!gf3d_extensions_check_available(extensions,extensionName))return false;
    if (extensions->enabled_extension_count >= extensions->available_extension_count)
    {
        slog("cannot enable extension '%s' no more space",extensionName);
        return false;
    }
    
    extensions->enabled_extension_names[extensions->enabled_extension_count++] = extensionName;
    return true;
}

const char* const* gf3d_extensions_get_instance_enabled_names(Uint32 *count)
{
    if (count != NULL)*count = gf3d_instance_extensions.enabled_extension_count;
    return gf3d_instance_extensions.enabled_extension_names;
}

const char* const* gf3d_extensions_get_device_enabled_names(Uint32 *count)
{
    if (count != NULL)*count = gf3d_device_extensions.enabled_extension_count;
    return gf3d_device_extensions.enabled_extension_names;
}

/*eol@eof*/
