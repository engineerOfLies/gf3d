#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_list.h"

#include "gf3d_vqueues.h"
#include "gf3d_validation.h"
#include "gf3d_extensions.h"
#include "gf3d_device.h"

extern int __DEBUG;

typedef struct
{
    SJson *config;                  /**<loaded config*/
    VkInstance instance;            /**<active vulkan instance*/
    List *device_list;              /**<list of GF3D_Device's*/
    VkPhysicalDevice *devices;      /**<array of physical device handles*/
    VkDevice          device;       /**<logical device handle*/
    int bestDevice;                 /**<index of the chosen physical device*/
    GF3D_Device *chosen_gpu;        /**< physical device to use for logical device*/
    VkSurfaceKHR renderSurface;     /**<vulkan surface target for the screen/window  owned by graphics*/
}GF3D_DeviceManager;

static GF3D_DeviceManager gf3d_device_manager = {0};

int gf3d_devices_enumerate();
void gf3d_device_manager_determine_best();
GF3D_Device *gf3d_device_get_info(VkPhysicalDevice device);
VkDevice gf3d_device_create_logic_device(Bool enableValidationLayers);
VkDeviceCreateInfo gf3d_device_get_logical_device_info(Bool enableValidationLayers);


void gf3d_device_manager_close()
{
    if (gf3d_device_manager.device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(gf3d_device_manager.device, NULL);
    }
    sj_free(gf3d_device_manager.config);
    if (gf3d_device_manager.device_list)
    {
        gfc_list_foreach(gf3d_device_manager.device_list,free);
    }
    gfc_list_delete(gf3d_device_manager.device_list);
    if (gf3d_device_manager.devices)
    {
        free(gf3d_device_manager.devices);
    }
    memset(&gf3d_device_manager,0,sizeof(GF3D_DeviceManager));
    if (__DEBUG)slog("gf2d_devices manager closed");
}

void gf3d_device_manager_init(const char *config, VkInstance instance, VkSurfaceKHR renderSurface)
{
    SJson *deviceConfig;
    short int enable_validation = false;
    if (!instance)
    {
        slog("no vulkan instance provided, failed to init device manager");
        return;
    }
    gf3d_device_manager.instance = instance;
    if (!config)
    {
        slog("no config file provided");
        return;
    }
    gf3d_device_manager.config = sj_load(config);
    if (!gf3d_device_manager.config)
    {
        slog("failed to load device config file: %s",config);
        return;
    }
    
    if (!gf3d_devices_enumerate())
    {
        slog("failed to enumerate devices");
        sj_free(gf3d_device_manager.config);
        return;
    }
    gf3d_device_manager.renderSurface = renderSurface;
    gf3d_device_manager.bestDevice = -1;
    gf3d_device_manager_determine_best();
    
    if (!gf3d_device_manager.chosen_gpu)
    {
        return;
    }
    
    deviceConfig = sj_object_get_value(gf3d_device_manager.config,"devices");
    if (deviceConfig)
    {
        sj_get_bool_value(sj_object_get_value(gf3d_device_manager.config,"enable_validation"),&enable_validation);
    }
    
    gf3d_vqueues_init(gf3d_device_manager.chosen_gpu->device,gf3d_device_manager.renderSurface);
    
    //setup device extensions
    gf3d_extensions_device_init(gf3d_device_manager.chosen_gpu->device);
    gf3d_extensions_enable(ET_Device,"VK_KHR_swapchain");//TODO: from CONFIG

    gf3d_device_create_logic_device(enable_validation);
    
    atexit(gf3d_device_manager_close);
    if (__DEBUG)slog("gf3d_devices manager initialized");
}

GF3D_Device *gf3d_device_get_info(VkPhysicalDevice device)
{
    GF3D_Device *device_info;
    SJson *device_config;
    
    if (!device)return NULL;
    device_info = gfc_allocate_array(sizeof(GF3D_Device),1);
    if (!device_info)return NULL;
    
    device_info->device = device;
    vkGetPhysicalDeviceFeatures(device, &device_info->deviceFeatures);
    vkGetPhysicalDeviceProperties(device, &device_info->deviceProperties);
    
    device_config = sj_object_get_value(gf3d_device_manager.config,"devices");
    if (device_config)
    {
        if (sj_object_get_value(device_config,"discrete"))
        {
            if (device_info->deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                device_info->score++;
            }
        }
        if (sj_object_get_value(device_config,"geometryShader"))
        {
            if (device_info->deviceFeatures.geometryShader)
            {
                device_info->score++;
            }
        }
        //TODO: add more features to check for
    }

    if (__DEBUG)
    {
        slog("Device Name: %s",device_info->deviceProperties.deviceName);
        slog("Dedicated GPU: %i",(device_info->deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)?1:0);
        slog("apiVersion: %i",device_info->deviceProperties.apiVersion);
        slog("driverVersion: %i",device_info->deviceProperties.driverVersion);
        slog("supports Geometry Shader: %i",device_info->deviceFeatures.geometryShader);
    }
    
    return device_info;
}


int gf3d_devices_enumerate()
{
    int i;
    Uint32 count;
    GF3D_Device *device;
    vkEnumeratePhysicalDevices(gf3d_device_manager.instance, &count, NULL);
    if (__DEBUG)slog("Discovered %i device(s) with this instance",count);
    if (!count)
    {
        slog("failed to create a vulkan instance with a usable device, no devices found");
        return 0;
    }
    slog_sync();
    
    gf3d_device_manager.device_list = gfc_list_new();
    gf3d_device_manager.devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice)*count);
    vkEnumeratePhysicalDevices(gf3d_device_manager.instance, &count, gf3d_device_manager.devices);
    
    for (i = 0; i < count; i++)
    {
        device = gf3d_device_get_info(gf3d_device_manager.devices[i]);
        if (!device)continue;
        gf3d_device_manager.device_list = gfc_list_append(gf3d_device_manager.device_list,device);
    }
    
    return 1;
}

void gf3d_device_manager_determine_best()
{
    int i,c;
    int bestValue = -1;
    GF3D_Device *device_info;
    GF3D_Device *device_best = NULL;
    c = gfc_list_get_count(gf3d_device_manager.device_list);
    for (i = 0; i < c; i++)
    {
        device_info = gfc_list_get_nth(gf3d_device_manager.device_list,i);
        if (!device_info)continue;
        if (device_info->score > bestValue)
        {
            bestValue = device_info->score;
            gf3d_device_manager.bestDevice = i;
            device_best = device_info;
        }
    }
    if (__DEBUG)
    {
        if (device_best)
        {
            slog("best CPU chosen: %s",device_best->deviceProperties.deviceName);
        }
        else
        {
            slog("failed to determine a best device!");
        }
    }
    gf3d_device_manager.chosen_gpu = device_best;
}

GF3D_Device *gf3d_device_get_chosen_gpu_info()
{
    return gf3d_device_manager.chosen_gpu;
}


VkPhysicalDevice gf3d_devices_get_device_by_name(const char *name)
{
    int i,c;
    GF3D_Device *device_info;
    c = gfc_list_get_count(gf3d_device_manager.device_list);
    for (i = 0; i < c; i++)
    {
        device_info = gfc_list_get_nth(gf3d_device_manager.device_list,i);
        if (!device_info)continue;
        if (strcmp(name,device_info->deviceProperties.deviceName)==0)
        {
            return device_info->device;
        }
    }
    return VK_NULL_HANDLE;
}

VkPhysicalDevice gf3d_devices_get_best_device()
{
    if (gf3d_device_manager.bestDevice == -1)
    {
        gf3d_device_manager_determine_best();
    }
    if (gf3d_device_manager.bestDevice == -1)
    {
        return VK_NULL_HANDLE;
    }
    return gf3d_device_manager.devices[gf3d_device_manager.bestDevice];
}

VkDevice gf3d_device_get()
{
    return gf3d_device_manager.device;
}

VkDevice gf3d_device_create_logic_device(Bool enableValidationLayers)
{
    VkPhysicalDevice gpu;
    VkDeviceCreateInfo createInfo = gf3d_device_get_logical_device_info(enableValidationLayers);
    
    if (createInfo.sType != VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO)
    {
        slog("failed to setup creation info for logical device");
        return VK_NULL_HANDLE;
    }
    gpu = gf3d_devices_get_best_device();
    if (gpu == VK_NULL_HANDLE)
    {
        slog("cannot create logical device, no physical device set");
        return VK_NULL_HANDLE;
    }
    if (vkCreateDevice(gpu, &createInfo, NULL, &gf3d_device_manager.device) != VK_SUCCESS)
    {
        slog("failed to create logical device");
        return VK_NULL_HANDLE;
    }
    return gf3d_device_manager.device;
}

VkDeviceCreateInfo gf3d_device_get_logical_device_info(Bool enableValidationLayers)
{
    VkDeviceCreateInfo createInfo = {0};
    Uint32 count;
    VkDeviceQueueCreateInfo *queueCreateInfo;

    
    if (!gf3d_device_manager.chosen_gpu)
    {
        slog("cannot create logical device info, no GPU chosen");
        return createInfo;
    }
    
    queueCreateInfo = (VkDeviceQueueCreateInfo *)gf3d_vqueues_get_queue_create_info(&count);
    if (!queueCreateInfo)
    {
        slog("cannot create logical device info, queueCreateInfo");
        return createInfo;
    }

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    createInfo.pQueueCreateInfos = queueCreateInfo;
    createInfo.queueCreateInfoCount = count;

    createInfo.pEnabledFeatures = &gf3d_device_manager.chosen_gpu->deviceFeatures;
    
    
    createInfo.ppEnabledExtensionNames = gf3d_extensions_get_device_enabled_names(&count);
    createInfo.enabledExtensionCount = count;
    

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = gf3d_validation_get_enabled_layer_count();
        createInfo.ppEnabledLayerNames = gf3d_validation_get_enabled_layer_names();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    
    return createInfo;
}

/*eol@eof*/
