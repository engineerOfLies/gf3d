#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 colorMod;
layout(location = 3) in vec4 fragAmbient;
layout(location = 4) in vec3 fragAmbientDir;
layout(location = 5) in vec3 position;
layout(location = 6) in vec3 camPosition;

layout(location = 0) out vec4 outColor;


void main()
{
    float shininess = 128;
    vec3 normal = normalize(fragNormal);
    vec3 eyeDir = normalize(camPosition - position);
    vec3 lightDir = normalize(fragAmbientDir);
    float cosTheta = dot( normal,lightDir);
    
    vec4 baseColor = texture(texSampler, fragTexCoord);
    
    vec4 ambient = clamp(vec4(fragAmbient.xyz * cosTheta * fragAmbient.w,0),0,1) +clamp(vec4(baseColor.xyz * cosTheta * fragAmbient.w,0),-1,0);;
    
    vec3 h = normalize(lightDir + eyeDir);
    float intSpec = clamp(dot(h,normal), 0,1);
    vec4 specular = fragAmbient * pow(intSpec, shininess);

    outColor = baseColor + specular * 0.5 + ambient * 0.5;
    outColor = clamp(outColor,vec4(0,0,0,0),colorMod);
    outColor.w = baseColor.w * colorMod.w;//only texture and specific intent will make something transparent
}
