#ifndef __GF3D_SWAPCHAIN_H__
#define __GF3D_SWAPCHAIN_H__

#include <vulkan/vulkan.h>
#include "gfc_types.h"

#include "gf3d_pipeline.h"

/**
 * @brief validate and setup swap chain
 * @param device the physical device to init the swap chain for
 * @param logicalDevice the logical device to make the swap chain for
 * @param surface the surface that the swap chain should support
 * @param width the desired width of the swap chain buffers
 * @param height the desired height of the swap chain buffers
 */
void gf3d_swapchain_init(VkPhysicalDevice device,VkDevice logicalDevice, VkSurfaceKHR surface,Uint32 width,Uint32 height);

/**
 * @brief check if the initialized swap chain is sufficient for rendering
 * @returns false if not, true if it will work for rendering
 */
Bool gf3d_swapchain_validation_check();

/**
 * @brief get the current extent configured for the swap chain
 * @returns the width and height of the swap chain images
 */
VkExtent2D gf3d_swapchain_get_extent();

/**
 * @brief get the swap image surface format
 * @returns the format chosen or 0 if none specified
 */
VkFormat gf3d_swapchain_get_format();

/**
 * @brief create frame buffers
 * @param pipe the pipeline to create the frame buffers for
 */
void gf3d_swapchain_setup_frame_buffers(Pipeline *pipe);

/**
 * @brief called at exit to clean up the swap chains
 */
void gf3d_swapchain_close();

Uint32 gf3d_swapchain_get_frame_buffer_count();
Uint32 gf3d_swapchain_get_swap_image_count();
Uint32 gf3d_swapchain_get_chain_length();

/**
 * @brief get the swapchain to use for rendering
 */
VkSwapchainKHR gf3d_swapchain_get();

/**
 * @brief get the frame buffer by the index
 * @param index the frame buffer to get
 * @return the framebuffer in question
 */
VkFramebuffer gf3d_swapchain_get_frame_buffer_by_index(Uint32 index);


void gf3d_swapchain_create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);

void gf3d_swapchain_transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

void gf3d_swapchain_create_depth_image();


#endif
