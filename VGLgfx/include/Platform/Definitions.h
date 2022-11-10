#pragma once

#ifdef VGL_RENDER_API_VULKAN
#include "Vulkan/VkDefinitions.h"
#elif defined VGL_RENDER_API_OPENGL
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
}