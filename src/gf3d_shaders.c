#include <SDL.h>
#include <GL/glew.h>
#include "gf3d_shaders.h"
#include "simple_logger.h"
#include "gf3d_graphics.h"
#include <stdio.h>
#include <stdlib.h>


GLuint gf3d_shader_load(const char * filepath,GLuint shader_type)
{
    FILE* file;
    char *buffer = NULL;
    GLuint shader_id = 0;
    GLint length,compiled;
    file = fopen(filepath, "r");
    if (!file)
    {
        slog("failed to open shader file: %s",filepath);
        return 0;
    }
    fseek (file , 0 , SEEK_END);
    length = ftell (file);
    if (length <= 0)
    {
        slog("failed to read from shader file %s",filepath);
        return 0;
    }
    rewind (file);
    buffer = (char*)malloc(sizeof(char)*length);
    if (!buffer)
    {
        slog("failed to allocate read buffer for shader file %s",filepath);
        fclose(file);
        return 0;
    }
    fread(buffer,1,length,file);
    fclose(file);
    
    shader_id = glCreateShader(shader_type);

    glShaderSource(shader_id, 1, (const GLchar **)&buffer, &length);
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE)
    {
        slog("failed to compile shader %s",filepath);
        return 0;
    }
    
    return shader_id;
}
    
    
/*eol@eof*/
