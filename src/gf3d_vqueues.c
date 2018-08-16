#include "gf3d_vqueues.h"
#include "gf3d_vector.h"
#include "simple_logger.h"

typedef struct
{
    VkDeviceQueueCreateInfo     queue_info;
    Uint32                      queue_property_count;
    VkQueueFamilyProperties    *queue_properties;
    Uint32                      render_queue_index;
    VkQueue                     device_queue;
}vQueues;

static vQueues gf3d_vqueues = {0};

void gf3d_vqueues_close();

void gf3d_vqueues_init(VkPhysicalDevice device,VkSurfaceKHR surface)
{
    int i;
    VkBool32 supported;
    
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &gf3d_vqueues.queue_property_count,
        NULL);
    
    if (!gf3d_vqueues.queue_property_count)
    {
        slog("failed to get any queue properties");
        gf3d_vqueues_close();
        return;
    }
    
    gf3d_vqueues.queue_properties = (VkQueueFamilyProperties*)gf3d_allocate_array(sizeof(VkQueueFamilyProperties),gf3d_vqueues.queue_property_count);
    
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &gf3d_vqueues.queue_property_count,
        gf3d_vqueues.queue_properties);
    
    slog("discoverd %i queue family properties",gf3d_vqueues.queue_property_count);
    for (i = 0; i < gf3d_vqueues.queue_property_count; i++)
    {
        slog("Queue family %i:",i);
        slog("queue flag bits %i",gf3d_vqueues.queue_properties[i].queueFlags);
        slog("queue count %i",gf3d_vqueues.queue_properties[i].queueCount);
        slog("queue timestamp valid bits %i",gf3d_vqueues.queue_properties[i].timestampValidBits);
        slog("queue min image transfer granularity %iw %ih %id",
             gf3d_vqueues.queue_properties[i].minImageTransferGranularity.width,
             gf3d_vqueues.queue_properties[i].minImageTransferGranularity.height,
             gf3d_vqueues.queue_properties[i].minImageTransferGranularity.depth);
    }
    for (i = 0; i < gf3d_vqueues.queue_property_count; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &supported);
        if (supported)
        {
            gf3d_vqueues.render_queue_index = i;
            slog("can use queue %i for render pipeline",i);
        }
    }
    slog("using queue %i for rendering pipeline",gf3d_vqueues.render_queue_index);

    atexit(gf3d_vqueues_close);
}

void gf3d_vqueues_close()
{
    if (gf3d_vqueues.queue_properties)
    {
        free(gf3d_vqueues.queue_properties);
    }
}

/*eol@eof*/
