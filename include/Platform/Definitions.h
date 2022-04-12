#pragma once

#ifdef VGL_USING_VULKAN
#include "Vulkan/VkDefinitions.h"
#elif defined VGL_USING_OPENGL
#include "OpenGL/GlDefinitions.h"
#endif

#include "../Math/Math.h"
#include "../Math/Matrix.h"

namespace vgl
{
	enum class glslType
	{
		Vertex,
		Fragment,
		Geometry
	};

	struct RenderInfo
	{
		bool p_AlphaBlending = false;
		CullMode p_CullMode = CullMode::BackBit;
		PolygonMode p_PolygonMode = PolygonMode::Fill;
		IATopoogy p_IATopology = IATopoogy::TriList;
	};
	enum class ShaderStage
	{
		None = -1,
		VertexBit = 0,
		FragmentBit = 1,
		GeometryBit = 2
	};
}