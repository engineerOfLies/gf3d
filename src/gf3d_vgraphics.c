/**
 * @purpose vulkan graphics setup and abstraction
*/

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "gf3d_vector.h"
#include "gf3d_types.h"
#include "gf3d_validation.h"
#include "gf3d_extensions.h"
#include "gf3d_vqueues.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"

#include "simple_logger.h"

typedef struct
{
    SDL_Window                 *main_window;

    VkApplicationInfo           vk_app_info;

    VkInstance                  vk_instance;
    VkInstanceCreateInfo        vk_instance_info;

    Uint32                      sdl_extension_count;
    const char                **sdl_extension_names;
    Bool                        enableValidationLayers;
    VkDebugUtilsMessengerEXT    debug_callback;    
    
    unsigned int                enabled_layer_count;

    //devices
    Uint32                      device_count;
    VkPhysicalDevice           *devices;
    VkPhysicalDevice            gpu;
    Bool                        logicalDeviceCreated;
    
    VkDevice                    device;
    VkSurfaceKHR                surface;

    // color space
    VkFormat                    color_format;
    VkColorSpaceKHR             color_space;

    VkDeviceQueueCreateInfo    *queueCreateInfo;
    VkPhysicalDeviceFeatures    deviceFeatures;
    
    VkSemaphore                 imageAvailableSemaphore;
    VkSemaphore                 renderFinishedSemaphore;
    
    Pipeline                   *pipe;
}vGraphics;

static vGraphics gf3d_vgraphics = {0};

void gf3d_vgraphics_close();
void gf3d_vgraphics_logical_device_close();
void gf3d_vgraphics_extension_init();
void gf3d_vgraphics_setup_debug();
void gf3d_vgraphics_semaphores_create();
VkPhysicalDevice gf3d_vgraphics_select_device();
VkDeviceCreateInfo gf3d_vgraphics_get_device_info(Bool enableValidationLayers);
void gf3d_vgraphics_debug_close();
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);
void gf3d_vgraphics_setup(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    Bool enableValidation
);

void gf3d_vgraphics_init(
    char *windowName,
    int renderWidth,
    int renderHeight,
    Vector4D bgcolor,
    Bool fullscreen,
    Bool enableValidation
)
{
    VkDevice device;

    gf3d_vgraphics_setup(
        windowName,
        renderWidth,
        renderHeight,
        bgcolor,
        fullscreen,
        enableValidation);
    
    device = gf3d_vgraphics_get_default_logical_device();
    
    gf3d_pipeline_init(2);
    
    gf3d_vgraphics.pipe = gf3d_pipeline_graphics_load(device,"shaders/vert.spv","shaders/frag.spv",gf3d_vgraphics_get_view_extent());

    gf3d_swapchain_setup_frame_buffers(gf3d_vgraphics.pipe);

    gf3d_command_pool_setup(device,gf3d_swapchain_get_frame_buffer_count(),gf3d_vgraphics.pipe);
    
    gf3d_vgraphics_semaphores_create();
}


void gf3d_vgraphics_setup(
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
    VkDeviceCreateInfo createInfo = {0};
    
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

    // instance extension configuration
    gf3d_extensions_instance_init();
    
    // get the extensions that are needed for rendering to an SDL Window
    SDL_Vulkan_GetInstanceExtensions(gf3d_vgraphics.main_window, &(gf3d_vgraphics.sdl_extension_count), NULL);
    if (gf3d_vgraphics.sdl_extension_count > 0)
    {
        gf3d_vgraphics.sdl_extension_names = gf3d_allocate_array(sizeof(const char *),gf3d_vgraphics.sdl_extension_count);
        
        SDL_Vulkan_GetInstanceExtensions(gf3d_vgraphics.main_window, &(gf3d_vgraphics.sdl_extension_count), gf3d_vgraphics.sdl_extension_names);
        for (i = 0; i < gf3d_vgraphics.sdl_extension_count;i++)
        {
            slog("SDL Vulkan extensions support: %s",gf3d_vgraphics.sdl_extension_names[i]);
            gf3d_extensions_enable(ET_Instance, gf3d_vgraphics.sdl_extension_names[i]);
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
        gf3d_vgraphics.enableValidationLayers = true;
        gf3d_validation_init();
        gf3d_vgraphics.vk_instance_info.enabledLayerCount = gf3d_validation_get_validation_layer_count();
        gf3d_vgraphics.vk_instance_info.ppEnabledLayerNames = gf3d_validation_get_validation_layer_names();
        gf3d_extensions_enable(ET_Instance,"VK_EXT_debug_utils");
    }
    else
    {
        //setup instance info
        gf3d_vgraphics.vk_instance_info.enabledLayerCount = 0;
        gf3d_vgraphics.vk_instance_info.ppEnabledLayerNames = NULL;
    }
    
    gf3d_vgraphics.vk_instance_info.ppEnabledExtensionNames = gf3d_extensions_get_instance_enabled_names(&enabledExtensionCount);
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
    atexit(gf3d_vgraphics_close);
    
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
    if(!gf3d_vgraphics.gpu){
        slog("Failed to select graphics card. If using integrated graphics, change variable in h file.");
        gf3d_vgraphics_close();
        return;
    }
    
    // create a surface for the window
    SDL_Vulkan_CreateSurface(gf3d_vgraphics.main_window, gf3d_vgraphics.vk_instance, &gf3d_vgraphics.surface);
    // setup a queue for rendering calls
        
    // setup queues
    gf3d_vqueues_init(gf3d_vgraphics.gpu,gf3d_vgraphics.surface);
    
    //setup device extensions
    gf3d_extensions_device_init(gf3d_vgraphics.gpu);
    gf3d_extensions_enable(ET_Device,"VK_KHR_swapchain");

    createInfo = gf3d_vgraphics_get_device_info(enableValidation);
    
    if (vkCreateDevice(gf3d_vgraphics.gpu, &createInfo, NULL, &gf3d_vgraphics.device) != VK_SUCCESS)
    {
        slog("failed to create logical device");
        gf3d_vgraphics_close();
        return;
    }
    gf3d_vgraphics.logicalDeviceCreated = true;
    
    gf3d_vqueues_setup_device_queues(gf3d_vgraphics.device);

    // swap chain!!!
    gf3d_swapchain_init(gf3d_vgraphics.gpu,gf3d_vgraphics.device,gf3d_vgraphics.surface,renderWidth,renderHeight);
}

void gf3d_vgraphics_close()
{
    slog("cleaning up vulkan graphics");
    gf3d_vgraphics_debug_close();
    if (gf3d_vgraphics.logicalDeviceCreated)
    {
        vkDestroyDevice(gf3d_vgraphics.device, NULL);
    }
    if (gf3d_vgraphics.devices)
    {
        free(gf3d_vgraphics.devices);
    }
    if (gf3d_vgraphics.sdl_extension_names)
    {
        free(gf3d_vgraphics.sdl_extension_names);
    }
    if(gf3d_vgraphics.surface && gf3d_vgraphics.vk_instance)
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
    memset(&gf3d_vgraphics,0,sizeof(vGraphics));
}

VkDevice gf3d_vgraphics_get_default_logical_device()
{
    return gf3d_vgraphics.device;
}

VkExtent2D gf3d_vgraphics_get_view_extent()
{
    return gf3d_swapchain_get_extent();
}


VkDeviceCreateInfo gf3d_vgraphics_get_device_info(Bool enableValidationLayers)
{
    VkDeviceCreateInfo createInfo = {0};
    Uint32 count;
    
    gf3d_vgraphics.queueCreateInfo = (VkDeviceQueueCreateInfo *)gf3d_vqueues_get_queue_create_info(&count);

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    createInfo.pQueueCreateInfos = gf3d_vgraphics.queueCreateInfo;
    createInfo.queueCreateInfoCount = count;

    createInfo.pEnabledFeatures = &gf3d_vgraphics.deviceFeatures;
    
    
    createInfo.ppEnabledExtensionNames = gf3d_extensions_get_device_enabled_names(&count);
    createInfo.enabledExtensionCount = count;
    

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = gf3d_validation_get_validation_layer_count();
        createInfo.ppEnabledLayerNames = gf3d_validation_get_validation_layer_names();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    
    return createInfo;
}

void gf3d_vgraphics_clear()
{
    
}

void gf3d_vgraphics_render()
{
    Uint32 imageIndex;
    VkPresentInfoKHR presentInfo = {0};
    VkSubmitInfo submitInfo = {0};
    VkSemaphore waitSemaphores[] = {gf3d_vgraphics.imageAvailableSemaphore};
    VkSemaphore signalSemaphores[] = {gf3d_vgraphics.renderFinishedSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSwapchainKHR swapChains[1] = {0};

    /*
    Acquire an image from the swap chain
    Execute the command buffer with that image as attachment in the framebuffer
    Return the image to the swap chain for presentation
    */
    swapChains[0] = gf3d_swapchain_get();
    
    vkAcquireNextImageKHR(
        gf3d_vgraphics.device,
        swapChains[0],
        UINT_MAX,
        gf3d_vgraphics.imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex);
    
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = gf3d_command_buffer_get_by_index(imageIndex);
    
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(gf3d_vqueues_get_graphics_queue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        slog("failed to submit draw command buffer!");
    }
    
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL; // Optional
    
    vkQueuePresentKHR(gf3d_vqueues_get_present_queue(), &presentInfo);
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
    return (deviceProperties.deviceType == GF3D_VGRAPHICS_DISCRETE)&&(deviceFeatures.geometryShader);
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

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
    auto PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func(instance, callback, pAllocator);
    }
}

void gf3d_vgraphics_debug_close()
{
    if (gf3d_vgraphics.enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(gf3d_vgraphics.vk_instance, gf3d_vgraphics.debug_callback, NULL);
    }
}

void gf3d_vgraphics_setup_debug()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};

    
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    
    createInfo.pfnUserCallback = gf3d_vgraphics_debug_parse;
    createInfo.pUserData = NULL; // Optional
    
    CreateDebugUtilsMessengerEXT(gf3d_vgraphics.vk_instance, &createInfo, NULL, &gf3d_vgraphics.debug_callback);
}

void gf3d_vgraphics_semaphores_close()
{
    vkDestroySemaphore(gf3d_vgraphics.device, gf3d_vgraphics.renderFinishedSemaphore, NULL);
    vkDestroySemaphore(gf3d_vgraphics.device, gf3d_vgraphics.imageAvailableSemaphore, NULL);
    
}

void gf3d_vgraphics_semaphores_create()
{
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    if ((vkCreateSemaphore(gf3d_vgraphics.device, &semaphoreInfo, NULL, &gf3d_vgraphics.imageAvailableSemaphore) != VK_SUCCESS) ||
        (vkCreateSemaphore(gf3d_vgraphics.device, &semaphoreInfo, NULL, &gf3d_vgraphics.renderFinishedSemaphore) != VK_SUCCESS))
    {
        slog("failed to create semaphores!");
    }
    atexit(gf3d_vgraphics_semaphores_close);
}
/*eol@eof*/

