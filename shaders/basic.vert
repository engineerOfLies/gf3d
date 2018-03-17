#version 300

layout(location = 0) in vec3 vertexBuffer;

void main()
{
    gl_Position.xyz = vertexBuffer;
    gl_Position.w = 1.0;
}