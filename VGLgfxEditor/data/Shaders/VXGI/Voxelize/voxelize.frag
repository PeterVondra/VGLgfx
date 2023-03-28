#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout (binding = 0, rgba8) uniform writeonly image3D gTexture3D;
layout(binding = 1) uniform sampler2D in_Albedo;
 
layout(location = 2) in vec2 UV;
layout(location = 3) in vec3 World_Position;
layout(location = 4) in vec3 World_Normal;

bool isInsideUnitCube()
{
    return abs(World_Position.x) < 1.0f && abs(World_Position.y) < 1.0f && abs(World_Position.z) < 1.0f;
}
 
void main()
{
    if (!isInsideUnitCube())
        return;
    
    vec3 albedo = texture(in_Albedo, UV).rgb; 
    vec3 position = World_Position * 0.5f + 0.5f;
    imageStore(gTexture3D, ivec3(imageSize(gTexture3D) * position), vec4(vec3(albedo), 1.0f));
}