#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    vec2 size;
    vec2 extent;
    vec4 colorMod;
    vec2 position;
    vec2 scale;
    vec2 frame_offset;
    vec3 rotation;
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 colorMod;

void main()
{
    gl_Position = vec4(ubo.scale * inPosition + (ubo.position * 2)/ubo.extent,0, 1.0);
    gl_Position = gl_Position - vec4(1,1,0,0);

    fragTexCoord = inTexCoord + ubo.frame_offset;
    colorMod = ubo.colorMod;
}
