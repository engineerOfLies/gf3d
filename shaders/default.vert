#version 450
#extension GL_ARB_separate_shader_objects : enable

const uint MAX_LIGHTS = 16;

struct MeshUBO
{
    mat4    model;
    mat4    view;
    mat4    proj;
    vec4    color; 
    vec4    camera;            //needed for many light calculations
};

struct MaterialUBO
{
    vec4    ambient;        //how much ambient light affects this material
    vec4    diffuse;        //how much diffuse light affects this material - primary influcen for material color
    vec4    specular;       //color of the shine on the materials
    vec4    emission;       //color that shows regardless of light
    float   transparency;   //how translucent the material should be overall
    float   shininess;      //how shiny the materials is.  // how pronounced the specular is
    vec2    padding;        //for alignment
};

layout(binding = 0) uniform UniformBufferObject
{
    MeshUBO         mesh;
    MaterialUBO     material;   //this may become an array
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec4 boneIndices;
layout(location = 4) in vec4 boneWeights;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 position;

void main()
{
    vec4 tempPosition;
    mat4 bone;
    mat4 mvp = ubo.mesh.proj * ubo.mesh.view * ubo.mesh.model;
    mat3 normalMatrix;
    fragTexCoord = inTexCoord;

    gl_Position = mvp * vec4(inPosition, 1.0);
    tempPosition = ubo.mesh.model * vec4(inPosition, 1.0);//now in world space
    normalMatrix = transpose(inverse(mat3(ubo.mesh.model)));
    fragNormal = normalize(inNormal * normalMatrix);
    position = tempPosition.xyz;
}
