#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4    model;
    mat4    view;
    mat4    proj;
    vec4    color;
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;


layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColorMod;


void main()
{
    mat4 view = ubo.view;
    outUV = inUV;
    outNormal = vec4(inNormal,0);
    view[0][3] = 0;
    view[1][3] = 0;
    view[2][3] = 0;
    view[3][0] = 0;
    view[3][1] = 0;
    view[3][2] = 0;
    outColorMod = ubo.color;
    gl_Position = ubo.proj * view * ubo.model * vec4(inPosition,1.0);
}