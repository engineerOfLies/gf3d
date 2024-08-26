#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 color;
    vec4 color2;
    vec2 viewportSize;
    vec2 texture_offset;
    vec2 texture_size;
    uint  textured;
    float size;
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor2;
layout(location = 2) out vec2 center;
layout(location = 3) out vec2 texture_offset;
layout(location = 4) out vec2 texture_size;
layout(location = 5) out float size;
layout(location = 6) out flat uint outTextured;

void main()
{
    mat4 mvp = ubo.proj * ubo.view * ubo.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
    outColor = ubo.color;
    outColor2 = ubo.color2;
    
    center = (0.5 * gl_Position.xy/gl_Position.w + 0.5) * ubo.viewportSize;
    gl_PointSize = ubo.viewportSize.y * ubo.size / gl_Position.w;
    size = gl_PointSize/2;
    outTextured = ubo.textured;
    texture_offset = ubo.texture_offset;
    texture_size = ubo.texture_size;
}
