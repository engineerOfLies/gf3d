#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inColor2;
layout(location = 2) in vec2 center;
layout(location = 3) in vec2 texture_offset;
layout(location = 4) in vec2 texture_size;
layout(location = 5) in float size;
layout(location = 6) in flat uint textured;

layout(location = 0) out vec4 outColor;

void main()
{
/*
    vec2 coord = (gl_FragCoord.xy - center) / size;
    float l = length(coord);
    if (l > 1.0) discard;
    outColor = inColor;
    outColor.w = 1 - l;
*/
    float l = 0;
    vec4 baseColor;
    vec2 texel = ((gl_FragCoord.xy - center) / (size * 2)) + vec2(0.5,0.5);
    
    
    if (textured != 0)
    {
        texel = (texel *texture_size)+texture_offset;// now its in the range of a frame
        baseColor = texture(texSampler, texel);
        l = 1 - length(baseColor.xyz);
        outColor = (inColor * l) + (inColor2 * (1-l));
        outColor.w  = baseColor.w;
    }
    else
    {
        vec2 coord = (gl_FragCoord.xy - center) / size;
        l = length(coord);
        if (l > 1.0) discard;
        outColor = (inColor * l) + (inColor2 * (1-l));
        outColor.w = 1 - l;
    }
}
