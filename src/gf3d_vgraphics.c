/**
 * @purpose vulkan graphics setup and abstraction
*/

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "gf3d_vector.h"
#include "gf3d_types.h"
#include "gf3d_validation.h"
#include "gf3d_extensions.h"
#include "gf3d_vqueues.h"
#include "simple_logger.h"

typedef struct
{
    VkImage image;
    VkImageView view;
}SwapChainBuffer;

typedef struct
{
    SDL_Window                 *main_window;

    VkApplicationInfo           vk_app_info;

    VkInstance                  vk_instance;
    VkInstanceCreateInfo        vk_instance_info;

    Uint32                      sdl_extension_count;
    const char                **sdl_extension_names;
    
    unsigned int                enabled_layer_count;

    //devices
    Uint32                      device_count;
    VkPhysicalDevice           *devices;
    VkPhysicalDevice            gpu;
    VkDevice                    device;
    VkSurfaceKHR                surface;
    VkDeviceCreateInfo          device_info;

    // color space
    VkFormat                    color_format;
    VkColorSpaceKHR             color_space;

    // swap chain
    VkSwapchainKHR              swap_chain;
    SwapChainBuffer            *buffers;
    VkImage                    *images;
    size_t                      node_index;
    
    // function pointers
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR        fpGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR   fpGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkCreateSwapchainKHR                        fpCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR                       fpDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR                     fpGetSwapchainImagesKHR;
    PFN_vkQueuePresentKHR                           fpQueuePresentKHR;
}vGraphics;

static vGraphics gf3d_vgraphics = {0};

void gf3d_vgraphics_close();
void gf3d_vgraphics_extension_init();
void gf3d_vgraphics_setup_debug();
VkPhysicalDevice gf3d_vgraphics_select_device();

void gf3d_vgraphics_init(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    Bool enableValidation
)
{
    Uint32 flags = SDL_WINDOW_VULKAN;
    Uint32 i;
    Uint32 enabledExtensionCount = 0;
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        slog("Unable to initilaize SDL system: %s",SDL_GetError());
        return;
    }
    atexit(SDL_Quit);
    if (fullscreen)
    {
        if (renderWidth == 0)
        {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else
        {
            flags |= SDL_WINDOW_FULLSCREEN;
        }
    }
    gf3d_vgraphics.main_window = SDL_CreateWindow(windowName,
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             renderWidth, renderHeight,
                             flags);

    if (!gf3d_vgraphics.main_window)
    {
        slog("failed to create main window: %s",SDL_GetError());
        gf3d_vgraphics_close();
        exit(0);
        return;
    }

    // extension configuration
    gf3d_extensions_init();
    
    // get the extensions that are needed for rendering to an SDL Window
    SDL_Vulkan_GetInstanceExtensions(gf3d_vgraphics.main_window, &(gf3d_vgraphics.sdl_extension_count), NULL);
    if (gf3d_vgraphics.sdl_extension_count > 0)
    {
        gf3d_vgraphics.sdl_extension_names = gf3d_allocate_array(sizeof(const char *),gf3d_vgraphics.sdl_extension_count);
        
        SDL_Vulkan_GetInstanceExtensions(gf3d_vgraphics.main_window, &(gf3d_vgraphics.sdl_extension_count), gf3d_vgraphics.sdl_extension_names);
        for (i = 0; i < gf3d_vgraphics.sdl_extension_count;i++)
        {
            slog("SDL Vulkan extensions support: %s",gf3d_vgraphics.sdl_extension_names[i]);
            gf3d_extensions_enable(gf3d_vgraphics.sdl_extension_names[i]);
        }
    }
    else
    {
        slog("SDL / Vulkan not supported");
        gf3d_vgraphics_close();
        exit(0);
        return;
    }

    // setup app info
    gf3d_vgraphics.vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    gf3d_vgraphics.vk_app_info.pNext = NULL;
    gf3d_vgraphics.vk_app_info.pApplicationName = windowName;
    gf3d_vgraphics.vk_app_info.applicationVersion = 0;
    gf3d_vgraphics.vk_app_info.pEngineName = windowName;
    gf3d_vgraphics.vk_app_info.engineVersion = 0;
    gf3d_vgraphics.vk_app_info.apiVersion = VK_API_VERSION_1_0;
    
    gf3d_vgraphics.vk_instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    gf3d_vgraphics.vk_instance_info.pNext = NULL;
    gf3d_vgraphics.vk_instance_info.pApplicationInfo = &gf3d_vgraphics.vk_app_info;
    
    if (enableValidation)
    {
        gf3d_validation_init();
        gf3d_vgraphics.vk_instance_info.enabledLayerCount = gf3d_validation_get_validation_layer_count();
        gf3d_vgraphics.vk_instance_info.ppEnabledLayerNames = gf3d_validation_get_validation_layer_names();
        gf3d_extensions_enable("VK_EXT_debug_utils");
    }
    else
    {
        //setup instance info
        gf3d_vgraphics.vk_instance_info.enabledLayerCount = 0;
        gf3d_vgraphics.vk_instance_info.ppEnabledLayerNames = NULL;
    }
    
    gf3d_vgraphics.vk_instance_info.ppEnabledExtensionNames = gf3d_extensions_get_enabled_names(&enabledExtensionCount);
    gf3d_vgraphics.vk_instance_info.enabledExtensionCount = enabledExtensionCount;

    // create instance
    vkCreateInstance(&gf3d_vgraphics.vk_instance_info, NULL, &gf3d_vgraphics.vk_instance);

    if (!gf3d_vgraphics.vk_instance)
    {
        slog("failed to create a vulkan instance");
        gf3d_vgraphics_close();
        return;
    }
    
    if (enableValidation)
    {
        gf3d_vgraphics_setup_debug();
    }
    
    //get a gpu to do work with
    vkEnumeratePhysicalDevices(gf3d_vgraphics.vk_instance, &gf3d_vgraphics.device_count, NULL);
    slog("vulkan discovered %i device(s) with this instance",gf3d_vgraphics.device_count);
    if (!gf3d_vgraphics.device_count)
    {
        slog("failed to create a vulkan instance with a usable device");
        gf3d_vgraphics_close();
        return;
    }

    gf3d_vgraphics.devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice)*gf3d_vgraphics.device_count);
    vkEnumeratePhysicalDevices(gf3d_vgraphics.vk_instance, &gf3d_vgraphics.device_count, gf3d_vgraphics.devices);
    
    gf3d_vgraphics.gpu = gf3d_vgraphics_select_device();
    
    // create a surface for the window
    SDL_Vulkan_CreateSurface(gf3d_vgraphics.main_window, gf3d_vgraphics.vk_instance, &gf3d_vgraphics.surface);
    // setup a queue for rendering calls
        
    // setup queues
    gf3d_vqueues_init(gf3d_vgraphics.gpu,gf3d_vgraphics.surface);

    atexit(gf3d_vgraphics_close);
}

void gf3d_vgraphics_close()
{
    if (gf3d_vgraphics.devices)
    {
        free(gf3d_vgraphics.devices);
    }
    if (gf3d_vgraphics.sdl_extension_names)
    {
        free(gf3d_vgraphics.sdl_extension_names);
    }
    if(gf3d_vgraphics.surface)
    {
        vkDestroySurfaceKHR(gf3d_vgraphics.vk_instance,gf3d_vgraphics.surface, NULL);
    }
    if (gf3d_vgraphics.vk_instance)
    {
        vkDestroyInstance(gf3d_vgraphics.vk_instance, NULL);
    }
    if (gf3d_vgraphics.main_window)
    {
        SDL_DestroyWindow(gf3d_vgraphics.main_window);
    }
}

void gf3d_vgraphics_clear()
{
    
}

void gf3d_vgraphics_render()
{
    
}

/**
 * VULKAN DEVEICE SUPPORT
 */

Bool gf3d_vgraphics_device_validate(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    
    slog("Device Name: %s",deviceProperties.deviceName);
    slog("Dedicated GPU: %i",(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)?1:0);
    slog("apiVersion: %i",deviceProperties.apiVersion);
    slog("driverVersion: %i",deviceProperties.driverVersion);
    slog("supports Geometry Shader: %i",deviceFeatures.geometryShader);
    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)&&(deviceFeatures.geometryShader);
}

VkPhysicalDevice gf3d_vgraphics_select_device()
{
    int i;
    VkPhysicalDevice chosen = VK_NULL_HANDLE;
    for (i = 0; i < gf3d_vgraphics.device_count; i++)
    {
        if (gf3d_vgraphics_device_validate(gf3d_vgraphics.devices[i]))
        {
            chosen = gf3d_vgraphics.devices[i];
        }
    }
    return chosen;
}


/**
 * VULKAN DEBUGGING CALLBACK SETUP
 */

static VKAPI_ATTR VkBool32 VKAPI_CALL gf3d_vgraphics_debug_parse(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    //setting this up to always log the message, but this can be adjusted later
    slog("VULKAN DEBUG [%i]:%s",messageSeverity,pCallbackData->pMessage);
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pCallback)
{
    auto PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void gf3d_vgraphics_setup_debug()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    VkDebugUtilsMessengerEXT callback;
    
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    
    createInfo.pfnUserCallback = gf3d_vgraphics_debug_parse;
    createInfo.pUserData = NULL; // Optional
    
    CreateDebugUtilsMessengerEXT(gf3d_vgraphics.vk_instance, &createInfo, NULL, &callback);
}

/*eol@eof*/

