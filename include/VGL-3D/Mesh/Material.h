#pragma once

#include <iostream>

#include "../../Math/Vector.h"
#include "../../VGL_Internal.h"

namespace vgl
{
	struct MTLShaderConfig
	{
		bool render = true;

		Vector3f m_Albedo = Vector3f(1.0f);
		float m_Metallic = 1.0f;
		float m_Roughness = 1.0f;
		float m_Ambient = 1.0f;

		float m_PDMDepth = 1.0f;

		bool operator==(MTLShaderConfig& conf)
		{
			return m_Metallic == conf.m_Metallic && m_Roughness == conf.m_Roughness && m_Ambient == conf.m_Ambient && m_Albedo == conf.m_Albedo;
		}
	};

	// Directional light
	struct D_Light {
		alignas(16)Vector3f Direction;
		alignas(16)Vector3f Color;
		alignas(4)int m_ShadowMapID = -1; // if m_ShadowMapID < 0, no shadow map in use for this light
	};
	// Spot light
	struct S_Light
	{
		Vector4f Direction;
		Vector4f Position;
		Vector4f Color;
		float Radius;
		int m_ShadowMapID = -1; // if m_ShadowMapID < 0, no shadow map in use for this light
	};
	// Point light
	struct P_Light
	{
		alignas(16)Vector4f Position = { 0, 300, 0, 0 };
		alignas(16)Vector4f Color = { 0.5, 0.5, 2, 1 };
		alignas(4)float Radius = 75;
		alignas(4)int m_ShadowMapID = -1; // if m_ShadowMapID < 0, no shadow map in use for this light
	};

	enum class ShaderType
	{
		Forward,
		Deferred,
	};

	struct ShaderInfo
	{
		// Bitflags
		uint32_t p_LightingType;
		uint32_t p_LightCaster;
		uint32_t p_LightMaps;
		uint32_t p_Effects;
		uint32_t p_MaxPrefilteredLod = 11;

		bool p_DirectionalLight;
		uint32_t p_SpotLights;
		std::vector<P_Light> p_PointLights;

		bool operator==(ShaderInfo& p_S2)
		{
			if (p_LightingType == p_S2.p_LightingType && p_LightCaster == p_S2.p_LightCaster && p_LightMaps == p_S2.p_LightMaps && p_Effects == p_S2.p_Effects)
				return true;
			return false;
		}
	};

	class Material
	{
		public:
			Material();
			Material(const Material& p);
			~Material();

			void setName(const std::string& name);

			std::string getName();

			vk::Image m_AlbedoMap;
			vk::Image m_NormalMap;
			vk::Image m_DisplacementMap;
			vk::Image m_RoughnessMap;
			vk::Image m_AOMap;
			vk::Image m_MetallicMap;
			
			std::string m_AlbedoMapPath;
			std::string m_NormalMapPath;
			std::string m_DisplacementMapPath;
			std::string m_RoughnessMapPath;
			std::string m_AOMapPath;
			std::string m_MetallicMapPath;

			std::string m_Name;
			uint32_t m_PipelineIndex;

			MTLShaderConfig config;
		private:
			friend class MeshData;
			friend class Renderer;

			MTLShaderConfig m_PrevConfig;
			ShaderInfo m_ShaderInfo;

			std::vector<ShaderDescriptor> m_Descriptors;

			bool recreate = false;
	};

	enum class LightingType
	{
		PBR = 0x01,
		BlinnPhong = 0x02,
	};
	enum class LightCaster
	{
		Direction = 0x01,
		Point = 0x02,
		Spotlight = 0x04
	};
	enum class LightMaps
	{
		Albedo = 0x01,
		Metallic = 0x02,
		Roughness = 0x04,
		AO = 0x10
	};
	enum class Effects
	{
		ShadowMapping = 0x01,
		IBL = 0x02,
		NormalMapping = 0x04,
		POM = 0x08,
		POMEdgeFix = 0x10,
		Bloom = 0x20
	};

	#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
	#define SHADER_DATA_T std::vector<std::string>

	// Uses Uber shader and sets specified macros for configuration when compiling (the shader)
	class ShaderPermutationsGenerator
	{
		public:
			/*
			element 0 is vertex shader
			element 1 is fragment shader
			element 2 is geometry shader
			 */

			static SHADER_DATA_T generateGBufferShader(ShaderInfo& p_ShaderInfo, ShaderType p_ShaderType);
			static SHADER_DATA_T generateShaderGBufferLightPass(ShaderInfo& p_ShaderInfo);
			static SHADER_DATA_T loadShader(std::string p_VertexShaderPath, std::string p_FragmentShaderPath);

		private:
			/*
			element 0 is vertex shader
			element 1 is fragment shader
			element 2 is geometry shader
			 */

			static SHADER_DATA_T m_Shader;
	};

	inline std::vector<std::string> ShaderPermutationsGenerator::loadShader(std::string p_VertexShaderPath, std::string p_FragmentShaderPath)
	{
		std::ifstream vs(p_VertexShaderPath);
		std::vector<std::string> shader(2);

		// vertex shader
		shader[0].assign((std::istreambuf_iterator<char>(vs)), (std::istreambuf_iterator<char>()) );

		std::ifstream fs(p_FragmentShaderPath);

		// fragment shader
		shader[1].assign((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));

		return shader;
	}

	inline std::vector<std::string> ShaderPermutationsGenerator::generateGBufferShader(ShaderInfo& p_ShaderInfo, ShaderType p_ShaderType)
	{
		std::vector<std::string> shader;
		std::string imageBindings;
		static std::vector<std::string> gbuffer_shader;
		int binding = 2;

		gbuffer_shader = loadShader("resources/Shaders/GBuffer.vert.glsl", "resources/Shaders/GBuffer.frag.glsl");

		shader.resize(gbuffer_shader.size());

		for (int i = 0; i < shader.size(); i++)
		{
			shader[i] += "#version 450\n";
			shader[i] += "#extension GL_ARB_separate_shader_objects : enable\n";
			shader[i] += "#extension GL_ARB_shading_language_include : enable\n";
			shader[i] += "#extension GL_GOOGLE_include_directive : enable\n";
		}

		// Using albedo map
		if (p_ShaderInfo.p_LightMaps & (uint32_t)LightMaps::Albedo)
		{
			shader[0] += "#define ALBEDO_MAP\n";
			shader[1] += "#define ALBEDO_MAP\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D albedoMap;\n";
			binding++;
		}

		// Using metallic map
		if (p_ShaderInfo.p_LightMaps & (uint32_t)LightMaps::Metallic)
		{
			shader[0] += "#define METALLIC_MAP\n";
			shader[1] += "#define METALLIC_MAP\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D metallicMap;\n";
			binding++;
		}

		// Using roughness map
		if (p_ShaderInfo.p_LightMaps & (uint32_t)LightMaps::Roughness)
		{
			shader[0] += "#define ROUGHNESS_MAP\n";
			shader[1] += "#define ROUGHNESS_MAP\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D roughnessMap;\n";
			binding++;
		}

		// Using AO map
		if (p_ShaderInfo.p_LightMaps & (uint32_t)LightMaps::AO)
		{
			shader[0] += "#define AO_MAP\n";
			shader[1] += "#define AO_MAP\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D aoMap;\n";
			binding++;
		}

		// Using normal mapping
		if (p_ShaderInfo.p_Effects & (uint32_t)Effects::NormalMapping)
		{
			shader[0] += "#define NORMAL_MAPPING\n";
			shader[1] += "#define NORMAL_MAPPING\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D normalMap;\n";
			binding++;
		}

		// Using parallax occlusion mapping
		if (p_ShaderInfo.p_Effects & (uint32_t)Effects::POM)
		{
			shader[0] += "#define PARALLAX_OCCLUSION_MAPPING\n";
			shader[1] += "#define PARALLAX_OCCLUSION_MAPPING\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D imageDisp;\n";
			binding++;
			// Edge fix for parallax occlusion mapping
			if (p_ShaderInfo.p_Effects & (uint32_t)Effects::POMEdgeFix)
			{

			}
		}

		// Using shadow mapping
		if (p_ShaderInfo.p_Effects & (uint32_t)Effects::ShadowMapping)
		{
			shader[0] += "#define SHADOW_MAPPING_PCF\n";
			shader[1] += "#define SHADOW_MAPPING_PCF\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D shadowMap;\n";
			binding++;
		}

		// Using Image Based Lighting
		if (p_ShaderInfo.p_Effects & (uint32_t)Effects::IBL)
		{
			shader[0] += "#define IBL\n";
			shader[1] += "#define IBL\n";
			shader[1] += "#define MAX_R_LOD " + std::to_string(p_ShaderInfo.p_MaxPrefilteredLod) + "\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform samplerCube irradianceMap;\n";
			binding++;
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform samplerCube preFilteredMap;\n";
			binding++;
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D brdfMap;\n";
			binding++;
		}

		//if (p_ShaderInfo.p_Effects & (uint32_t)Effects::Bloom)
		//{
		//	shader[0] += "#define Bloom\n";
		//	shader[1] += "#define Bloom\n";
		//	binding++;
		//}

		shader[1] += imageBindings;

		// Combine macros with uber shader
		for (int i = 0; i < gbuffer_shader.size(); i++)
			shader[i] += gbuffer_shader[i];

		return shader;
	}
	inline std::vector<std::string> ShaderPermutationsGenerator::generateShaderGBufferLightPass(ShaderInfo& p_ShaderInfo)
	{
		std::vector<std::string> shader;
		std::string imageBindings;
		static std::vector<std::string> ubershader;
		int binding = 6;

		// PBR
		ubershader = loadShader("data/Shaders/GBufferLightPass.vert.glsl", "data/Shaders/GBufferLightPass.frag.glsl");

		shader.resize(ubershader.size());

		for (int i = 0; i < shader.size(); i++)
		{
			shader[i] += "#version 450\n";
			shader[i] += "#extension GL_ARB_separate_shader_objects : enable\n";
			shader[i] += "#extension GL_ARB_shading_language_include : enable\n";
			shader[i] += "#extension GL_GOOGLE_include_directive : enable\n";
		}

		// Light caster = direction
		if (p_ShaderInfo.p_LightingType & (uint32_t)LightingType::PBR) {
			if (p_ShaderInfo.p_DirectionalLight)
				shader[1] += "#define LIGHT_DIR\n";
		}

		shader[1] += "#define SPOT_LIGHTS " + std::to_string(p_ShaderInfo.p_SpotLights) + "\n";


		shader[1] += "#define MAX_POINT_LIGHTS " + std::to_string(500) + "\n";

		// Using shadow mapping
		if (p_ShaderInfo.p_Effects & (uint32_t)Effects::ShadowMapping)
		{
			shader[0] += "#define SHADOW_MAPPING\n";
			shader[1] += "#define SHADOW_MAPPING\n";

			if (p_ShaderInfo.p_DirectionalLight) {
				imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D shadowMap_" + std::to_string(binding) + ";\n";
				shader[1] += "#define DIR_SHADOWMAP shadowMap_" + std::to_string(binding) + "\n";
				binding++;
			}

			for (int32_t i = 0; i < p_ShaderInfo.p_SpotLights; i++) {
				imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D shadowMap_" + std::to_string(binding) + ";\n";
				binding++;
			}

			for (int32_t i = 0; i < p_ShaderInfo.p_PointLights.size(); i++) {
				if (p_ShaderInfo.p_PointLights[i].m_ShadowMapID >= 0)
					imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform samplerCube shadowCube_" + std::to_string(binding) + ";\n";
				binding++;
			}
		}
		
		// Using Image Based Lighting
		if (p_ShaderInfo.p_Effects & (uint32_t)Effects::IBL)
		{
			shader[0] += "#define IBL\n";
			shader[1] += "#define IBL\n";
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform samplerCube irradianceMap;\n";
			binding++;
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform samplerCube preFilteredMap;\n";
			binding++;
			imageBindings += "layout(binding = " + Utils::to_string(binding) + ") uniform sampler2D brdfMap;\n";
			binding++;
		}
		
		// Using Image Based Lighting
		if (p_ShaderInfo.p_Effects & (uint32_t)Effects::IBL)
		{
			shader[0] += "#define Bloom\n";
			shader[1] += "#define Bloom\n";
			binding++;
		}

		shader[1] += imageBindings;

		// Combine macros with uber shader
		for (int i = 0; i < ubershader.size(); i++)
			shader[i] += ubershader[i];

		return shader;
	}

	inline uint32_t operator|(LightingType a, LightingType b)
	{
		return static_cast<uint32_t>(static_cast<int>(a) | static_cast<int>(b));
	}
	inline uint32_t operator|(LightMaps a, LightMaps b)
	{
		return static_cast<uint32_t>(static_cast<int>(a) | static_cast<int>(b));
	}
	inline uint32_t operator|(Effects a, Effects b)
	{
		return static_cast<uint32_t>(static_cast<int>(a) | static_cast<int>(b));
	}

}

/*
This is an example, using a 8-bit unsigned integer to store 8 flags:

unsigned char options;

It is common to use larger fields, e.g. 32 bits, but I use 8 here for simplicity.

The possible options, that can be turned on or off independently are declared in an enum like this (just using some arbitrary identifiers):
1
2
3
4
5
6
7
8
9
enum Options {
  OpAutoRedraw    = 0x01,
  OpAntiAlias     = 0x02,
  OpPixelShader   = 0x04,
  OpVertexShader  = 0x08,
  OpFullscreen    = 0x10,
  OpDaylight      = 0x20
  // ...
};

Note how each option is given a specific value. These values are carefully picked to match each bit in the 8-bit variable:
1
2
3
4
5
6
7
8
// 0x01 ==   1 == "00000001"
// 0x02 ==   2 == "00000010"
// 0x04 ==   4 == "00000100"
// 0x08 ==   8 == "00001000"
// 0x10 ==  16 == "00010000"
// 0x20 ==  32 == "00100000"
// 0x40 ==  64 == "01000000"
// 0x80 == 128 == "10000000"

Now, each flag can be set independently, by using the bitwise OR operator:
1
2
options = OpAutoRedraw | OpVertexShader | OpFullscreen;
// options == 0x01 | 0x08 | 0x10 == "00011001"

And can be tested using the bitwise AND operator:
1
2
if (options & OpAutoRedraw) {} // true
if (options & OpAntiAlias) {} // false


I hope this helps!
*/