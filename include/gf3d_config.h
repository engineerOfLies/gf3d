#ifndef __GF3D_CONFIG_H__
#define __GF3D_CONFIG_H__

#include <vulkan/vulkan.h>
#include "simple_json.h"


/**
 * @brief parse a VkDescriptorType from a str
 * @param str the string to parse
 * @return 0 on error or the VkDescriptorType, note zero is also a valid response
 */
VkDescriptorType gf3d_config_descriptor_type_from_str(const char *str);

/**
 * @brief parse a VkPipelineBindPoint from a str
 * @param str the string to parse
 * @return 0 on error or the VkPipelineBindPoint
 */
VkPipelineBindPoint gf3d_config_pipeline_bindpoint_from_str(const char *str);

/**
 * @brief parse a json object containing VkSubpassDependency data
 * @param config the json to parse
 * @return an empty VkDependencyFlags on error or configured VkSubpassDependency otherwise
 */
VkSubpassDependency gf3d_config_subpass_dependency(SJson *config);

/**
 * @brief parse a json array containing VkDependencyFlags names
 * @param array the json array containing strings of VkDependencyFlags
 * @return 0 on fail or VkDependencyFlags otherwise
 */
VkDependencyFlags gf3d_config_dependency_flag_bits(SJson *array);

/**
 * @brief parse  a json array containing VkAccessFlagBits names
 * @param array the json array containing strings of VkAccessFlagBits
 * @return 0 on fail or VkAccessFlagBits otherwise
 */
VkAccessFlagBits gf3d_config_access_flag_bits(SJson *array);

/**
 * @brief parse  a json array containing VkPipelineStageFlag names
 * @param array the json array containing strings of VkPipelineStageFlags
 * @return 0 on fail or VkPipelineStageFlags otherwise
 */
VkPipelineStageFlags gf3d_config_pipeline_stage_flags(SJson *array);

/**
 * @brief parse the str into the VkPipelineStageFlag
 * @param str the input
 * @return 0 on fail or VkPipelineStageFlag otherwise
 */
VkPipelineStageFlags gf3d_config_pipeline_stage_flags_from_str(const char *str);

/**
 * @brief parse the image layout from config str
 * @param config the config file to parse
 * @return VK_IMAGE_LAYOUT_UNDEFINED as default or the proper VK_IMAGE_LAYOUT_* otherwise
 */
VkImageLayout gf3d_config_image_layer(SJson *config);

/**
 * @brief parse the image layout from str
 * @param config the config file to parse
 * @return VK_IMAGE_LAYOUT_UNDEFINED as default or the proper VK_IMAGE_LAYOUT_* otherwise
 */
VkImageLayout gf3d_config_image_layer_from_str(const char *str);

/**
 * @brief extract VkAttachmentDescription info from a json config
 * @param config the json containing description information
 * @param format provide this, as it may need to come from screen format, configured elsewhere
 * @return an empty description on error or a configured on otherwise
 */
VkAttachmentDescription gf3d_config_attachment_description(SJson *config,VkFormat format);

#endif
