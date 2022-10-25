#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 colorMod;

layout(location = 0) out vec4 outColor;


void main()
{
    vec4 baseColor = texture(texSampler, fragTexCoord);
    outColor = baseColor;
//     outColor.x = baseColor.x * colorMod.x;
//     outColor.y = baseColor.y * colorMod.y;
//     outColor.z = baseColor.z * colorMod.z;
//     outColor.w = baseColor.w * colorMod.w;
}
