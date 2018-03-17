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
    char error[1024];
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
        glGetShaderInfoLog(shader_id,1024,NULL,(GLchar *)error);
        slog("failed to compile shader %s, re: %s",filepath,error);
        return 0;
    }
    
    return shader_id;
}

GLuint gf3d_shader_program_load(char *vertfile, char *fragfile)
{
    GLuint program,vert,frag;
    GLint linked = 0;
    vert = gf3d_shader_load(vertfile, GL_VERTEX_SHADER);
    frag = gf3d_shader_load(fragfile, GL_FRAGMENT_SHADER);
    if ((!vert)||(!frag))
    {
        slog("failed to load shader progam");
        return 0;
    }

    program = glCreateProgram();
    if (!program)
    {
        slog("failed to create shader program");
        return 0;
    }
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
      slog("failed to link shader");
      return 0;
    }
    if (vert)
    {
      glDeleteShader(vert);
    }
    
    if (frag)
    {
      glDeleteShader(frag);
    }
    return program;
}

    
/*eol@eof*/
