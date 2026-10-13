#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColorMod;

layout(location = 0) out vec4 outColor;


void main()
{
    vec4 texColor = texture(texSampler, inUV);
    //do math for normals
    outColor = texColor * inColorMod;
}
