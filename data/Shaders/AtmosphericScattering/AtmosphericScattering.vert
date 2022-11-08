#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 vp;
	mat4 p;

	vec3 p_RayOrigin;			// ray origin
	vec3 p_SunPos;				// position of the sun
	float p_SunIntensity;       // intensity of the sun
	float p_PlanetRadius;       // radius of the planet in meters
	float p_AtmosphereRadius;	// radius of the atmosphere in meters
	vec3 p_RayleighSC;			// Rayleigh scattering coefficient
	float p_MieSC;				// Mie scattering coefficient
	float p_RayleighSHeight;    // Rayleigh scale height
	float p_MieSHeight;         // Mie scale height
	float p_MieDir;				// Mie preferred scattering direction
	float p_Scale;				// Mie preferred scattering direction
}ubo;

layout(location = 0) out vec3 localPos;
layout(location = 1) out vec3	_RayOrigin;
layout(location = 2) out vec3	_SunPos;
layout(location = 3) out float	_SunIntensity;
layout(location = 4) out float	_PlanetRadius;
layout(location = 5) out float	_AtmosphereRadius;
layout(location = 6) out vec3	_RayleighSC;
layout(location = 7) out float	_MieSC;
layout(location = 8) out float	_RayleighSHeight;
layout(location = 9) out float _MieSHeight;
layout(location = 10) out float _MieDir;
layout(location = 11) out float _Scale;


void main()
{
	gl_Position =  ubo.vp * vec4(inPosition, 1.0f);
	localPos = normalize(inPosition);

	_RayOrigin = ubo.p_RayOrigin;
	_SunPos = ubo.p_SunPos;
	_SunIntensity = ubo.p_SunIntensity;
	_PlanetRadius = ubo.p_PlanetRadius;
	_AtmosphereRadius = ubo.p_AtmosphereRadius;
	_RayleighSC = ubo.p_RayleighSC;
	_MieSC = ubo.p_MieSC;
	_RayleighSHeight = ubo.p_RayleighSHeight;
	_MieSHeight = ubo.p_MieSHeight;
	_MieDir = ubo.p_MieDir;
	_Scale = ubo.p_Scale;
}