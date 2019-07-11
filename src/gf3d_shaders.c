#include <stdio.h>
#include <stdlib.h>
#include "simple_logger.h"

#include "gf3d_shaders.h"


VkShaderModule gf3d_shaders_create_module(char *shader,size_t size,VkDevice device)
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

char *gf3d_shaders_load_data(char * filename,size_t *rsize)
{
    char *buffer = NULL;
    FILE *file;
    size_t size;
    file = fopen(filename,"rb");
    if (!file)
    {
        slog("failed to open shader file %s",filename);
        return NULL;
    }
    fseek(file,0,SEEK_END);
    size = ftell(file);
    if (!size)
    {
        slog("file %s contained no data",filename);
        fclose(file);
        return NULL;
    }
    rewind(file);
    buffer = gfc_allocate_array(sizeof(char),size);
    if (!buffer)
    {
        slog("failed to allocate memory for shader file %s",filename);
        fclose(file);
        return NULL;
    }
    fread(buffer,size,1,file);
    fclose(file);
    if (rsize)*rsize = size;
    return buffer;
}
    
/*eol@eof*/
