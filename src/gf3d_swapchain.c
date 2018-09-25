#include "gf3d_swapchain.h"
#include "gf3d_vqueues.h"

#include <string.h>
#include <stdio.h>

#include "simple_logger.h"

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
}vSwapChain;

static vSwapChain gf3d_swapchain = {0};

void gf3d_swapchain_create(VkDevice device,VkSurfaceKHR surface);
void gf3d_swapchain_close();
int gf3d_swapchain_choose_format();
int gf3d_swapchain_get_presentation_mode();
VkExtent2D gf3d_swapchain_configure_extent(Uint32 width,Uint32 height);
VkImageView gf3d_swapchain_create_imageview(VkDevice device,VkImage image);

void gf3d_swapchain_init(VkPhysicalDevice device,VkDevice logicalDevice,VkSurfaceKHR surface,Uint32 width,Uint32 height)
{
    int i;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &gf3d_swapchain.capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &gf3d_swapchain.formatCount, NULL);

    slog("device supports %i surface formats",gf3d_swapchain.formatCount);
    if (gf3d_swapchain.formatCount != 0)
    {
        gf3d_swapchain.formats = (VkSurfaceFormatKHR*)gf3d_allocate_array(sizeof(VkSurfaceFormatKHR),gf3d_swapchain.formatCount);
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
        gf3d_swapchain.presentModes = (VkPresentModeKHR*)gf3d_allocate_array(sizeof(VkPresentModeKHR),gf3d_swapchain.presentModeCount);
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

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = pipe->renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = imageView;
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
    gf3d_swapchain.frameBuffers = (VkFramebuffer *)gf3d_allocate_array(sizeof(VkFramebuffer),gf3d_swapchain.swapImageCount);
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
    VkSwapchainCreateInfoKHR createInfo = {0};
    Uint32 queueFamilyIndices[2];
    
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
    queueFamilyIndices[0] = graphicsFamily;
    queueFamilyIndices[1] = presentFamily;
    
    if (graphicsFamily != presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
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
    gf3d_swapchain.swapImages = (VkImage *)gf3d_allocate_array(sizeof(VkImage),gf3d_swapchain.swapImageCount);
    vkGetSwapchainImagesKHR(device, gf3d_swapchain.swapChain, &gf3d_swapchain.swapImageCount,gf3d_swapchain.swapImages );
    slog("created swap chain with %i images",gf3d_swapchain.swapImageCount);
    
    gf3d_swapchain.imageViews = (VkImageView *)gf3d_allocate_array(sizeof(VkImageView),gf3d_swapchain.swapImageCount);
    for (i = 0 ; i < gf3d_swapchain.swapImageCount; i++)
    {
        gf3d_swapchain.imageViews[i] = gf3d_swapchain_create_imageview(device,gf3d_swapchain.swapImages[i]);
    }
    slog("create image views");
}

VkImageView gf3d_swapchain_create_imageview(VkDevice device,VkImage image)
{
    VkImageView imageView;
    VkImageViewCreateInfo createInfo = {0};
    
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = gf3d_swapchain.formats[gf3d_swapchain.chosenFormat].format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(device, &createInfo, NULL, &imageView) != VK_SUCCESS)
    {
        slog("failed to create image view");
        return NULL;
    }
    return imageView;
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
    if (gf3d_swapchain.frameBuffers)
    {
        for (i = 0;i < gf3d_swapchain.framebufferCount; i++)
        {
            vkDestroyFramebuffer(gf3d_swapchain.device, gf3d_swapchain.frameBuffers[i], NULL);
        }
        free (gf3d_swapchain.frameBuffers);
    }
    vkDestroySwapchainKHR(gf3d_swapchain.device, gf3d_swapchain.swapChain, NULL);
    if (gf3d_swapchain.imageViews)
    {
        for (i = 0;i < gf3d_swapchain.swapImageCount;i++)
        {
            vkDestroyImageView(gf3d_swapchain.device,gf3d_swapchain.imageViews[i],NULL);
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

/*eol@eof*/
