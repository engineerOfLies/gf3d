#include <stdio.h>
#include <string.h>

#include "simple_logger.h"
#include "gfc_vector.h"

#include "gf3d_vqueues.h"

/*
 * -- Theory --
 *
 * Within Vulkan, and computer science at large, there is a concept called a "Queue". You might have heard the term before if you've ever talked to a
 * cranky British person complaining about the line at a shopping center. A "Queue", as a concept, is a temporary storage space for a thing that is
 * waiting to be processed. There are many different types of Queues. A non-exhaustive list of types of Queues that you *should* know the existence
 * of: First-In-First-Out, Last-In-First-Out, and Priority.
 *
 * You use a Queue to separate the production of work from the completion of work. This prevents a situation called "Blocking" where a component (A) has a
 * computation or piece of information it wants to pass to another component (B) but that other component (B) is busy doing other operations. In this case
 * the first component (A) has to wait until the second component (B) is done with whatever it is working on before it can hand off it's work item. It is a
 * really ugly case of hot-potato. If, however, a Queue is in between your two components (A & B) the Queue can immediately take the hot-potato from component
 * A and when B is finished it can just take the work already stored in the Queue. It is important to note that this is only useful if A and B have about the
 * same throughput over a long amount of time or if you are ok with dropping work (some expiration).
 *
 * In a similar way to how the line at your local DMV might look different from the line at your local grocery store different queues are designed with
 * different use cases in mind. The engine the Queue is sitting in front, and the usage patterns of things that are put into the Queue, dictates how
 * fast, what priority, and how complex the Queue must be.
 *
 * The GPU your graphics driver stack is talking, and the software that is talking to those drivers, are unimaginably more "complicated" then the DMV or your
 * local ShopRight. Because of this the Vulkan spec allows driver writers and card manufacturers to build multiple queues fo different tasks between your
 * software, the driver, and the hardware. They expose this concept with the aptly named VkQueue. Each Queue has "Physical Properties" that describe exactly
 * what the hardware behind this queue can actually do. Example: A VkQueue might only be able to read and write to memory but might have absolutely no
 * connection to the display memory of your GPU (Surface). This VkQueue would be able to transfer between CPU and GPU memory but NOT be able to ask the GPU to
 * render a frame.
 *
 * In an ideal world, where all computers are infinitely fast, this is unnecessarily complex. This is purely a performance mitigation and we can see why this
 * is so important if we relate our graphics software to real life Queues that exist in our day to day life.
 *
 * If we stick to the ShopRite example: If you do your shopping in the middle of the night so you can minimize human interaction, as well adjusted people like
 * myself do, and you go to checkout there will likely be 1 lane and you will be the only one trying to check out. In this case there is effectively no Queue
 * because the production of work (you attempting to check out) exactly matches the capacity of the worker (the cashier).
 *
 * If instead of 1 lane there were 4 open lanes: you now likely stood in front of the lanes, choose a lane a random, and went to check out with the cashier
 * at the lane you choose. You might not have chosen the fastest lane but you "Queued" up and went to a single worker. This is an example of distributing a
 * job across multiple workers. The Queue allows you to "fan-out" the number of workers behind it.
 *
 * And if instead of shopping during the middle of the night: you likely arrived at checkout with a non-zero amount of people (lets say 10) who have finished
 * shopping at the same time as you. You all form a Queue and get dispatched to different workers. The workers don't (ideally) work faster or slower based on
 * how backed up the Queue is. This is because work is coming in as batches (your batch was ~11 people) and as long as the workers process all of you before
 * the next batch shows up all is well.
 *
 * (*warning* *warning* *warning*: major simplification)
 *
 * Now your GPU: A GPU can represent anywhere from 1 worker to 1000s of workers depending on design. Your graphics software also creates extremely peaky
 * demands. If you're rendering your software at 60 frames per second your game can, at max, take ~15ms/frame to update it's internal state and shovel a HUGE
 * number of updates to the GPU after that is done (render this, this memory changed, load this new shader, load this new texture, run this compute shader,
 * etc). After the GPU gets & finishes that work it just sits idle until more work comes it's way. This is very similar to our shopping situation.
 *
 * -- Implementation --
 *
 * The VkPhysicalDevice contains a set number of provisionable slots for VkQueues to be allocated to. These slots have different
 * VkQueueFamilyProperties which describe what each slot in the VkPhysicalDevice is capable of doing. This portion of the code has
 * two main objectives:
 *  GOAL 1. Create VkDeviceQueueCreateInfo packets that specifies how our VkDevice interact with these these queue families
 *  GOAL 2. Retrieve instances of the VkQueue from the VkDevice when it is allocated
 *  GOAL 3. Keep track of what each will a VkQueue we are going to use for what task (based on what features it supports)
 */

/**
 * gf3d wrapper for Vulkan VkQueue that contains metadata useful to us.
 */
typedef struct
{
    /**
     * Track if this VkQueue can be used for graphics (shader stuff), presentation (drawing to a surface), or transfer (memcpy) work.
     */
    VkBool32 graphics, present, transfer;
    /**
     * Pointer to a flat array of VkDeviceQueueCreationInfo structs.
     */
    VkDeviceQueueCreateInfo *creation_info;
    /**
     * Priority of this queue. We have a really simple application so ours will always be set to 1.0f.
     */
    float priority;
    /**
     * The ID of the Vulakn Queue Family. Vulkan docs will likely call this a "queueFamilyIndex"
     */
    Sint32 family;

    /**
     * Vulkan's application-level abstraction of what a "Queue" is.
     */
    VkQueue queue;
} vQueue;


typedef struct
{
    /**
     * The number of available queue families on this card. We currently make use of every single one of
     * them so this is also the number of VkQueue objects we will allocate. This is mainly done because
     * we are lazy and don't care about performance.
     */
    Uint32                      num_available_queue_families;

    /**
     * Information about what each queue family can do.
     *
     * This information is polled from the VkPhysicalDevice using vkGetPhysicalDeviceQueueFamilyProperties.
     */
    VkQueueFamilyProperties    *queue_properties;
    /**
     * Information used when creating a VkDeice.
     */
    VkDeviceQueueCreateInfo    *creation_infos;
    vQueue                     *queues;

    // Pointers to the different vQueues we use for different work.
    vQueue                     *graphics_queue;
    vQueue                     *present_queue;
    vQueue                     *transfer_queue;
} vQueues;

static vQueues gf3d_vqueues = {0};

void gf3d_vqueues_close();

/**
 * Configure a vQueue wrapper with information polled from Vulkan.
 * @param device - Physical device that has this queue attached
 * @param surface - Surface we want to present to
 * @param properties - What this queue is wired up to
 * @param creation_info - Where to store information for VkDevice configuration process
 * @param queue - The Queue itself
 * @return true if everything went well. false if there was an issue talking to the GPU/Driver
 */
Bool gf3d_vqueues_init_configure_vqueue(VkPhysicalDevice device, VkSurfaceKHR surface, VkQueueFamilyProperties *properties, VkDeviceQueueCreateInfo *creation_info, vQueue *queue)
{
    if (!properties) {
        slog("attempted to initialize vqueue from NULL VkQueueFamilyProperties");
        return false;
    }

    if (!queue) {
        slog("attempted to initialize vqueue into NULL");
        return false;
    }

    // Make if this queue supports present, graphics, and transfer
    if (vkGetPhysicalDeviceSurfaceSupportKHR(device, queue->family, surface, &queue->present) != VK_SUCCESS) {
        slog("failed to query if queue family %d could present to a surface.", queue->family);
        return false;
    }
    queue->graphics = properties->queueFlags & VK_QUEUE_GRAPHICS_BIT;
    queue->transfer = properties->queueFlags & VK_QUEUE_TRANSFER_BIT;

    // GOAL 1: Make a VkDeviceQueueCreationInfo packet for vulkan
    queue->creation_info = creation_info;
    memset(creation_info, 0, sizeof(VkDeviceQueueCreateInfo));
    {
        creation_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        creation_info->queueFamilyIndex = queue->family;
        creation_info->queueCount = 1;
        creation_info->pQueuePriorities = &queue->priority;
    }

    slog("queue family %d supports:");
    slog("\t present: %d", (queue->present != 0));
    slog("\ttransfer: %d", (queue->transfer != 0));
    slog("\t    draw: %d", (queue->graphics != 0));
    return true;
}

/**
 * Choose a vQueue for each queue task (graphics, transfer, present) based upon what it internally supports.
 */
void gf3d_vqueues_init_vqueue_assign_to_tasks()
{
    // GOAL 3: Assign a vQueue to each task we need to complete.
    for (Uint32 i = 0; i < gf3d_vqueues.num_available_queue_families; i++)
    {
        vQueue *queue = &gf3d_vqueues.queues[i];
        if (queue->graphics) {
            gf3d_vqueues.graphics_queue = queue;
        }

        if (queue->transfer) {
            gf3d_vqueues.transfer_queue = queue;
        }

        if (queue->present) {
            gf3d_vqueues.present_queue = queue;
        }
    }

    slog("using queue family %i for graphics commands", gf3d_vqueues.graphics_queue->family);
    slog("using queue family %i for rendering pipeline", gf3d_vqueues.present_queue->family);
    slog("using queue family %i for transfer pipeline", gf3d_vqueues.transfer_queue->family);
}

void gf3d_vqueues_init(VkPhysicalDevice device,VkSurfaceKHR surface)
{
    // Find number of physical queues available on this device. This varies from card to card.
    vkGetPhysicalDeviceQueueFamilyProperties(device, &gf3d_vqueues.num_available_queue_families, NULL);
    slog("device reported queue family count: %i", gf3d_vqueues.num_available_queue_families);
    if (!gf3d_vqueues.num_available_queue_families)
    {
        slog("failed to get any queue properties");
        gf3d_vqueues_close();
        return;
    }

    // Load all metadata about these queues from the card
    gf3d_vqueues.queue_properties = (VkQueueFamilyProperties *) gfc_allocate_array(sizeof(VkQueueFamilyProperties), gf3d_vqueues.num_available_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &gf3d_vqueues.num_available_queue_families, gf3d_vqueues.queue_properties);

    // Allocate a vQueue for each of these queue families
    gf3d_vqueues.creation_infos = (VkDeviceQueueCreateInfo *) gfc_allocate_array(sizeof(VkDeviceQueueCreateInfo), gf3d_vqueues.num_available_queue_families);
    gf3d_vqueues.queues = (vQueue *) gfc_allocate_array(sizeof(vQueue), gf3d_vqueues.num_available_queue_families);
    for (Uint32 i = 0; i < gf3d_vqueues.num_available_queue_families; i++)
    {
        VkDeviceQueueCreateInfo *creation_info = &gf3d_vqueues.creation_infos[i];
        vQueue *queue = &gf3d_vqueues.queues[i];
        VkQueueFamilyProperties *properties = &gf3d_vqueues.queue_properties[i];
        slog("Queue family %i:", i);
        slog("queue flag bits %i", properties->queueFlags);
        slog("queue count %i", properties->queueCount);
        slog("queue timestamp valid bits %i", properties->timestampValidBits);
        slog("queue min image transfer granularity %iw %ih %id",
             properties->minImageTransferGranularity.width,
             properties->minImageTransferGranularity.height,
             properties->minImageTransferGranularity.depth);
        queue->family = i;
        queue->priority = 1.0f;
        gf3d_vqueues_init_configure_vqueue(device, surface, properties, creation_info, &gf3d_vqueues.queues[i]);

    }

    gf3d_vqueues_init_vqueue_assign_to_tasks();

    atexit(gf3d_vqueues_close);
}

const VkDeviceQueueCreateInfo *gf3d_vqueues_get_queue_create_info(Uint32 *count)
{
    if (count)*count = gf3d_vqueues.num_available_queue_families;
    return gf3d_vqueues.creation_infos;
}

void gf3d_vqueues_close()
{
    slog("cleaning up vulkan queues");
    memset(&gf3d_vqueues,0,sizeof(vQueues));
}

void gf3d_vqueues_setup_device_queues(VkDevice device)
{
    // GOAL 2: populate our internal pointers with reference to VkQueue. This allows dependencies on these pointers
    // to be injected into other components of the application. See the below functions and their usages.
    for (Uint32 i = 0; i < gf3d_vqueues.num_available_queue_families; i++) {
        vQueue *queue = &gf3d_vqueues.queues[i];
        vkGetDeviceQueue(device, queue->family, 0, &queue->queue);
    }
}

Sint32 gf3d_vqueues_get_graphics_queue_family()
{
    return gf3d_vqueues.graphics_queue->family;
}

Sint32 gf3d_vqueues_get_present_queue_family()
{
    return gf3d_vqueues.present_queue->family;
}

Sint32 gf3d_vqueues_get_transfer_queue_family()
{
    return gf3d_vqueues.transfer_queue->family;
}

VkQueue gf3d_vqueues_get_graphics_queue()
{
    return gf3d_vqueues.graphics_queue->queue;
}

VkQueue gf3d_vqueues_get_present_queue()
{
    return gf3d_vqueues.present_queue->queue;
}

VkQueue gf3d_vqueues_get_transfer_queue()
{
    return gf3d_vqueues.transfer_queue->queue;
}
/*eol@eof*/
