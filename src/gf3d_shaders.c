#include <stdio.h>
#include <stdlib.h>

#include "simple_logger.h"

#include "gfc_pak.h"

#include "gf3d_shaders.h"


VkShaderModule gf3d_shaders_create_module(const char *shader,size_t size,VkDevice device)
{
    VkShaderModule module = {0};
    VkShaderModuleCreateInfo createInfo = {0};
    
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = (const uint32_t*)shader;
    
    if (vkCreateShaderModule(device, &createInfo, NULL, &module) != VK_SUCCESS)
    {
        slog("failed to create shader module");
    }
    return module;
}

char *gf3d_shaders_load_data(const char * filename,size_t *rsize)
{
    char *buffer = NULL;
    size_t size;
    
    buffer = gfc_pak_file_extract(filename,&size);
    if (!buffer)
    {
        slog("failed to laod shader file %s",filename);
        return NULL;
    }    
    if (rsize)*rsize = size;
    return buffer;
}
    
/*eol@eof*/
