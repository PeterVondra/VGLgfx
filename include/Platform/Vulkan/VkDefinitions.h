#pragma once

#include<vulkan/vulkan.h>
#include "../../Math/Vector.h"


namespace vgl
{
	struct Viewport
	{
		Viewport() : m_Size(0), m_Position(0) {}
		Viewport(Vector2i p_Size, Vector2i p_Position) : m_Size(p_Size), m_Position(p_Position) {};
		Viewport(const Viewport& p_Viewport) { *this = p_Viewport; }

		friend class Pipeline;
		friend class CommandBuffer;

		Vector2i m_Size;
		Vector2i m_Position;

	};
	struct Scissor
	{
		Scissor() : m_Size(0), m_Position(0) {}
		Scissor(Vector2i p_Size, Vector2i p_Position) : m_Size(p_Size), m_Position(p_Position) {};
		Scissor(const Scissor& p_Scissor) { *this = p_Scissor; }

		Vector2i m_Size;
		Vector2i m_Position;

	};

	namespace vk{
		class RenderPass;
		class g_Pipeline;
		class g_PipelineInfo;
	}

	struct ShaderPackage
	{
		vk::RenderPass* p_RenderPass = nullptr;
		bool p_CreateGraphicsPipeline = false;
		vk::g_Pipeline* p_Pipeline = nullptr; // Use another pipeline
		vk::g_PipelineInfo p_GraphicsPipelineInfo;
		Shader* p_Shader;
	};

	enum class InputRate
	{
		Vertex = VK_VERTEX_INPUT_RATE_VERTEX,
		Instance = VK_VERTEX_INPUT_RATE_INSTANCE
	};

	enum class BorderColor
	{
		OpaqueWhite = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		OpaqueBlack = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		TransparentBlack = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK
	};

	enum class ImageFormat
	{
		Custom,
		D16UN = VK_FORMAT_D16_UNORM, 				// 16-bit depth unormalized
		D32SF = VK_FORMAT_D32_SFLOAT, 				// 32-bit depth floating-point
		C8SN_1C = VK_FORMAT_R8_SNORM, 				// 8-bit color signed normalized floating-point (1 channels)
		C8SN_2C = VK_FORMAT_R8G8_SNORM, 			// 8-bit color signed normalized floating-point (2 channels)
		C8SN_3C = VK_FORMAT_R8G8B8_SNORM, 			// 8-bit color signed normalized floating-point (3 channels)
		C8SN_4C = VK_FORMAT_R8G8B8A8_SNORM, 		// 8-bit color signed normalized floating-point (4 channels)
		C16SF_1C = VK_FORMAT_R16_SFLOAT, 			// 16-bit color floating-point (1 channels)
		C16SF_2C = VK_FORMAT_R16G16_SFLOAT, 		// 16-bit color floating-point (2 channels)
		C16SF_3C = VK_FORMAT_R16G16B16_SFLOAT, 		// 16-bit color floating-point (3 channels)
		C16SF_4C = VK_FORMAT_R16G16B16A16_SFLOAT, 	// 16-bit color floating-point (4 channels)
		C32SF_1C = VK_FORMAT_R32_SFLOAT, 			// 32-bit color floating-point (1 channels)
		C32SF_2C = VK_FORMAT_R32G32_SFLOAT, 		// 32-bit color floating-point (2 channels)
		C32SF_3C = VK_FORMAT_R32G32B32_SFLOAT, 		// 32-bit color floating-point (3 channels)
		C32SF_4C = VK_FORMAT_R32G32B32A32_SFLOAT, 	// 32-bit color floating-point (4 channels)

	};

	enum class CullMode
	{
		None = VK_CULL_MODE_NONE,
		BackBit = VK_CULL_MODE_BACK_BIT,
		FrontBit = VK_CULL_MODE_FRONT_BIT
	};
	enum class FrontFace
	{
		Default = VK_FRONT_FACE_CLOCKWISE,
		Clockwise = VK_FRONT_FACE_CLOCKWISE,
		CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE
	};
	enum class PolygonMode
	{
		Fill = VK_POLYGON_MODE_FILL,
		Line = VK_POLYGON_MODE_LINE,
		Point = VK_POLYGON_MODE_POINT
	};
	enum class IATopoogy
	{
		TriList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		TriStrip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		TriFan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		Line = VK_PRIMITIVE_TOPOLOGY_LINE_LIST
	};

	enum class PipelineBindPoint
	{
		Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
		Compute = VK_PIPELINE_BIND_POINT_COMPUTE
	};

	enum class SamplerMode
	{
		Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		ClampToBorder = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		MirroredClampToEdge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE

	};
	enum class Filter
	{
		Nearest = VK_FILTER_NEAREST,
		Linear = VK_FILTER_LINEAR
	};
	enum class Channels
	{
		None = 0,
		R = VK_FORMAT_R8_UNORM,
		RG = VK_FORMAT_R8G8_UNORM,
		RGB = VK_FORMAT_R8G8B8_UNORM,
		RGBA = VK_FORMAT_R8G8B8A8_UNORM
	};

	enum class AttachmentType
	{
		Color = 0,
		Depth = 1,
		Resolve = 2,
		Input = 3
	};
	enum class StoreOp
	{
		Null = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		Store = VK_ATTACHMENT_STORE_OP_STORE
	};
	enum class LoadOp
	{
		Clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
		Null = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		Load = VK_ATTACHMENT_LOAD_OP_LOAD
	};
	enum class Layout
	{
		Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
		Depth = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		DepthR = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		Color = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		ShaderR = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	enum class RenderPassType
	{
		Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
		Compute = VK_PIPELINE_BIND_POINT_COMPUTE
	};
}