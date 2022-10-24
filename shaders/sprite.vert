#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 rotation;
    vec4 colorMod;
    vec2 size;
    vec2 extent;
    vec2 position;
    vec2 scale;
    vec2 frame_offset;
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
    vec2 s_position = inPosition.xy * ubo.scale;
    vec4 r_position = ubo.rotation * vec4(s_position,0,1);
    vec4 drawOffset = vec4((ubo.position * 2)/ubo.extent,0,0);
    gl_Position = vec4(r_position.xy,0,1) - vec4(1,1,0,0) + drawOffset;
        
    fragTexCoord = inTexCoord + ubo.frame_offset;
    colorMod = ubo.colorMod;
}
