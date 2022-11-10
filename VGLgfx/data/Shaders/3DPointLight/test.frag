#version 450
#extension GL_ARB_separate_shader_objects : enable
#define PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformBufferObject
{
	vec4 diffuse;
	vec4 specular;
	vec4 ambient;
	vec3 lightDir;
}ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec3 viewPos;
layout(location = 5) in mat3 toModelSpace;
layout(location = 8) in vec4 lightPosFrag;

layout(binding = 2) uniform sampler2D image;
layout(binding = 3) uniform sampler2D imageBump;
layout(binding = 4) uniform sampler2D shadowMap;

vec4 lightPoint(vec3	p_LightPos, vec3 p_Color);
vec4 lightDir(vec3		p_LightDir, vec3 p_Color);
vec4 lightDirPBR(vec3	p_LightDir, vec3 p_Color);
vec4 lightPointPBR(vec3 p_LightPos, vec3 p_Color);

void main()
{
	if(texture(image, UV).a < 0.5)
		discard;

	vec4 l1 = /*lightPointPBR(viewPos, vec3(1.0f, 1.0f, 1.0f)) + */lightDirPBR(ubo.lightDir * -1, vec3(7.0f));

	outColor = l1;

	//outColor *= dtexture;

	float gamma = 1.7f;
	vec4 mapped = outColor / (outColor + vec4(1.0f));
	mapped = pow(mapped, vec4(1.0f / gamma));
	
	outColor = mapped;
}

vec4 lightPoint(vec3 p_LightPos, vec3 p_Color)
{
	float constant = 1.0f;
	float linear = 0.0014;
	float quadratic = 0.000007;

	float distance = length(p_LightPos - fragPos);
	float attenuation = 1.0f / (constant + linear * distance + quadratic * distance * distance);

	float ambientStr = 0.05;

	vec3 normal = normalize(texture(imageBump, UV).rgb * 2.0f - 1.0f);
	normal = normalize(toModelSpace * normal);

	vec3 lightDir = normalize(p_LightPos - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfDir = normalize(viewDir + lightDir);

	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * ubo.diffuse.xyz;
	vec3 ambientColor = ambientStr * (p_Color * ubo.ambient.xyz);
	vec3 specular = pow(max(dot(normal, halfDir), 0.0f), 64.0f) * vec3(1, 1, 1);
	
	return vec4((ambientColor + diffuse + specular) * attenuation * p_Color, 1.0f);
}

vec4 lightDir(vec3 p_LightDir, vec3 p_Color)
{
	float ambientStr = 0.3;

	vec3 normal = normalize(texture(imageBump, UV).rgb * 2.0f - 1.0f);
	normal = normalize(toModelSpace * normal);

	vec3 lightDir = normalize(-p_LightDir);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfDir = normalize(viewDir + lightDir);

	vec3 diffuse = max(dot(normal, lightDir), 0.0f) * vec3(1, 1, 1);
	vec3 ambientColor = ambientStr * (p_Color);
	vec3 specular = pow(max(dot(normal, halfDir), 0.0f), 64.0f) * vec3(1, 1, 1);
	
	return vec4((ambientColor + diffuse + specular) * p_Color, 1.0f);
}


float distGGX(vec3 N, vec3 H, float a)
{
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH*NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return nom / denom;
}

float geoSchlickGGX(float NdotV, float k)
{
	float nom = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}

float geoSmith(vec3 N, vec3 V, vec3 L, float k)
{
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);

	float ggx1 = geoSchlickGGX(NdotV, k);
	float ggx2 = geoSchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(1.0f - min(cosTheta, 1.0), 5.0f);
}

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, vec2(shadowCoord.s, 1.0f - shadowCoord.t) + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.0;
		}
	}
	return shadow;
}

float shadowMappingPCF()
{
	vec4 sc = lightPosFrag / lightPosFrag.w;

	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 0.8;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 2;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}


vec4 lightDirPBR(vec3 p_LightDir, vec3 p_Color)
{
	vec3 lightPos = viewPos;
	vec3 lightColor = p_Color;
	vec3 lightDir = p_LightDir;


	vec3 albedo		= pow(texture(image, UV).rgb, vec3(2.2));
	vec3 normal		= normalize(texture(imageBump, UV).rgb * 2.0f - 1.0f);
	normal			= normalize(toModelSpace * normal);

	float metallic	= 0.7;
	float roughness = 0.5;
	float ao		= 1.0f;

	//float metallic = 1.0;
	//float roughness = 0.1;
	//float ao = 1.0;

	vec3 V = normalize(viewPos - fragPos);

	vec3 Lo = vec3(0.0f);
	vec3 L = normalize(-lightDir);//normalize(lightPos - fragPos);
	vec3 H = normalize(V + L);

	float dist = length(lightPos - fragPos);
	float attenuation = 1.0f / (dist * dist);
	vec3 radiance = lightColor;// * attenuation;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

	float NDF = distGGX(normal, H, roughness);
	float G = geoSmith(normal, V, L, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0f * max(dot(normal, V), 0.0f) * max(dot(normal, L), 0.0f);
	vec3 specular = numerator / max(denominator, 0.001f);

	vec3 Ks = F;
	vec3 Kd = vec3(1.0f) - Ks;
	Kd *= 1.0f - metallic;

	float NdotL = max(dot(normal, L), 0.0f);
	Lo += (Kd * albedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.025) * albedo * ao;
	vec3 color = ambient + Lo * shadowMappingPCF();

	return vec4(color, 1.0f);
}

vec4 lightPointPBR(vec3 p_Pos, vec3 p_Color)
{
	vec3 lightPos = p_Pos;
	vec3 lightColor = p_Color;
	vec3 lightDir = normalize(p_Pos - fragPos);

	vec3 albedo		= pow(texture(image, UV).rgb, vec3(2.2));
	vec3 normal		= normalize(texture(imageBump, UV).rgb * 2.0f - 1.0f);
	normal			= normalize(toModelSpace * normal);

	float metallic	= 0.7;
	float roughness = 0.5;
	float ao		= 1.0f;
	//float roughness = 0.1;
	//float ao = 1.0;

	vec3 V = normalize(viewPos - fragPos);

	vec3 Lo = vec3(0.0f);
	vec3 L = normalize(lightPos - fragPos);
	vec3 H = normalize(V + L);

	float dist = length(lightPos - fragPos);
	float lightRadius = 100;
	//float attenuation = 1.0f / (constant + linear * dist + quadratic * dist * dist);
	float attenuation = (clamp(pow(1-pow(dist/lightRadius, 4), 2), 1, 0))/dist*dist+1;
	vec3 radiance = lightColor * attenuation;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

	float NDF = distGGX(normal, H, roughness);
	float G = geoSmith(normal, V, L, roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0f * max(dot(normal, V), 0.0f) * max(dot(normal, L), 0.0f);
	vec3 specular = numerator / max(denominator, 0.001f);

	vec3 Ks = F;
	vec3 Kd = vec3(1.0f) - Ks;
	Kd *= 1.0f - metallic;

	float NdotL = max(dot(normal, L), 0.0f);
	Lo += (Kd * albedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;

	return vec4(color, 1);
}
