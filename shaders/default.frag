#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main()
{
    vec3 lightVector = vec3(0,0,1);
    float cosTheta = dot( fragNormal,lightVector );
    vec4 baseColor = texture(texSampler, fragTexCoord);
    outColor = baseColor + baseColor * cosTheta;
    outColor.w = baseColor.w;
}
