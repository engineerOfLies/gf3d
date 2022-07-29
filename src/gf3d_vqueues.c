#include <stdio.h>
#include <string.h>

#include "simple_logger.h"

#include "gfc_vector.h"

#include "gf3d_vqueues.h"

extern int __DEBUG;

typedef struct
{
    VkQueue                     queue;
    Sint32                      queue_family;
    float                       queue_priority;
}VQueue;

typedef struct
{
    VkPhysicalDevice            physical_device;            /**<this device was used to set up the queues*/
    VkSurfaceKHR                surface;                    /**<presentation is to this surface*/
    Uint32                      queue_family_count;
    VkQueueFamilyProperties    *queue_family_properties;
    Uint32                      work_queue_count;
    VkDeviceQueueCreateInfo    *queue_create_info;          /**<used when the logical device is created*/
    VQueue                      queue_list[VQ_MAX];
}vQueues;

static vQueues gf3d_vqueues = {0};

void gf3d_vqueues_close();
VkDeviceQueueCreateInfo gf3d_vqueues_get_graphics_queue_info();
VkDeviceQueueCreateInfo gf3d_vqueues_get_present_queue_info();
VkDeviceQueueCreateInfo gf3d_vqueues_get_transfer_queue_info();

void gf3d_vqueues_choose_graphics_family()
{
    int i;
    Uint32 bestScore = 0;
    int bestFamily = -1;
    for (i = 0; i < gf3d_vqueues.queue_family_count; i++)
    {
        if (gf3d_vqueues.queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            if (gf3d_vqueues.queue_family_properties[i].queueCount > bestScore)
            {
                bestScore = gf3d_vqueues.queue_family_properties[i].queueCount;
                bestFamily = i;
            }
        }
    }
    if (bestFamily == -1)
    {
        slog("no suitable queue family for graphics operations");
        exit(0);
    }
    gf3d_vqueues.queue_list[VQ_Graphics].queue_family = bestFamily;
}

void gf3d_vqueues_choose_transfer_family()
{
    int i;
    Uint32 bestScore = 0;
    int bestFamily = -1;
    for (i = 0; i < gf3d_vqueues.queue_family_count; i++)
    {
        if (gf3d_vqueues.queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            if (gf3d_vqueues.queue_family_properties[i].queueCount > bestScore)
            {
                bestScore = gf3d_vqueues.queue_family_properties[i].queueCount;
                bestFamily = i;
            }
        }
    }
    if (bestFamily == -1)
    {
        slog("no suitable queue family for transfer operations");
        exit(0);
    }
    gf3d_vqueues.queue_list[VQ_Transfer].queue_family = bestFamily;
}

void gf3d_vqueues_choose_present_family()
{
    int i;
    Uint32 bestScore = 0;
    int bestFamily = -1;
    VkBool32 supported;
    
    for (i = 0; i < gf3d_vqueues.queue_family_count; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(
            gf3d_vqueues.physical_device,
            i,
            gf3d_vqueues.surface,
            &supported);

        if (supported)
        {
            if (gf3d_vqueues.queue_family_properties[i].queueCount > bestScore)
            {
                bestScore = gf3d_vqueues.queue_family_properties[i].queueCount;
                bestFamily = i;
            }
        }
    }
    if (bestFamily == -1)
    {
        slog("no suitable queue family for present operations");
        exit(0);
    }
    gf3d_vqueues.queue_list[VQ_Present].queue_family = bestFamily;
}


void gf3d_vqueues_init(VkPhysicalDevice device,VkSurfaceKHR surface)
{
    Uint32 i;
    VkBool32 supported;

    gf3d_vqueues.queue_list[VQ_Graphics].queue_family = -1;
    gf3d_vqueues.queue_list[VQ_Present].queue_family = -1;
    gf3d_vqueues.queue_list[VQ_Transfer].queue_family = -1;
    
    gf3d_vqueues.physical_device = device;
    gf3d_vqueues.surface = surface;

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
    
    gf3d_vqueues.queue_family_properties = (VkQueueFamilyProperties*)gfc_allocate_array(sizeof(VkQueueFamilyProperties),gf3d_vqueues.queue_family_count);
    
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &gf3d_vqueues.queue_family_count,
        gf3d_vqueues.queue_family_properties);
    
    if (__DEBUG)slog("discovered %i queue families",gf3d_vqueues.queue_family_count);
    for (i = 0; i < gf3d_vqueues.queue_family_count; i++)
    {
        if (__DEBUG)
        {
            slog("Queue family %i:",i);
            slog("queue flag bits %i",gf3d_vqueues.queue_family_properties[i].queueFlags);
            slog("queue count %i",gf3d_vqueues.queue_family_properties[i].queueCount);
            slog("queue timestamp valid bits %i",gf3d_vqueues.queue_family_properties[i].timestampValidBits);
            slog("queue min image transfer granularity %iw %ih %id",
                gf3d_vqueues.queue_family_properties[i].minImageTransferGranularity.width,
                gf3d_vqueues.queue_family_properties[i].minImageTransferGranularity.height,
                gf3d_vqueues.queue_family_properties[i].minImageTransferGranularity.depth);
        }
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &supported);
        if (gf3d_vqueues.queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            if (__DEBUG)slog("Queue handles graphics operations");
        }
        if (gf3d_vqueues.queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            if (__DEBUG)slog("Queue handles transfer operations");
        }
        if (gf3d_vqueues.queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            if (__DEBUG)slog("Queue handles compute operations");
        }
        if (supported)
        {
            if (__DEBUG)slog("Queue handles present operations");
        }
    }
    gf3d_vqueues_choose_graphics_family();
    gf3d_vqueues_choose_present_family();
    gf3d_vqueues_choose_transfer_family();
    if (__DEBUG)
    {
        slog("using queue family %i for graphics commands",gf3d_vqueues.queue_list[VQ_Graphics].queue_family);
        slog("using queue family %i for rendering pipeline",gf3d_vqueues.queue_list[VQ_Present].queue_family);
        slog("using queue family %i for transfer pipeline",gf3d_vqueues.queue_list[VQ_Transfer].queue_family);
    }
    
    if (gf3d_vqueues.queue_list[VQ_Graphics].queue_family != -1)
    {
        gf3d_vqueues.work_queue_count++;
    }
    if ((gf3d_vqueues.queue_list[VQ_Present].queue_family != -1) && (gf3d_vqueues.queue_list[VQ_Present].queue_family != gf3d_vqueues.queue_list[VQ_Graphics].queue_family))
    {
        gf3d_vqueues.work_queue_count++;
    }
    
    if (!gf3d_vqueues.work_queue_count)
    {
        slog("No suitable queues for graphics calls or presentation");
    }
    else
    {
        gf3d_vqueues.queue_create_info = (VkDeviceQueueCreateInfo*)gfc_allocate_array(
            sizeof(VkDeviceQueueCreateInfo),
            gf3d_vqueues.work_queue_count);
        i = 0;
        if (gf3d_vqueues.queue_list[VQ_Graphics].queue_family != -1)
        {
            gf3d_vqueues.queue_create_info[i++] = gf3d_vqueues_get_graphics_queue_info();
        }
        if ((gf3d_vqueues.queue_list[VQ_Present].queue_family != -1) && (gf3d_vqueues.queue_list[VQ_Present].queue_family != gf3d_vqueues.queue_list[VQ_Graphics].queue_family))
        {
            gf3d_vqueues.queue_create_info[i++] = gf3d_vqueues_get_present_queue_info();
        }
    }
    
    atexit(gf3d_vqueues_close);
    slog("vqueues initialized");
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
    queueCreateInfo.queueFamilyIndex = gf3d_vqueues.queue_list[VQ_Graphics].queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &gf3d_vqueues.queue_list[VQ_Graphics].queue_priority;
    return queueCreateInfo;
}

VkDeviceQueueCreateInfo gf3d_vqueues_get_present_queue_info()
{
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = gf3d_vqueues.queue_list[VQ_Present].queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &gf3d_vqueues.queue_list[VQ_Present].queue_priority;
    return queueCreateInfo;
}

VkDeviceQueueCreateInfo gf3d_vqueues_get_transfer_queue_info()
{
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = gf3d_vqueues.queue_list[VQ_Transfer].queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &gf3d_vqueues.queue_list[VQ_Transfer].queue_priority;
    return queueCreateInfo;
}

void gf3d_vqueues_setup_device_queues(VkDevice device)
{
    if (gf3d_vqueues.queue_list[VQ_Graphics].queue_family != -1)
    {
        vkGetDeviceQueue(device, gf3d_vqueues.queue_list[VQ_Graphics].queue_family, 0, &gf3d_vqueues.queue_list[VQ_Graphics].queue);
    }
    if (gf3d_vqueues.queue_list[VQ_Present].queue_family != -1)
    {
        vkGetDeviceQueue(device, gf3d_vqueues.queue_list[VQ_Present].queue_family, 0, &gf3d_vqueues.queue_list[VQ_Present].queue);
    }
    if (gf3d_vqueues.queue_list[VQ_Transfer].queue_family != -1)
    {
        vkGetDeviceQueue(device, gf3d_vqueues.queue_list[VQ_Transfer].queue_family, 0, &gf3d_vqueues.queue_list[VQ_Transfer].queue);
    }
}

void gf3d_vqueues_close()
{
    if (gf3d_vqueues.queue_create_info)
    {
        free(gf3d_vqueues.queue_create_info);
    }
    if (gf3d_vqueues.queue_family_properties)
    {
        free(gf3d_vqueues.queue_family_properties);
    }
    memset(&gf3d_vqueues,0,sizeof(vQueues));
    slog("vqueues closed");
}

Sint32 gf3d_vqueues_get_graphics_queue_family()
{
    return gf3d_vqueues.queue_list[VQ_Graphics].queue_family;
}

Sint32 gf3d_vqueues_get_present_queue_family()
{
    return gf3d_vqueues.queue_list[VQ_Present].queue_family;
}

Sint32 gf3d_vqueues_get_transfer_queue_family()
{
    return gf3d_vqueues.queue_list[VQ_Transfer].queue_family;
}

VkQueue gf3d_vqueues_get_graphics_queue()
{
    return gf3d_vqueues.queue_list[VQ_Graphics].queue;
}

VkQueue gf3d_vqueues_get_present_queue()
{
    return gf3d_vqueues.queue_list[VQ_Present].queue;
}

VkQueue gf3d_vqueues_get_transfer_queue()
{
    return gf3d_vqueues.queue_list[VQ_Transfer].queue;
}
/*eol@eof*/
