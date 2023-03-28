#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 out_Position;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec4 out_Albedo;
// R = Metallic, G = Roughness, B = Ambient Occlusion
layout(location = 3) out vec4 out_MRAA; // A = Depth

layout(location = 0) in vec3 in_LocalPosition;
layout(location = 1) in float in_Depth;

layout (push_constant) uniform PushConstants {
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
}psc;

#define PI 3.14159265358979323846

// The scattering coefficients for the Earth's atmosphere
// These values are approximate and may vary depending on the actual composition of the atmosphere
const vec3 RAYLEIGH_COEFF = vec3(0.0025, 0.0028, 0.0030);
const vec3 MIE_COEFF = vec3(0.0010, 0.0012, 0.0014);

// The scattering anisotropy factor for the Earth's atmosphere
const float MIE_G = 0.8;

// The maximum height of the atmosphere
const float ATMOSPHERE_HEIGHT = 10.0;

// Computes the optical depth for a given distance
float computeOpticalDepth(float height) {
    float h = clamp(height, 0.0, ATMOSPHERE_HEIGHT);
    return exp(-h * dot(RAYLEIGH_COEFF + MIE_COEFF, vec3(1.0f)));
}

// Computes the in-scattering color for a given direction and height
vec3 computeInscattering(vec3 direction, float height) {
    // Compute the optical depth along the given direction
    float cosTheta = dot(direction, normalize(psc.p_SunPos));
    float opticalDepth = computeOpticalDepth(height) / (4.0 * PI);
    
    // Compute the Rayleigh and Mie scattering colors
    vec3 rayleighColor = RAYLEIGH_COEFF * (1.0 + cosTheta * cosTheta);
    vec3 mieColor = MIE_COEFF * (1.0 - MIE_G * MIE_G) / pow(1.0 + MIE_G * MIE_G - 2.0 * MIE_G * cosTheta, 1.5);
    
    // Return the combined in-scattering color
    return (rayleighColor + mieColor) * opticalDepth;
}

void main()
{
    out_Normal = vec4(0.0f);
    
    // Compute the in-scattering color for the current pixel
    vec3 color = computeInscattering(in_LocalPosition, 1.0f - psc.p_PlanetRadius);
	
	out_Albedo = vec4(color * psc.p_SunIntensity, 0.0f);
    out_Position.a = -1.0f;
    out_MRAA = vec4(vec3(0.0f), 1.0f);
}