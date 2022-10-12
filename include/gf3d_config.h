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
 * @param str the name
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

#if defined(VkPipelineDepthStencilStateCreateFlagBits)
/**
 * @brief parse the depth stencil state create flag from str
 * @param str the name identifier to parse
 * @return 0 as default or the proper VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE* otherwise
 */
VkPipelineDepthStencilStateCreateFlagBits gf3d_config_depth_stencil_create_flag_from_str(const char *str);

/**
 * @brief parse a list of depth stencil state create flags from a json array of strings
 * @param flags the json array containing a list of strings
 * @return 0 if not an array, empty array, or invalid strings.
 * @note strings need to math the case of the enumaration from Vulkan specification
 * @url https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineDepthStencilStateCreateFlagBits.html
 */
VkPipelineDepthStencilStateCreateFlagBits gf3d_config_depth_stencil_create_flags(SJson *flags);
#endif

/**
 * @brief parse the compare op flag from str
 * @param str the name identifier to parse
 * @return 0 as default or the proper VkCompareOp otherwise
 */
VkCompareOp gf3d_config_compar_op_flag_from_str(const char *str);

/**
 * @brief parse the primitive topology from str
 * @param str the name identifier to parse
 * @return 0 as default or the proper VK_PRIMITIVE_TOPOLOGY_* otherwise
 */
VkPrimitiveTopology gf3d_config_primitive_topology_from_str(const char *str);

/**
 * @brief extract VkPipelineRasterizationStateCreateInfo info from a json config
 * @param config the json containing description information
 * @return an empty struct on error or configured otherwise
 */
VkPipelineRasterizationStateCreateInfo gf3d_config_pipline_rasterization_state_create_info(SJson *config);

/**
 * @brief extract VkPipelineMultisampleStateCreateInfo info from a json config
 * @param config the json containing description information
 * @return an empty struct on error or configured otherwise
 */
VkPipelineMultisampleStateCreateInfo gf3d_config_pipline_multisample_state_create_info(SJson *config);

/**
 * @brief extract VkPipelineColorBlendAttachmentState info from a json config
 * @param config the json containing description information
 * @return an empty struct on error or configured otherwise
 */
VkPipelineColorBlendAttachmentState gf3d_config_pipeline_color_blend_attachment(SJson *config);

#endif
