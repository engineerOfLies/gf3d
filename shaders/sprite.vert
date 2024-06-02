#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4    rotation;
    vec4    colorMod;
    vec4    clip;
    vec2    size;
    vec2    extent;
    vec2    position;
    vec2    scale;
    vec2    frame_offset;
    vec2    center;
    float   drawOrder;
    vec3    padding;
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 colorMod;
layout(location = 2) out float drawOrder;

void main()
{
    vec2 center;
    mat4 scale_m = mat4(ubo.scale.x,0,0,0,
                        0,ubo.scale.y,0,0,
                        0,0,1,0,
                        0,0,0,1);
    
    fragTexCoord = inTexCoord + ubo.frame_offset;
    vec4 clip_position = vec4(inPosition,0,1);
    
    switch (gl_VertexIndex)
    {
        case 0:
            clip_position = vec4(inPosition.x + ubo.clip.x*2,inPosition.y + ubo.clip.y*2,0,1);
            fragTexCoord.x = fragTexCoord.x + ubo.clip.x/ubo.size.x;
            fragTexCoord.y = fragTexCoord.y + ubo.clip.y/ubo.size.y;
        break;
        case 1:
            clip_position = vec4(inPosition.x - ubo.clip.z*2,inPosition.y + ubo.clip.y*2,0,1);
            fragTexCoord.x = fragTexCoord.x - ubo.clip.z/ubo.size.x;
            fragTexCoord.y = fragTexCoord.y + ubo.clip.y/ubo.size.y;
        break;
        case 2:
            clip_position = vec4(inPosition.x + ubo.clip.x*2,inPosition.y - ubo.clip.w*2,0,1);
            fragTexCoord.x = fragTexCoord.x + ubo.clip.x/ubo.size.x;
            fragTexCoord.y = fragTexCoord.y - ubo.clip.w/ubo.size.y;
        break;
        case 3:
            clip_position = vec4(inPosition.x - ubo.clip.z*2,inPosition.y - ubo.clip.w*2,0,1);
            fragTexCoord.x = fragTexCoord.x - ubo.clip.z/ubo.size.x;
            fragTexCoord.y = fragTexCoord.y - ubo.clip.w/ubo.size.y;
        break;
    }
    center = ubo.center*2;
    clip_position.xy = clip_position.xy - center;
    vec4 r_position = scale_m * ubo.rotation * clip_position;
    r_position.xy = r_position.xy + center;
    vec4 drawOffset = vec4((ubo.position * 2)/ubo.extent,0,0);
    gl_Position = vec4(r_position.xy/ubo.extent,0,1) - vec4(1,1,0,0) + drawOffset;
    colorMod = ubo.colorMod;
    drawOrder = ubo.drawOrder;
}
