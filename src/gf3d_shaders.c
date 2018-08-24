#include <stdio.h>
#include <stdlib.h>

#include "gf3d_shaders.h"
#include "simple_logger.h"


char *gf3d_shaders_load_data(char * filename)
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
    buffer = gf3d_allocate_array(sizeof(char),size);
    if (!buffer)
    {
        slog("failed to allocate memory for shader file %s",filename);
        fclose(file);
        return NULL;
    }
    fread(buffer,size,1,file);
    fclose(file);
    return buffer;
}
    
/*eol@eof*/
