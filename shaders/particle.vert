#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 color;
    vec2 viewportSize;
    float size;
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 center;
layout(location = 2) out float size;

void main()
{
    mat4 mvp = ubo.proj * ubo.view * ubo.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
    outColor = ubo.color;
    
    center = (0.5 * gl_Position.xy/gl_Position.w + 0.5) * ubo.viewportSize;
    gl_PointSize = ubo.viewportSize.y * ubo.size / gl_Position.w;
    size = gl_PointSize/2;
}
