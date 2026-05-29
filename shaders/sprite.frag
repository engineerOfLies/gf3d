#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 colorMod;
layout(location = 2) in float drawOrder;
layout(location = 4) in vec4 clipRect;

layout(location = 0) out vec4 outColor;

layout(origin_upper_left) in vec4 gl_FragCoord;

void main()
{
    if ((gl_FragCoord.x < clipRect.x)||(gl_FragCoord.y < clipRect.y))discard;
    if ((clipRect.z > 0)&&(gl_FragCoord.x > (clipRect.x+clipRect.z)))discard;
    if ((clipRect.w > 0)&&(gl_FragCoord.y > (clipRect.y+clipRect.w)))discard;
    vec4 texColor = texture(texSampler, fragTexCoord);
    outColor = texColor * colorMod;
    gl_FragDepth = drawOrder;
}
