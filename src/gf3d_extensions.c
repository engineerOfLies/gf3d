#include <string.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_vector.h"

#include "gf3d_extensions.h"

extern int __DEBUG;

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
void gf3d_extensions_config(const char *config,ExtensionType extType);

void gf3d_extensions_device_init(VkPhysicalDevice device, const char *config)
{
    Uint32 i;
    
    vkEnumerateDeviceExtensionProperties(device,NULL, &gf3d_device_extensions.available_extension_count, NULL);
    if (__DEBUG)slog("Total available device extensions: %i",gf3d_device_extensions.available_extension_count);
    if (!gf3d_device_extensions.available_extension_count)return;

    gf3d_device_extensions.available_extensions = (VkExtensionProperties*)gfc_allocate_array(sizeof (VkExtensionProperties),gf3d_device_extensions.available_extension_count);    

    if (!gf3d_device_extensions.available_extensions)return;

    gf3d_device_extensions.enabled_extension_names = gfc_allocate_array(sizeof(const char *),gf3d_device_extensions.available_extension_count);
    if (!gf3d_device_extensions.enabled_extension_names)return;

    vkEnumerateDeviceExtensionProperties(device,NULL, &gf3d_device_extensions.available_extension_count, gf3d_device_extensions.available_extensions);
    
    if (__DEBUG)
    {
        for (i = 0;i < gf3d_device_extensions.available_extension_count; i++)
        {
            slog("available device extension: %s",gf3d_device_extensions.available_extensions[i].extensionName);
        }
    }
    gf3d_extensions_config(config,ET_Device);
    atexit(gf3d_extensions_device_close);
    slog("device extensions initialized");
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
    memset(&gf3d_device_extensions,0,sizeof(vExtensions));
    slog("device extensions closed");
}

void gf3d_extensions_instance_init(const char *config)
{
    int i;
    
    vkEnumerateInstanceExtensionProperties(NULL, &gf3d_instance_extensions.available_extension_count, NULL);
    if (__DEBUG)slog("Total available instance extensions: %i",gf3d_instance_extensions.available_extension_count);
    if (!gf3d_instance_extensions.available_extension_count)return;

    gf3d_instance_extensions.available_extensions = (VkExtensionProperties*)gfc_allocate_array(sizeof (VkExtensionProperties),gf3d_instance_extensions.available_extension_count);    
    if (!gf3d_instance_extensions.available_extensions)return;

    gf3d_instance_extensions.enabled_extension_names = gfc_allocate_array(sizeof(const char *),gf3d_instance_extensions.available_extension_count);
    if (!gf3d_instance_extensions.enabled_extension_names)return;

    vkEnumerateInstanceExtensionProperties(NULL, &gf3d_instance_extensions.available_extension_count, gf3d_instance_extensions.available_extensions);
    
    if (__DEBUG)
    {
        for (i = 0;i < gf3d_instance_extensions.available_extension_count; i++)
        {
            slog("available instance extension: %s",gf3d_instance_extensions.available_extensions[i].extensionName);
        }
    }
    gf3d_extensions_config(config,ET_Instance);
    atexit(gf3d_extensions_instance_close);
    slog("intance extensions initialized");
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
    memset(&gf3d_instance_extensions,0,sizeof(vExtensions));
    slog("instance extentions closed");
}

Bool gf3d_extensions_check_available(vExtensions *extensions,const char *extensionName, int *index)
{
    int i;
    
    if (!extensions)return false;
    
    for (i = 0; i < extensions->available_extension_count;i++)
    {
        if (strcmp(extensions->available_extensions[i].extensionName,extensionName) == 0)
        {
            if (index)*index = i;
            return true;
        }
    }
    slog("Extension '%s' not available",extensionName);
    return false;
}

void gf3d_extensions_config(const char *config,ExtensionType extType)
{
    int i,c;
    SJson *extensions,*json, *extension;
    const char *extensionName;
    if (!config)return;
    json = sj_load(config);
    if (!json)return;
    if (extType == ET_Instance)
    {
        extensions = sj_object_get_value(json,"instance_extensions");
        if (!extensions)
        {
            slog("config missing 'instance_extensions' array");
            sj_free(json);
            return;
        }
    }
    else if (extType == ET_Device)
    {
        extensions = sj_object_get_value(json,"device_extensions");
        if (!extensions)
        {
            slog("config missing 'device_extensions' array");
            sj_free(json);
            return;
        }
    }
    else
    {
        sj_free(json);
        return;
    }
    c = sj_array_get_count(extensions);
    for (i = 0;i < c; i++)
    {
        extension = sj_array_get_nth(extensions,i);
        if (!extension)continue;
        extensionName = sj_get_string_value(extension);
        if (!extensionName)continue;
        gf3d_extensions_enable(extType,extensionName);
    }
    
    sj_free(json);
}

Bool gf3d_extensions_enable(ExtensionType extType, const char *extensionName)
{
    vExtensions *extensions;
    Uint32 i;
    int index = 0;
    if (!extensionName)return false;
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
    if (!gf3d_extensions_check_available(extensions,extensionName,&index))return false;
    if (extensions->enabled_extension_count >= extensions->available_extension_count)
    {
        slog("cannot enable extension '%s' no more space",extensionName);
        return false;
    }
    
    extensions->enabled_extension_names[extensions->enabled_extension_count++] = extensions->available_extensions[index].extensionName;
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
