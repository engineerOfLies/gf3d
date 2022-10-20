#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outColor;


void main()
{
    vec3 lightVector = vec3(0,0,-1);
    float cosTheta = dot( normalize(fragNormal),lightVector );
    vec4 newColor = (inColor * 0.5) + (inColor * cosTheta * 0.5);
    newColor.w = inColor.w;
    outColor = newColor;
}
