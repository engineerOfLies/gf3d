#version 300 es

layout(location = 0) in vec3 vertexBuffer;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vertexBuffer,1);
}