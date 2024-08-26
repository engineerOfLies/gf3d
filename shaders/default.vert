#version 450
#extension GL_ARB_separate_shader_objects : enable

const uint MAX_BONES = 100;
const uint MAX_LIGHTS = 16;

struct MeshUBO
{
    mat4    model;
    mat4    view;
    mat4    proj;
    vec4    color; 
    vec4    camera;            //needed for many light calculations
};

struct ArmatureUBO
{
    mat4 bones[MAX_BONES];
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

struct Light
{
    vec4    color;         //color of light
    vec4    direction;     //used for directional light
    vec4    position;      //where the light is from.  w determines if its a directional light or attenuated light
    float   ambientCoefficient;
    float   attenuation;   //how fast this light falls off
    float   angle;         //If nonzero, it is a spot light.  
    float   range;        //if nonzero, light will not illuminate past this distance
};

struct LightUBO
{
    Light lights[MAX_LIGHTS];       //list of all lights
    vec4  flags;
};


layout(binding = 0) uniform UniformBufferObject
{
    MeshUBO         mesh;
    ArmatureUBO     armature;
    MaterialUBO     material;   //this may become an array
    LightUBO        lights;
    vec4            flags;      //.x is for bones
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

    //bones or no bones
    if (ubo.flags.x != 0.0)
    {
        //determine net bone matrix
        bone = (boneWeights.x * ubo.armature.bones[int(boneIndices.x)])
             + (boneWeights.y * ubo.armature.bones[int(boneIndices.y)])
             + (boneWeights.z * ubo.armature.bones[int(boneIndices.z)])
             + (boneWeights.w * ubo.armature.bones[int(boneIndices.w)]);
        //applyg it to the position
        gl_Position =  mvp * bone * vec4(inPosition, 1.0);
        tempPosition = ubo.mesh.model * bone * vec4(inPosition, 1.0);//now in world space

        //apply it to the normal
        normalMatrix = mat3(bone);
         normalMatrix = transpose(inverse(mat3(ubo.mesh.model) * normalMatrix));
        fragNormal = normalize(normalMatrix * inNormal);
    }
    else
    {
        gl_Position = mvp * vec4(inPosition, 1.0);
        tempPosition = ubo.mesh.model * vec4(inPosition, 1.0);//now in world space
        normalMatrix = transpose(inverse(mat3(ubo.mesh.model)));
        fragNormal = normalize(inNormal * normalMatrix);
    }
    position = tempPosition.xyz;
}
