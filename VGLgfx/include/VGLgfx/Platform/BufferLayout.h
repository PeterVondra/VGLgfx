#pragma once

#include<iostream>

#include "../libs/imgui/imgui.h"
#include "../VGL_Logger.h"

namespace vgl
{
	enum class ShaderDataType
	{
		None = 0,
		Float, Vec2f, Vec3f, Vec4f, ImVec2f, ImVec4f,
		Int, Vec2i, Vec3i, Vec4i,
		Mat3f, Mat4f,
		Bool
	};

	struct BufferElement
	{
		static uint32_t getShaderDataTypeSize(ShaderDataType p_Type)
		{
			switch (p_Type)
			{
			case ShaderDataType::Float:		return sizeof(float);
			case ShaderDataType::Vec2f:		return sizeof(float) * 2;
			case ShaderDataType::Vec3f:		return sizeof(float) * 3;
			case ShaderDataType::Vec4f:		return sizeof(float) * 4;
			case ShaderDataType::ImVec2f:	return sizeof(ImVec2);
			case ShaderDataType::ImVec4f:	return sizeof(ImVec4);
			case ShaderDataType::Int:		return sizeof(int);
			case ShaderDataType::Vec2i:		return sizeof(int) * 2;
			case ShaderDataType::Vec3i:		return sizeof(int) * 3;
			case ShaderDataType::Vec4i:		return sizeof(int) * 4;
			case ShaderDataType::Mat3f:		return sizeof(float) * 3 * 3;
			case ShaderDataType::Mat4f:		return sizeof(float) * 4 * 4;
			case ShaderDataType::Bool:		return sizeof(int);
			}

			VGL_INTERNAL_WARNING("[BufferLayout]Unknown data type for buffer element");
			return 0;
		}

		inline ShaderDataType getShaderDataType()
		{
			return m_Type;
		}
		inline uint32_t getOffset()
		{
			return m_Offset;
		}
		inline uint32_t getLocation()
		{
			return m_Location;
		}

		BufferElement(ShaderDataType p_Type, const uint32_t p_Location, const std::string& p_Name) :
			m_Type(p_Type),
			m_Location(p_Location),
			m_Name(p_Name),
			m_Size(getShaderDataTypeSize(p_Type)),
			m_Offset(0),
			m_Normalized(false)
		{};

		BufferElement(ShaderDataType p_Type, const uint32_t p_Location, const std::string& p_Name, const bool p_Normalized) :
			m_Type(p_Type),
			m_Location(p_Location),
			m_Name(p_Name),
			m_Size(getShaderDataTypeSize(p_Type)),
			m_Offset(0),
			m_Normalized(p_Normalized)
		{};

	private:

		friend class BufferLayout;

		std::string m_Name;
		ShaderDataType m_Type;

		uint32_t m_Size;
		uint32_t m_Offset;
		uint32_t m_Location;

		const bool m_Normalized;
	};

	class BufferLayout
	{
		public:
			BufferLayout() : m_BindingIndex(0), m_Stride(0) {};
			BufferLayout(const std::initializer_list<BufferElement>& p_Elements, const uint32_t p_BindingIndex)
				: m_Elements(p_Elements), m_BindingIndex(p_BindingIndex), m_Stride(0)
			{
				for (auto& element : m_Elements)
				{
					element.m_Offset = m_Stride;
					m_Stride += element.m_Size;
				}
			};

			void push_back(BufferElement& p_Element);

			inline std::vector<BufferElement>& getElements()
			{
				return m_Elements;
			}
			inline uint32_t getStride()
			{
				return m_Stride;
			}
			inline uint32_t getBindingIndex()
			{
				return m_BindingIndex;
			}
		private:
			uint32_t m_Stride;
			uint32_t m_BindingIndex;

			std::vector<BufferElement> m_Elements;
	};
}