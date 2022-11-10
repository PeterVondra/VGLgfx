#pragma once

#include <GL/glew.h>
#include "../../Math/Vector.h"

namespace vgl
{
	struct Viewport
	{
		Viewport(Vector2i p_Size, Vector2i p_Position) : m_Size(p_Size), m_Position(p_Position) {};
		Viewport(const Viewport& p_Viewport) { *this = p_Viewport; }
	private:
		friend class Pipeline;
		friend class CommandBuffer;

		Vector2i m_Size;
		Vector2i m_Position;

	};
	struct Scissor
	{
		Scissor(Vector2i p_Size, Vector2i p_Position) : m_Size(p_Size), m_Position(p_Position) {};
		Scissor(const Scissor& p_Scissor) { *this = p_Scissor; }
	private:
		friend class Pipeline;
		friend class CommandBuffer;

		Vector2i m_Size;
		Vector2i m_Position;

	};

	enum class InputRate
	{
		Vertex = 1,
		Instance = 2
	};

	enum class CullMode
	{
		None = 0,
		BackBit = GL_BACK,
		FrontBit = GL_FRONT
	};
	enum class FrontFace
	{
		Default = GL_CW,
		Clockwise = GL_CW,
		CounterClockwise = GL_CCW
	};
	enum class PolygonMode
	{
		Fill = GL_FILL,
		Line = GL_LINE,
		Point = GL_POINT
	};
	enum class IATopoogy
	{
		TriList = GL_TRIANGLES,
		TriStrip = GL_TRIANGLE_STRIP,
		TriFan = GL_TRIANGLE_FAN,
		Line = GL_LINE
	};

	enum class SamplerMode
	{
		Repeat = GL_REPEAT,
		MirroredRepeat = GL_MIRRORED_REPEAT,
		ClampToEdge = GL_CLAMP_TO_EDGE,
		ClampToBorder = GL_CLAMP_TO_BORDER,
		MirroredClampToEdge = GL_MIRROR_CLAMP_TO_EDGE

	};
	enum class Filter
	{
		Nearest = GL_NEAREST,
		Linear = GL_LINEAR
	};
	enum class Channels
	{
		None = 0,
		R = GL_R8,
		RG = GL_RG8,
		RGB = GL_RGB8,
		RGBA = GL_RGBA8
	};
}