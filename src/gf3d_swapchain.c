#include <string.h>
#include <stdio.h>

#include "simple_logger.h"

#include "gf3d_swapchain.h"
#include "gf3d_vqueues.h"
#include "gf3d_vgraphics.h"


typedef struct
{
    VkDevice                    device;
    VkSurfaceCapabilitiesKHR    capabilities;
    Uint32                      formatCount;
    VkSurfaceFormatKHR         *formats;
    Uint32                      presentModeCount;
    VkPresentModeKHR           *presentModes;
    int                         chosenFormat;
    int                         chosenPresentMode;
    VkExtent2D                  extent;                 // resolution of the swap buffers
    Uint32                      swapChainCount;
    VkSwapchainKHR              swapChain;
    VkImage                    *swapImages;
    Uint32                      swapImageCount;
    VkImageView                *imageViews;
    VkFramebuffer              *frameBuffers;
    Uint32                      framebufferCount;
    VkImage                     depthImage;
    VkDeviceMemory              depthImageMemory;
    VkImageView                 depthImageView;
}vSwapChain;

static vSwapChain gf3d_swapchain = {0};

void gf3d_swapchain_create(VkDevice device,VkSurfaceKHR surface);
void gf3d_swapchain_close();
int gf3d_swapchain_choose_format();
void gf3d_swapchain_create_depth_image();
int gf3d_swapchain_get_presentation_mode();
VkExtent2D gf3d_swapchain_configure_extent(Uint32 width,Uint32 height);
uint32_t gf3d_swapchain_find_Memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void gf3d_swapchain_init(VkPhysicalDevice device,VkDevice logicalDevice,VkSurfaceKHR surface,Uint32 width,Uint32 height)
{
    int i;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &gf3d_swapchain.capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &gf3d_swapchain.formatCount, NULL);

    slog("device supports %i surface formats",gf3d_swapchain.formatCount);
    if (gf3d_swapchain.formatCount != 0)
    {
        gf3d_swapchain.formats = (VkSurfaceFormatKHR*)gfc_allocate_array(sizeof(VkSurfaceFormatKHR),gf3d_swapchain.formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &gf3d_swapchain.formatCount, gf3d_swapchain.formats);
        for (i = 0; i < gf3d_swapchain.formatCount; i++)
        {
            slog("surface format %i:",i);
            slog("format: %i",gf3d_swapchain.formats[i].format);
            slog("colorspace: %i",gf3d_swapchain.formats[i].colorSpace);
        }
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &gf3d_swapchain.presentModeCount, NULL);

    slog("device supports %i presentation modes",gf3d_swapchain.presentModeCount);
    if (gf3d_swapchain.presentModeCount != 0)
    {
        gf3d_swapchain.presentModes = (VkPresentModeKHR*)gfc_allocate_array(sizeof(VkPresentModeKHR),gf3d_swapchain.presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &gf3d_swapchain.presentModeCount, gf3d_swapchain.presentModes);
        for (i = 0; i < gf3d_swapchain.presentModeCount; i++)
        {
            slog("presentation mode: %i is %i",i,gf3d_swapchain.presentModes[i]);
        }
    }
    
    gf3d_swapchain.chosenFormat = gf3d_swapchain_choose_format();
    slog("chosing surface format %i",gf3d_swapchain.chosenFormat);
    
    gf3d_swapchain.chosenPresentMode = gf3d_swapchain_get_presentation_mode();
    slog("chosing presentation mode %i",gf3d_swapchain.chosenPresentMode);
    
    gf3d_swapchain.extent = gf3d_swapchain_configure_extent(width,height);
    slog("chosing swap chain extent of (%i,%i)",gf3d_swapchain.extent.width,gf3d_swapchain.extent.height);
    
    gf3d_swapchain_create(logicalDevice,surface);
    gf3d_swapchain.device = logicalDevice;
    
    atexit(gf3d_swapchain_close);
}

void gf3d_swapchain_create_frame_buffer(VkFramebuffer *buffer,VkImageView *imageView,Pipeline *pipe)
{
    VkFramebufferCreateInfo framebufferInfo = {0};
    VkImageView imageViews[2];
    
    imageViews[0] = *imageView;
    imageViews[1] = gf3d_swapchain.depthImageView;

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = pipe->renderPass;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = imageViews;
    framebufferInfo.width = gf3d_swapchain.extent.width;
    framebufferInfo.height = gf3d_swapchain.extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(gf3d_swapchain.device, &framebufferInfo, NULL, buffer) != VK_SUCCESS)
    {
        slog("failed to create framebuffer!");
    }
    else
    {
        slog("created framebuffer");
    }
}

void gf3d_swapchain_setup_frame_buffers(Pipeline *pipe)
{
    int i;
    gf3d_swapchain.frameBuffers = (VkFramebuffer *)gfc_allocate_array(sizeof(VkFramebuffer),gf3d_swapchain.swapImageCount);
    for (i = 0; i < gf3d_swapchain.swapImageCount;i++)
    {
        gf3d_swapchain_create_frame_buffer(&gf3d_swapchain.frameBuffers[i],&gf3d_swapchain.imageViews[i],pipe);
    }
    gf3d_swapchain.framebufferCount = gf3d_swapchain.swapImageCount;
}

VkFormat gf3d_swapchain_get_format()
{
    return gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format;
}

void gf3d_swapchain_create(VkDevice device,VkSurfaceKHR surface)
{
    int i;
    Sint32 graphicsFamily;
    Sint32 presentFamily;
    Sint32 transferFamily;
    VkSwapchainCreateInfoKHR createInfo = {0};
    Uint32 queueFamilyIndices[3];
    
    slog("minimum images needed for swap chain: %i",gf3d_swapchain.capabilities.minImageCount);
    slog("Maximum images needed for swap chain: %i",gf3d_swapchain.capabilities.maxImageCount);
    gf3d_swapchain.swapChainCount = gf3d_swapchain.capabilities.minImageCount + 1;
    if (gf3d_swapchain.capabilities.maxImageCount)gf3d_swapchain.swapChainCount = MIN(gf3d_swapchain.swapChainCount,gf3d_swapchain.capabilities.maxImageCount);
    slog("using %i images for the swap chain",gf3d_swapchain.swapChainCount);
    
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = gf3d_swapchain.swapChainCount;
    createInfo.imageFormat = gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format;
    createInfo.imageColorSpace = gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].colorSpace;
    createInfo.imageExtent = gf3d_swapchain.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    graphicsFamily = gf3d_vqueues_get_graphics_queue_family();
    presentFamily = gf3d_vqueues_get_present_queue_family();
    transferFamily = gf3d_vqueues_get_transfer_queue_family();
    queueFamilyIndices[0] = graphicsFamily;
    queueFamilyIndices[1] = presentFamily;
    queueFamilyIndices[2] = transferFamily;
    
    if (graphicsFamily != presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 3;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.preTransform = gf3d_swapchain.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // our window is opaque, but it doesn't have to be

    createInfo.presentMode = gf3d_swapchain.presentModes[gf3d_swapchain.chosenPresentMode];
    createInfo.clipped = VK_TRUE;
    
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &gf3d_swapchain.swapChain) != VK_SUCCESS)
    {
        slog("failed to create swap chain!");
        gf3d_swapchain_close();
        return;
    }
    slog("created a swap chain with length %i",gf3d_swapchain.swapChainCount);
    
    vkGetSwapchainImagesKHR(device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount, NULL);
    if (gf3d_swapchain.swapImageCount == 0)
    {
        slog("failed to create any swap images!");
        gf3d_swapchain_close();
        return;
    }
    gf3d_swapchain.swapImages = (VkImage *)gfc_allocate_array(sizeof(VkImage),gf3d_swapchain.swapImageCount);
    vkGetSwapchainImagesKHR(device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount,gf3d_swapchain.swapImages );
    slog("created swap chain with %i images",gf3d_swapchain.swapImageCount);
    
    gf3d_swapchain.imageViews = (VkImageView *)gfc_allocate_array(sizeof(VkImageView),gf3d_swapchain.swapImageCount);
    for (i = 0 ; i < gf3d_swapchain.swapImageCount; i++)
    {
        gf3d_swapchain.imageViews[i] = gf3d_vgraphics_create_image_view(gf3d_swapchain.swapImages[i],gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format);
    }
    slog("create image views");
}

VkExtent2D gf3d_swapchain_configure_extent(Uint32 width,Uint32 height)
{
    VkExtent2D actualExtent;
    slog("Requested resolution: (%i,%i)",width,height);
    slog("Minimum resolution: (%i,%i)",gf3d_swapchain.capabilities.minImageExtent.width,gf3d_swapchain.capabilities.minImageExtent.height);
    slog("Maximum resolution: (%i,%i)",gf3d_swapchain.capabilities.maxImageExtent.width,gf3d_swapchain.capabilities.maxImageExtent.height);
    
    actualExtent.width = MAX(gf3d_swapchain.capabilities.minImageExtent.width,MIN(width,gf3d_swapchain.capabilities.maxImageExtent.width));
    actualExtent.height = MAX(gf3d_swapchain.capabilities.minImageExtent.height,MIN(height,gf3d_swapchain.capabilities.maxImageExtent.height));
    return actualExtent;
}

VkExtent2D gf3d_swapchain_get_extent()
{
    return gf3d_swapchain.extent;
}


int gf3d_swapchain_get_presentation_mode()
{
    int i;
    int chosen = -1;
    for (i = 0; i < gf3d_swapchain.formatCount; i++)
    {
        if (gf3d_swapchain.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return i;
        chosen = i;
    }
    return chosen;
}

int gf3d_swapchain_choose_format()
{
    int i;
    int chosen = -1;
    for (i = 0; i < gf3d_swapchain.formatCount; i++)
    {
        if ((gf3d_swapchain.formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
            (gf3d_swapchain.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
            return i;
        chosen = i;
    }
    return chosen;
}

void gf3d_swapchain_close()
{
    int i;
    slog("cleaning up swapchain");
    
    if (gf3d_swapchain.depthImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gf3d_swapchain.device, gf3d_swapchain.depthImageView, NULL);
    }
    if (gf3d_swapchain.depthImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(gf3d_swapchain.device, gf3d_swapchain.depthImage, NULL);
    }
    if (gf3d_swapchain.depthImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gf3d_swapchain.device, gf3d_swapchain.depthImageMemory, NULL);
    }
    if (gf3d_swapchain.frameBuffers)
    {
        for (i = 0;i < gf3d_swapchain.framebufferCount; i++)
        {
            vkDestroyFramebuffer(gf3d_swapchain.device, gf3d_swapchain.frameBuffers[i], NULL);
            slog("framebuffer destroyed");
        }
        free (gf3d_swapchain.frameBuffers);
    }
    vkDestroySwapchainKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, NULL);
    if (gf3d_swapchain.imageViews)
    {
        for (i = 0;i < gf3d_swapchain.swapImageCount;i++)
        {
            vkDestroyImageView(gf3d_swapchain.device,gf3d_swapchain.imageViews[i],NULL);
            slog("imageview destroyed");
        }
        free(gf3d_swapchain.imageViews);
    }
    if (gf3d_swapchain.swapImages)
    {
        free(gf3d_swapchain.swapImages);
    }
    if (gf3d_swapchain.formats)
    {
        free(gf3d_swapchain.formats);
    }
    if (gf3d_swapchain.presentModes)
    {
        free(gf3d_swapchain.presentModes);
    }
    memset(&gf3d_swapchain,0,sizeof(vSwapChain));
}

Bool gf3d_swapchain_validation_check()
{
    if (!gf3d_swapchain.presentModeCount)
    {
        slog("swapchain has no usable presentation modes");
        return false;
    }
    if (!gf3d_swapchain.formatCount)
    {
        slog("swapchain has no usable surface formats");
        return false;
    }
    return true;
}

Uint32 gf3d_swapchain_get_chain_length()
{
    return gf3d_swapchain.swapChainCount;
}

Uint32 gf3d_swapchain_get_swap_image_count()
{
    return gf3d_swapchain.swapImageCount;
}

Uint32 gf3d_swapchain_get_frame_buffer_count()
{
    return gf3d_swapchain.framebufferCount;
}

VkSwapchainKHR gf3d_swapchain_get()
{
    return gf3d_swapchain.swapChain;
}

VkFramebuffer gf3d_swapchain_get_frame_buffer_by_index(Uint32 index)
{
    if (index >= gf3d_swapchain.framebufferCount)
    {
        slog("FATAL: index for framebuffer out of range");
        return 0;
    }
    return gf3d_swapchain.frameBuffers[index];
}

VkImageView gf3d_swapchain_create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = {0};
    VkImageView imageView;
    
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(gf3d_swapchain.device, &viewInfo, NULL, &imageView) != VK_SUCCESS)
    {
        slog("failed to create texture image view!");
        return VK_NULL_HANDLE;
    }

    return imageView;
}
    
Uint8 gf3d_swapchain_has_stencil_component(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
    
void gf3d_swapchain_transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = {0};
    Command * commandPool;
    VkCommandBuffer commandBuffer;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (gf3d_swapchain_has_stencil_component(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        slog("unsupported layout transition!");
    }
    
    commandPool = gf3d_vgraphics_get_graphics_command_pool();
    commandBuffer = gf3d_command_begin_single_time(commandPool);

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier);
    
    gf3d_command_end_single_time(commandPool, commandBuffer);
}

void gf3d_swapchain_create_depth_image()
{
    gf3d_swapchain_create_image(gf3d_swapchain.extent.width, gf3d_swapchain.extent.height, gf3d_pipeline_find_depth_format(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &gf3d_swapchain.depthImage, &gf3d_swapchain.depthImageMemory);
    gf3d_swapchain.depthImageView = gf3d_swapchain_create_image_view(gf3d_swapchain.depthImage, gf3d_pipeline_find_depth_format(),VK_IMAGE_ASPECT_DEPTH_BIT);
    gf3d_swapchain_transition_image_layout(gf3d_swapchain.depthImage, gf3d_pipeline_find_depth_format(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

uint32_t gf3d_swapchain_find_Memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    uint32_t i;
    VkPhysicalDeviceMemoryProperties memProperties;
    
    vkGetPhysicalDeviceMemoryProperties(gf3d_vgraphics_get_default_physical_device(), &memProperties);

    for (i= 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    slog("failed to find suitable memory type!");
    return 0;
}

void gf3d_swapchain_create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory)
{
    VkImageCreateInfo imageInfo = {0};
    VkMemoryAllocateInfo allocInfo = {0};
    VkMemoryRequirements memRequirements;

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(gf3d_swapchain.device, &imageInfo, NULL, image) != VK_SUCCESS)
    {
        slog("failed to create image!");
    }

    vkGetImageMemoryRequirements(gf3d_swapchain.device, *image, &memRequirements);

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = gf3d_swapchain_find_Memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(gf3d_swapchain.device, &allocInfo, NULL, imageMemory) != VK_SUCCESS)
    {
        slog("failed to allocate image memory!");
    }

    vkBindImageMemory(gf3d_swapchain.device, *image, *imageMemory, 0);
}

/*eol@eof*/
