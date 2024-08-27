#include <stdio.h>
#include <stdlib.h>

#include "simple_logger.h"

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
    FILE* file;
    char *buffer = NULL;
    size_t size;
    file = fopen(filename,"r+b");
    size = get_file_Size(file);

    buffer = (char*)gfc_allocate_array(sizeof(char),size);
    if (!buffer)
    {
        slog("failed to allocate shader buffer for %s", filename);
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, size, file);
    fclose(file);
    if (rsize)*rsize = size;
    return buffer;
}
    
/*eol@eof*/
