#include <stdio.h>
#include <string.h>

#include "simple_logger.h"
#include "gfc_vector.h"

#include "gf3d_vqueues.h"


typedef struct
{
    VkDeviceQueueCreateInfo     queue_info;
    Uint32                      queue_family_count;
    VkQueueFamilyProperties    *queue_properties;
    VkQueue                     device_queue;
    Sint32                      graphics_queue_family;
    Sint32                      present_queue_family;
    Sint32                      transfer_queue_family;
    float                       graphics_queue_priority;
    float                       present_queue_priority;
    float                       transfer_queue_priority;
    Uint32                      work_queue_count;
    VkQueue                     graphics_queue;
    VkQueue                     present_queue;
    VkQueue                     transfer_queue;
    VkDeviceQueueCreateInfo    *presentation_queue_info;
    VkDeviceQueueCreateInfo    *queue_create_info;
    VkDeviceQueueCreateInfo    *transfer_queue_info;
}vQueues;

static vQueues gf3d_vqueues = {0};

void gf3d_vqueues_close();
VkDeviceQueueCreateInfo gf3d_vqueues_get_graphics_queue_info();
VkDeviceQueueCreateInfo gf3d_vqueues_get_present_queue_info();
VkDeviceQueueCreateInfo gf3d_vqueues_get_transfer_queue_info();

void gf3d_vqueues_init(VkPhysicalDevice device,VkSurfaceKHR surface)
{
    Uint32 i;
    VkBool32 supported;

    gf3d_vqueues.graphics_queue_family = -1;
    gf3d_vqueues.present_queue_family = -1;
    gf3d_vqueues.transfer_queue_family = -1;
    
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &gf3d_vqueues.queue_family_count,
        NULL);
    
    if (!gf3d_vqueues.queue_family_count)
    {
        slog("failed to get any queue properties");
        gf3d_vqueues_close();
        return;
    }
    
    gf3d_vqueues.queue_properties = (VkQueueFamilyProperties*)gfc_allocate_array(sizeof(VkQueueFamilyProperties),gf3d_vqueues.queue_family_count);
    
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &gf3d_vqueues.queue_family_count,
        gf3d_vqueues.queue_properties);
    
    slog("discoverd %i queue family properties",gf3d_vqueues.queue_family_count);
    for (i = 0; i < gf3d_vqueues.queue_family_count; i++)
    {
        slog("Queue family %i:",i);
        slog("queue flag bits %i",gf3d_vqueues.queue_properties[i].queueFlags);
        slog("queue count %i",gf3d_vqueues.queue_properties[i].queueCount);
        slog("queue timestamp valid bits %i",gf3d_vqueues.queue_properties[i].timestampValidBits);
        slog("queue min image transfer granularity %iw %ih %id",
             gf3d_vqueues.queue_properties[i].minImageTransferGranularity.width,
             gf3d_vqueues.queue_properties[i].minImageTransferGranularity.height,
             gf3d_vqueues.queue_properties[i].minImageTransferGranularity.depth);
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &supported);
        if (gf3d_vqueues.queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            gf3d_vqueues.graphics_queue_family = i;
            gf3d_vqueues.graphics_queue_priority = 1.0f;
            slog("Queue handles graphics calls");
        }
        if (gf3d_vqueues.queue_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            gf3d_vqueues.transfer_queue_family = i;
            gf3d_vqueues.transfer_queue_priority = 1.0f;
            slog("Queue handles transfer calls");
        }
        if (supported)
        {
            gf3d_vqueues.present_queue_family = i;
            gf3d_vqueues.present_queue_priority = 1.0f;
            slog("Queue handles present calls");
        }
    }
    slog("using queue family %i for graphics commands",gf3d_vqueues.graphics_queue_family);
    slog("using queue family %i for rendering pipeline",gf3d_vqueues.present_queue_family);
    slog("using queue family %i for transfer pipeline",gf3d_vqueues.transfer_queue_family);
    
    if (gf3d_vqueues.graphics_queue_family != -1)
    {
        gf3d_vqueues.work_queue_count++;
    }
    if ((gf3d_vqueues.present_queue_family != -1) && (gf3d_vqueues.present_queue_family != gf3d_vqueues.graphics_queue_family))
    {
        gf3d_vqueues.work_queue_count++;
    }
    
    if (!gf3d_vqueues.work_queue_count)
    {
        slog("No suitable queues for graphics calls or presentation");
    }
    else
    {
        gf3d_vqueues.queue_create_info = (VkDeviceQueueCreateInfo*)gfc_allocate_array(sizeof(VkDeviceQueueCreateInfo),gf3d_vqueues.work_queue_count);
        slog("work queue count: %i",gf3d_vqueues.work_queue_count);
        i = 0;
        if (gf3d_vqueues.graphics_queue_family != -1)
        {
            gf3d_vqueues.queue_create_info[i++] = gf3d_vqueues_get_graphics_queue_info();
        }
        if ((gf3d_vqueues.present_queue_family != -1) && (gf3d_vqueues.present_queue_family != gf3d_vqueues.graphics_queue_family))
        {
            gf3d_vqueues.queue_create_info[i++] = gf3d_vqueues_get_present_queue_info();
        }
    }
    
    atexit(gf3d_vqueues_close);
}

const VkDeviceQueueCreateInfo *gf3d_vqueues_get_queue_create_info(Uint32 *count)
{
    if (count)*count = gf3d_vqueues.work_queue_count;
    return gf3d_vqueues.queue_create_info;
}

VkDeviceQueueCreateInfo gf3d_vqueues_get_graphics_queue_info()
{
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = gf3d_vqueues.graphics_queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &gf3d_vqueues.graphics_queue_priority;
    return queueCreateInfo;
}

VkDeviceQueueCreateInfo gf3d_vqueues_get_present_queue_info()
{
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = gf3d_vqueues.present_queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &gf3d_vqueues.present_queue_priority;
    return queueCreateInfo;
}

VkDeviceQueueCreateInfo gf3d_vqueues_get_transfer_queue_info()
{
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = gf3d_vqueues.transfer_queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &gf3d_vqueues.transfer_queue_priority;
    return queueCreateInfo;
}

void gf3d_vqueues_setup_device_queues(VkDevice device)
{
    if (gf3d_vqueues.graphics_queue_family != -1)
    {
        vkGetDeviceQueue(device, gf3d_vqueues.graphics_queue_family, 0, &gf3d_vqueues.graphics_queue);
    }
    if (gf3d_vqueues.present_queue_family != -1)
    {
        vkGetDeviceQueue(device, gf3d_vqueues.present_queue_family, 0, &gf3d_vqueues.present_queue);
    }
    if (gf3d_vqueues.transfer_queue_family != -1)
    {
        vkGetDeviceQueue(device, gf3d_vqueues.transfer_queue_family, 0, &gf3d_vqueues.transfer_queue);
    }
}

void gf3d_vqueues_close()
{
    slog("cleaning up vulkan queues");
    if (gf3d_vqueues.queue_create_info)
    {
        free(gf3d_vqueues.queue_create_info);
    }
    if (gf3d_vqueues.queue_properties)
    {
        free(gf3d_vqueues.queue_properties);
    }
    if (gf3d_vqueues.presentation_queue_info)
    {
        free(gf3d_vqueues.presentation_queue_info);
    }
    memset(&gf3d_vqueues,0,sizeof(vQueues));
}

void gf3d_vqueues_create_presentation_queues()
{
    gf3d_vqueues.presentation_queue_info = (VkDeviceQueueCreateInfo*)gfc_allocate_array(sizeof(VkDeviceQueueCreateInfo),gf3d_vqueues.queue_family_count);
}

Sint32 gf3d_vqueues_get_graphics_queue_family()
{
    return gf3d_vqueues.graphics_queue_family;
}

Sint32 gf3d_vqueues_get_present_queue_family()
{
    return gf3d_vqueues.present_queue_family;
}

Sint32 gf3d_vqueues_get_transfer_queue_family()
{
    return gf3d_vqueues.transfer_queue_family;
}

VkQueue gf3d_vqueues_get_graphics_queue()
{
    return gf3d_vqueues.graphics_queue;
}

VkQueue gf3d_vqueues_get_present_queue()
{
    return gf3d_vqueues.present_queue;
}

VkQueue gf3d_vqueues_get_transfer_queue()
{
    return gf3d_vqueues.transfer_queue;
}
/*eol@eof*/
