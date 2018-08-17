#include "gf3d_vqueues.h"
#include "gf3d_vector.h"
#include "simple_logger.h"

typedef struct
{
    VkDeviceQueueCreateInfo     queue_info;
    Uint32                      queue_family_count;
    VkQueueFamilyProperties    *queue_properties;
    VkQueue                     device_queue;
    Uint32                      graphics_queue_family;
    float                       queue_priority;
    VkQueue                     graphics_queue;
    VkQueue                     present_queue;
    VkDeviceQueueCreateInfo    *presentation_queue_info;
}vQueues;

static vQueues gf3d_vqueues = {0};

void gf3d_vqueues_close();

void gf3d_vqueues_init(VkPhysicalDevice device,VkSurfaceKHR surface)
{
    int i;
    VkBool32 supported;
    Bool graphics_bit_enabled;
    
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
    
    gf3d_vqueues.queue_properties = (VkQueueFamilyProperties*)gf3d_allocate_array(sizeof(VkQueueFamilyProperties),gf3d_vqueues.queue_family_count);
    
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &gf3d_vqueues.queue_family_count,
        gf3d_vqueues.queue_properties);
    
    slog("discoverd %i queue family properties",gf3d_vqueues.queue_family_count);
    for (i = 0; i < gf3d_vqueues.queue_family_count; i++)
    {
        slog("Queue family %i:",i);
        slog("queue flag bits %i",gf3d_vqueues.queue_properties[i].queueFlags);
        slog("Queue handles graphics calls: %i",gf3d_vqueues.queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
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
        graphics_bit_enabled = (gf3d_vqueues.queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
        if (supported && graphics_bit_enabled)
        {
            gf3d_vqueues.graphics_queue_family = i;
            gf3d_vqueues.queue_priority = 1.0;
        }
    }
    slog("using queue %i for rendering pipeline",gf3d_vqueues.graphics_queue_family);

    atexit(gf3d_vqueues_close);
}

VkDeviceQueueCreateInfo gf3d_vqueues_get_graphics_queue_info()
{
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = gf3d_vqueues.graphics_queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &gf3d_vqueues.queue_priority;
    return queueCreateInfo;
}

void gf3d_vqueues_set_graphics_queue(VkQueue graphicsQueue)
{
    gf3d_vqueues.graphics_queue = graphicsQueue;
}

void gf3d_vqueues_close()
{
    if (gf3d_vqueues.queue_properties)
    {
        free(gf3d_vqueues.queue_properties);
    }
}

void gf3d_vqueues_create_presentation_queues()
{
    gf3d_vqueues.presentation_queue_info = (VkDeviceQueueCreateInfo*)gf3d_allocate_array(sizeof(VkDeviceQueueCreateInfo),gf3d_vqueues.queue_family_count);
}
/*eol@eof*/
