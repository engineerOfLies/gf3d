#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 center;
layout(location = 2) in float size;
layout(location = 3) in float depth;


void main()
{
    vec2 coord = (gl_FragCoord.xy - center) / size;
    float l = length(coord);
    if (l > 1.0) discard;
    outColor = inColor;
    outColor.w = 1 - l;
}
