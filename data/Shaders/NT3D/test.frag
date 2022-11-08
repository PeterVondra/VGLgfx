#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 viewPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformBufferObject
{
	vec4 diffuse;
	vec4 specular;
	vec4 ambient;
}ubo;

vec4 light(vec3 p_LightPos, vec3 p_Color);

void main()
{
	vec4 l1 = light(vec3(0, 200, 0), vec3(2.0f, 2.0f, 2.0f));
	//vec4 l2 = light(vec3(-500, -700, -500), vec3(0.0f, 2.0f, 0.0f));

	outColor = l1 * vec4(fragColor, 1);

	const float gamma = 1.7f;
	vec4 mapped = outColor / (outColor + vec4(1.0f));
	mapped = pow(mapped, vec4(1.0f / gamma));

	outColor = mapped;
}

vec4 light(vec3 p_LightPos, vec3 p_Color)
{
	float ambientStr = 0.3;
	vec3 ambientColor = ambientStr * (p_Color * ubo.ambient.xyz);

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(p_LightPos - fragPos);

	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * ubo.diffuse.xyz;

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);

	vec3 specular = 0.6 * pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f) * ubo.specular.xyz;
	
	return vec4((ambientColor + diffuse + specular) * p_Color, 1.0f);
}