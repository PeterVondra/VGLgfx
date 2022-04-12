#include "VkShader.h"

#include<iostream>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include "../../Utils/Logger.h"

namespace vgl
{
	namespace vk
	{
		Shader::Shader()
		{
			m_ContextPtr = &ContextSingleton::getInstance();
		}

		std::vector<char> Shader::readFile(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open())
				throw std::runtime_error("failed to open file " + filename);

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}
		Shader::Shader(std::string p_VertexShader, std::string p_FragmentShader, std::string p_GeometryShader)
		{
			m_ContextPtr = &ContextSingleton::getInstance();
		}

		Shader::Shader(std::string p_VertexShader, std::string p_FragmentShader)
		{
			m_ContextPtr = &ContextSingleton::getInstance();
		}
		Shader::~Shader()
		{

		}

		void Shader::destroy()
		{

		}

		void Shader::setShader(std::string p_VertexShader, std::string p_FragmentShader, std::string p_GeometryShader)
		{
			// Create shader modules and parse the pre-compiled SPIRV-shaders
			auto vertShaderCode = readFile(p_VertexShader);
			auto fragShaderCode = readFile(p_FragmentShader);

			if (vertShaderCode.empty() || fragShaderCode.empty())
			{
				#ifdef USE_LOGGING
				Utils::Logger::logMSG("shader code is nullptr", "Shader", Utils::Severity::Error);
				#endif
				return;
			}

			m_VertShaderModule = createShaderModule(vertShaderCode);
			m_FragShaderModule = createShaderModule(fragShaderCode);

			m_VertShaderStageInfo = {};
			m_VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

			m_VertShaderStageInfo.module = m_VertShaderModule;
			m_VertShaderStageInfo.pName = "main";

			m_FragShaderStageInfo = {};
			m_FragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_FragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

			m_FragShaderStageInfo.module = m_FragShaderModule;
			m_FragShaderStageInfo.pName = "main";
		}

		void Shader::setShader(std::string p_VertexShader, std::string p_FragmentShader)
		{
			// Create shader modules and parse the pre-compiled SPIRV-shaders
			std::vector<char> vertShaderCode = readFile(p_VertexShader);
			std::vector<char> fragShaderCode = readFile(p_FragmentShader);

			if (vertShaderCode.empty() || fragShaderCode.empty())
			{
				#ifdef USE_LOGGING
				Utils::Logger::logMSG("shader code is empty", "Shader", Utils::Severity::Error);
				#endif
				return;
			}

			m_VertShaderModule = createShaderModule(vertShaderCode);
			m_FragShaderModule = createShaderModule(fragShaderCode);

			m_VertShaderStageInfo = {};
			m_VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

			m_VertShaderStageInfo.module = m_VertShaderModule;
			m_VertShaderStageInfo.pName = "main";

			m_FragShaderStageInfo = {};
			m_FragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_FragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

			m_FragShaderStageInfo.module = m_FragShaderModule;
			m_FragShaderStageInfo.pName = "main";
		}
		void Shader::compile(std::vector<std::string> p_Shader)
		{
			
			m_VertShaderModule = createShaderModule(glslToSpirv(p_Shader[0], glslType::Vertex));
			m_FragShaderModule = createShaderModule(glslToSpirv(p_Shader[1], glslType::Fragment));
			
			m_VertShaderStageInfo = {};
			m_VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			
			m_VertShaderStageInfo.module = m_VertShaderModule;
			m_VertShaderStageInfo.pName = "main";
			
			m_FragShaderStageInfo = {};
			m_FragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			m_FragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			
			m_FragShaderStageInfo.module = m_FragShaderModule;
			m_FragShaderStageInfo.pName = "main";
		}

		std::vector<unsigned int> Shader::glslToSpirv(std::string& p_Code, glslType p_Type)
		{
			static bool initialized = false;
			if (!initialized)
			{
				glslang::InitializeProcess();
				initialized = true;
			}

			EShLanguage type;
			if (p_Type == glslType::Vertex)
				type = EShLangVertex;
			else if (p_Type == glslType::Fragment)
				type = EShLangFragment;
			
			glslang::TShader shader(type);
			const char* code = p_Code.c_str();
			shader.setStrings(&code, 1);

			int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
			glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_1;
			glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

			shader.setEnvInput(glslang::EShSourceGlsl, type, glslang::EShClientVulkan, ClientInputSemanticsVersion);
			shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
			shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

			TBuiltInResource resources;
			resources.maxLights = 32;
			resources.maxClipPlanes = 6;
			resources.maxTextureUnits = 32;
			resources.maxTextureCoords = 32;
			resources.maxVertexAttribs = 64;
			resources.maxVertexUniformComponents = 4096;
			resources.maxVaryingFloats = 64;
			resources.maxVertexTextureImageUnits = 32;
			resources.maxCombinedTextureImageUnits = 80;
			resources.maxTextureImageUnits = 32;
			resources.maxFragmentUniformComponents = 4096;
			resources.maxDrawBuffers = 32;
			resources.maxVertexUniformVectors = 128;
			resources.maxVaryingVectors = 8;
			resources.maxFragmentUniformVectors = 16;
			resources.maxVertexOutputVectors = 16;
			resources.maxFragmentInputVectors = 15;
			resources.minProgramTexelOffset = -8;
			resources.maxProgramTexelOffset = 7;
			resources.maxClipDistances = 8;
			resources.maxComputeWorkGroupCountX = 65535;
			resources.maxComputeWorkGroupCountY = 65535;
			resources.maxComputeWorkGroupCountZ = 65535;
			resources.maxComputeWorkGroupSizeX = 1024;
			resources.maxComputeWorkGroupSizeY = 1024;
			resources.maxComputeWorkGroupSizeZ = 64;
			resources.maxComputeUniformComponents = 1024;
			resources.maxComputeTextureImageUnits = 16;
			resources.maxComputeImageUniforms = 8;
			resources.maxComputeAtomicCounters = 8;
			resources.maxComputeAtomicCounterBuffers = 1;
			resources.maxVaryingComponents = 60;
			resources.maxVertexOutputComponents = 64;
			resources.maxGeometryInputComponents = 64;
			resources.maxGeometryOutputComponents = 128;
			resources.maxFragmentInputComponents = 128;
			resources.maxImageUnits = 8;
			resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
			resources.maxCombinedShaderOutputResources = 8;
			resources.maxImageSamples = 0;
			resources.maxVertexImageUniforms = 0;
			resources.maxTessControlImageUniforms = 0;
			resources.maxTessEvaluationImageUniforms = 0;
			resources.maxGeometryImageUniforms = 0;
			resources.maxFragmentImageUniforms = 8;
			resources.maxCombinedImageUniforms = 8;
			resources.maxGeometryTextureImageUnits = 16;
			resources.maxGeometryOutputVertices = 256;
			resources.maxGeometryTotalOutputComponents = 1024;
			resources.maxGeometryUniformComponents = 1024;
			resources.maxGeometryVaryingComponents = 64;
			resources.maxTessControlInputComponents = 128;
			resources.maxTessControlOutputComponents = 128;
			resources.maxTessControlTextureImageUnits = 16;
			resources.maxTessControlUniformComponents = 1024;
			resources.maxTessControlTotalOutputComponents = 4096;
			resources.maxTessEvaluationInputComponents = 128;
			resources.maxTessEvaluationOutputComponents = 128;
			resources.maxTessEvaluationTextureImageUnits = 16;
			resources.maxTessEvaluationUniformComponents = 1024;
			resources.maxTessPatchComponents = 120;
			resources.maxPatchVertices = 32;
			resources.maxTessGenLevel = 64;
			resources.maxViewports = 16;
			resources.maxVertexAtomicCounters = 0;
			resources.maxTessControlAtomicCounters = 0;
			resources.maxTessEvaluationAtomicCounters = 0;
			resources.maxGeometryAtomicCounters = 0;
			resources.maxFragmentAtomicCounters = 8;
			resources.maxCombinedAtomicCounters = 8;
			resources.maxAtomicCounterBindings = 1;
			resources.maxVertexAtomicCounterBuffers = 0;
			resources.maxTessControlAtomicCounterBuffers = 0;
			resources.maxTessEvaluationAtomicCounterBuffers = 0;
			resources.maxGeometryAtomicCounterBuffers = 0;
			resources.maxFragmentAtomicCounterBuffers = 1;
			resources.maxCombinedAtomicCounterBuffers = 1;
			resources.maxAtomicCounterBufferSize = 16384;
			resources.maxTransformFeedbackBuffers = 4;
			resources.maxTransformFeedbackInterleavedComponents = 64;
			resources.maxCullDistances = 8;
			resources.maxCombinedClipAndCullDistances = 8;
			resources.maxSamples = 4;
			resources.limits.nonInductiveForLoops = 1;
			resources.limits.whileLoops = 1;
			resources.limits.doWhileLoops = 1;
			resources.limits.generalUniformIndexing = 1;
			resources.limits.generalAttributeMatrixVectorIndexing = 1;
			resources.limits.generalVaryingIndexing = 1;
			resources.limits.generalSamplerIndexing = 1;
			resources.limits.generalVariableIndexing = 1;
			resources.limits.generalConstantMatrixVectorIndexing = 1;

			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

			DirStackFileIncluder Includer;
			std::string vPreprocessed;

			if (!shader.preprocess(&resources, ClientInputSemanticsVersion, ENoProfile, false, false, messages, &vPreprocessed, Includer))
			{
#ifdef USE_LOGGING
				Utils::Logger::logMSG("GLSL preprocessing failed", "GLSLANG", Utils::Severity::Error);
				Utils::Logger::logMSG(std::string(shader.getInfoLog()) + "\n", "GLSLANG", Utils::Severity::Error);
				Utils::Logger::logMSG(std::string(shader.getInfoDebugLog()) + "\n", "GLSLANG", Utils::Severity::Error);
#endif
			}

			const char* vPreprocessedC = vPreprocessed.c_str();

			shader.setStrings(&vPreprocessedC, 1);

			if (!shader.parse(&resources, ClientInputSemanticsVersion, false, messages))
			{
#ifdef USE_LOGGING
				Utils::Logger::logMSG("GLSL parsing failed", "GLSLANG", Utils::Severity::Error);
				Utils::Logger::logMSG(std::string(shader.getInfoLog()) + "\n", "GLSLANG", Utils::Severity::Error);
				Utils::Logger::logMSG(std::string(shader.getInfoDebugLog()) + "\n", "GLSLANG", Utils::Severity::Error);
#endif
			}

			glslang::TProgram program;
			program.addShader(&shader);

			if (!program.link(messages))
			{
#ifdef USE_LOGGING
				Utils::Logger::logMSG("GLSL linking failed", "GLSLANG", Utils::Severity::Error);
				Utils::Logger::logMSG(std::string(shader.getInfoLog()) + "\n", "GLSLANG", Utils::Severity::Error);
				Utils::Logger::logMSG(std::string(shader.getInfoDebugLog()) + "\n", "GLSLANG", Utils::Severity::Error);
#endif
			}

			std::vector<uint32_t> spirv;
			spv::SpvBuildLogger logger;
			glslang::SpvOptions spvOptions;
			glslang::GlslangToSpv(*program.getIntermediate(type), spirv, &spvOptions);

#ifdef USE_LOGGING
			if(p_Type == glslType::Vertex)
				Utils::Logger::logMSG("Vertex shader compilation", "GLSLANG", Utils::Result::Success);
			else if(p_Type == glslType::Fragment)
				Utils::Logger::logMSG("Fragment shader compilation", "GLSLANG", Utils::Result::Success);
#endif

			return spirv;
		}

		VkShaderModule Shader::createShaderModule(const std::vector<uint32_t>& p_Code)
		{
			// Construct the shader module
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = p_Code.size() * sizeof(uint32_t);
			createInfo.pCode = p_Code.data();

			VkShaderModule shaderModule;
			VkResult result = vkCreateShaderModule(m_ContextPtr->m_Device, &createInfo, nullptr, &shaderModule);

#ifdef USE_LOGGING
			if (result != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to create shader module", "Shader", Utils::Severity::Error);
#endif

			return shaderModule;
		}
		VkShaderModule Shader::createShaderModule(const std::vector<char>& p_Code)
		{
			// Construct the shader module
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = p_Code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(p_Code.data());

			VkShaderModule shaderModule;
			VkResult result = vkCreateShaderModule(m_ContextPtr->m_Device, &createInfo, nullptr, &shaderModule);

#ifdef USE_LOGGING
			if (result != VK_SUCCESS)
				Utils::Logger::logMSG("Failed to create shader module", "Shader", Utils::Severity::Error);
#endif

			return shaderModule;
		}
	}
}
