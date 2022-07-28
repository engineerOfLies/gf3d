#include <string.h>

#include "simple_logger.h"

#include "gf3d_debug.h"
/**
 * VULKAN DEBUGGING CALLBACK SETUP
 */

typedef struct
{
    VkDebugUtilsMessengerEXT   debug_callback;
    VkInstance instance;
}GF3D_Debug;

static GF3D_Debug gf3d_debug = {0};


static VKAPI_ATTR VkBool32 VKAPI_CALL gf3d_debug_parse(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    //setting this up to always log the message, but this can be adjusted later
    slog("VULKAN DEBUG [%i]:%s",messageSeverity,pCallbackData->pMessage);
	slog_sync();
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

void gf3d_debug_close()
{
    if (gf3d_debug.debug_callback)
    {
        DestroyDebugUtilsMessengerEXT(gf3d_debug.instance, gf3d_debug.debug_callback, NULL);
    }
    memset(&gf3d_debug,0,sizeof(GF3D_Debug));
}

void gf3d_debug_setup(VkInstance instance)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};

    gf3d_debug.instance = instance;
    
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    
    createInfo.pfnUserCallback = gf3d_debug_parse;
    createInfo.pUserData = NULL; // Optional
    
    CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL, &gf3d_debug.debug_callback);
}

/*eol@eof*/
