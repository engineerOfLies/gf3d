#version 450
#extension GL_ARB_separate_shader_objects : enable

const uint MAX_BONES = 100;
const uint MAX_LIGHTS = 8;

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
    float   padding;        //for alignment
};

struct LightUBO
{
    Light lights[MAX_LIGHTS];       //list of all lights
    vec4  flags;                    //.x is how many lights are used
};


layout(binding = 0) uniform UniformBufferObject
{
    MeshUBO         mesh;
    ArmatureUBO     armature;
    MaterialUBO     material;   //this may become an array
    LightUBO        lights;
    vec4            flags;      //.x is for bones
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 position;


layout(location = 0) out vec4 outColor;


vec3 ApplyLight(Light light, vec3 surfaceColor, vec3 normal, vec3 surfacePos, vec3 surfaceToCamera);

void main()
{   
    int i;
    vec4 surfaceColor = texture(texSampler, fragTexCoord);
    vec3 linearColor = vec3(0);
    vec3 normal = fragNormal;
    vec3 surfaceToCamera = normalize(ubo.mesh.camera.xyz - position);
    
    surfaceColor.xyz *= ubo.mesh.color.xyz * ubo.material.diffuse.xyz;
    surfaceColor.w *= ubo.mesh.color.w * ubo.material.transparency;

    if (outColor.w < 0.999999)
    {
        //drop if solid pass
    }
    else
    {
        //drop if translucent pass
    }

    if (ubo.lights.flags.x > 0)
    {
        for (i= 0; i < ubo.lights.flags.x;i++)
        {
            linearColor += ApplyLight(ubo.lights.lights[i], surfaceColor.xyz, normal, position, surfaceToCamera);
        }
    }
    else
    {
        linearColor = surfaceColor.xyz;
    }
    outColor = vec4(linearColor,surfaceColor.w);
}

vec3 ApplyLight(Light light, vec3 surfaceColor, vec3 normal, vec3 surfacePos, vec3 surfaceToCamera)
{
    vec3 surfaceToLight;
    float attenuation = 1.0;
    if(light.position.w == 0.0)
    {
        //directional light
        surfaceToLight = -normalize(light.direction.xyz);
        attenuation = 1.0; //no attenuation for directional lights
    } 
    else
    {
        //point light
        surfaceToLight = normalize(light.position.xyz - surfacePos);
        float distanceToLight = length(light.position.xyz - surfacePos);
        attenuation = (light.attenuation * light.attenuation) / (1.0 + (distanceToLight * distanceToLight));

        //cone restrictions (affects attenuation)
        if (light.angle > 0)
        {
            float lightToSurfaceAngle = degrees(acos(dot(-surfaceToLight, normalize(light.direction.xyz))));
            if (lightToSurfaceAngle > light.angle)
            {
                attenuation = 0.0;
            }
            else
            {
                attenuation *= (1.0 - (lightToSurfaceAngle / light.angle));
            }
        }
    }

    //ambient
    vec3 ambient = light.ambientCoefficient * surfaceColor.rgb * light.color.xyz;

    //diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
    vec3 diffuse = diffuseCoefficient * surfaceColor.rgb * light.color.xyz;
    
    //specular
    float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
    {
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), ubo.material.shininess);
    }
    vec3 specular = specularCoefficient * ubo.material.specular.xyz * light.color.xyz;

    //linear color (color before gamma correction)
    return ambient + attenuation*(diffuse + specular);
}
