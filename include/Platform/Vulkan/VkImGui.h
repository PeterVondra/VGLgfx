#pragma once

#define NOMINMAX
#define IMGUI_VK_IMPL

#include "../../../lib/imgui/imgui.h"
#include "../../../lib/imgui/imgui_internal.h"
#include "../../../lib/imgui/examples/imgui_impl_glfw.h"
#include "../../../lib/imgui/examples/imgui_impl_vulkan.h"

#include "VkRenderPass.h"
#include "VkShader.h"
#include "VkCommandBuffer.h"
#include "VkWindow.h"

#include "../../Utils/Logger.h"

static uint32_t _ReverseInt(uint32_t i)
{
	unsigned char ch1, ch2, ch3, ch4;
	ch1 = i & 255;
	ch2 = (i >> 8) & 255;
	ch3 = (i >> 16) & 255; 
	ch4 = (i >> 24) & 255;
	return((int)ch1 << 24) + ((int)ch2 << 16) + ((int)ch3 << 8) + ch4;
}
static uint32_t BE_HEXTOU32(std::string str)
{
	std::istringstream converter(str);
	uint32_t value;
	converter >> std::hex >> value;
	return value;
}
static uint32_t LE_HEXTOU32(std::string str)
{
	std::istringstream converter(str);
	uint32_t value;
	converter >> std::hex >> value;
	return _ReverseInt(value);
}

namespace vgl
{
	inline static void DrawRowsBackground(int row_count, float line_height, float x1, float x2, float y_offset, ImU32 col_even, ImU32 col_odd)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		float y0 = ImGui::GetCursorScreenPos().y + (float)(int)y_offset;

		int row_display_start;
		int row_display_end;
		ImGui::CalcListClipping(row_count, line_height, &row_display_start, &row_display_end);
		for (int row_n = row_display_start; row_n < row_display_end; row_n++)
		{
			ImU32 col = (row_n & 1) ? col_odd : col_even;
			if ((col & IM_COL32_A_MASK) == 0)
				continue;
			float y1 = y0 + (line_height * row_n);
			float y2 = y1 + line_height;
			draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col);
		}
	}

	inline static void imGuiRowsBackground()
	{
		float x1 = ImGui::GetCurrentWindow()->WorkRect.Min.x;
		float x2 = ImGui::GetCurrentWindow()->WorkRect.Max.x;
		float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
		float item_offset_y = -item_spacing_y * 0.5f;
		float line_height = ImGui::GetTextLineHeight() + item_spacing_y;
		DrawRowsBackground(50, line_height, x1, x2, item_offset_y, 0, ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 0.5f)));
	}

	inline static void drawVec3Control(const std::string p_Label, Vector3f& p_Vec3f, float p_ResetValue = 0.0f, float p_ColumnWith = 100.0f)
	{
		ImGui::PushID(p_Label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, p_ColumnWith);
		ImGui::Text(p_Label.c_str());
		ImGui::NextColumn();
		ImGui::Spacing();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 2.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize))
			p_Vec3f.x = p_ResetValue;
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##X", &p_Vec3f.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.3f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
			p_Vec3f.y = p_ResetValue;
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &p_Vec3f.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
			p_Vec3f.z = p_ResetValue;
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &p_Vec3f.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PopStyleVar();
		ImGui::Spacing();

		ImGui::Columns(1);
		ImGui::PopID();
	}

	class EditorColorScheme
	{
		public:
			// 0xRRGGBBAA
			inline static uint32_t BackGroundColor = 0x252526FF;
			inline static uint32_t TextColor = 0xFFFFFF00;
			inline static uint32_t MainColor = BE_HEXTOU32("292929FF");
			inline static uint32_t MainAccentColor = 0x363636FF;
			inline static uint32_t HighlightColor = 0x393939FF;
			inline static uint32_t BorderColor = 0x292929FF;

		private:
			inline static uint32_t Black = 0x00000000;
			inline static uint32_t White = 0xFFFFFF00;

			inline static uint32_t AlphaTransparent = 0x00;
			inline static uint32_t Alpha20 = 0x33;
			inline static uint32_t Alpha40 = 0x66;
			inline static uint32_t Alpha50 = 0x80;
			inline static uint32_t Alpha60 = 0x99;
			inline static uint32_t Alpha80 = 0xCC;
			inline static uint32_t Alpha90 = 0xE6;
			inline static uint32_t AlphaFull = 0xFF;

			static float GetR(uint32_t colorCode) { return (float)((colorCode & 0xFF000000) >> 24) / (float)(0xFF); }
			static float GetG(uint32_t colorCode) { return (float)((colorCode & 0x00FF0000) >> 16) / (float)(0xFF); }
			static float GetB(uint32_t colorCode) { return (float)((colorCode & 0x0000FF00) >> 8) / (float)(0xFF); }
			static float GetA(uint32_t alphaCode) { return ((float)alphaCode / (float)0xFF); }

		public:
			inline static ImGuiTreeNodeFlags TreeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;
			inline static ImGuiTreeNodeFlags TreeNodeFlagsFramed = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;

			static ImVec4 GetColor(uint32_t c, uint32_t a = Alpha80) { return ImVec4(GetR(c), GetG(c), GetB(c), GetA(a)); }
			static ImVec4 Darken(ImVec4 c, float p) { return ImVec4(fmax(0.f, c.x - 1.0f * p), fmax(0.f, c.y - 1.0f * p), fmax(0.f, c.z - 1.0f * p), c.w); }
			static ImVec4 Lighten(ImVec4 c, float p) { return ImVec4(fmax(0.f, c.x + 1.0f * p), fmax(0.f, c.y + 1.0f * p), fmax(0.f, c.z + 1.0f * p), c.w); }

		private:
			static ImVec4 Disabled(ImVec4 c) { return Darken(c, 0.6f); }
			static ImVec4 Hovered(ImVec4 c) { return Lighten(c, 0.2f); }
			static ImVec4 Active(ImVec4 c) { return Lighten(ImVec4(c.x, c.y, c.z, 1.0f), 0.1f); }
			static ImVec4 Collapsed(ImVec4 c) { return Darken(c, 0.2f); }

		public:

			static void SetColors(uint32_t backGroundColor, uint32_t textColor, uint32_t mainColor, uint32_t mainAccentColor, uint32_t highlightColor)
			{
				BackGroundColor = backGroundColor;
				TextColor = textColor;
				MainColor = mainColor;
				MainAccentColor = mainAccentColor;
				HighlightColor = highlightColor;
			}

			static void ApplyTheme()
			{
				ImVec4* colors = ImGui::GetStyle().Colors;

				colors[ImGuiCol_Text] = GetColor(TextColor);
				colors[ImGuiCol_TextDisabled] = Disabled(colors[ImGuiCol_Text]);
				colors[ImGuiCol_WindowBg] = GetColor(BackGroundColor, Alpha90);
				colors[ImGuiCol_ChildBg] = GetColor(BackGroundColor, Alpha90);
				colors[ImGuiCol_PopupBg] = GetColor(BackGroundColor, Alpha90);
				colors[ImGuiCol_Border] = GetColor(BorderColor);
				colors[ImGuiCol_BorderShadow] = Darken(GetColor(BorderColor), 0);
				colors[ImGuiCol_FrameBg] = GetColor(MainAccentColor, AlphaFull);
				colors[ImGuiCol_FrameBgHovered] = Hovered(colors[ImGuiCol_FrameBg]);
				colors[ImGuiCol_FrameBgActive] = Active(colors[ImGuiCol_FrameBg]);
				colors[ImGuiCol_TitleBg] = GetColor(BackGroundColor, AlphaFull);
				colors[ImGuiCol_TitleBgActive] = Active(colors[ImGuiCol_TitleBg]);
				colors[ImGuiCol_TitleBgCollapsed] = Collapsed(colors[ImGuiCol_TitleBg]);
				colors[ImGuiCol_MenuBarBg] = Darken(GetColor(BackGroundColor), Alpha90);
				colors[ImGuiCol_ScrollbarBg] = Lighten(GetColor(BackGroundColor, Alpha50), 0.4f);
				colors[ImGuiCol_ScrollbarGrab] = Lighten(GetColor(BackGroundColor), 0.3f);
				colors[ImGuiCol_ScrollbarGrabHovered] = Hovered(colors[ImGuiCol_ScrollbarGrab]);
				colors[ImGuiCol_ScrollbarGrabActive] = Active(colors[ImGuiCol_ScrollbarGrab]);
				colors[ImGuiCol_CheckMark] = GetColor(HighlightColor);
				colors[ImGuiCol_SliderGrab] = GetColor(HighlightColor);
				colors[ImGuiCol_SliderGrabActive] = Active(colors[ImGuiCol_SliderGrab]);
				colors[ImGuiCol_Button] = GetColor(MainAccentColor, Alpha80);
				colors[ImGuiCol_ButtonHovered] = Hovered(colors[ImGuiCol_Button]);
				colors[ImGuiCol_ButtonActive] = Active(colors[ImGuiCol_Button]);
				colors[ImGuiCol_Header] = GetColor(MainColor, Alpha80);
				colors[ImGuiCol_HeaderHovered] = Hovered(colors[ImGuiCol_Header]);
				colors[ImGuiCol_HeaderActive] = Active(colors[ImGuiCol_Header]);
				colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
				colors[ImGuiCol_SeparatorHovered] = Hovered(colors[ImGuiCol_Separator]);
				colors[ImGuiCol_SeparatorActive] = Active(colors[ImGuiCol_Separator]);
				colors[ImGuiCol_ResizeGrip] = GetColor(MainColor, Alpha20);
				colors[ImGuiCol_ResizeGripHovered] = Hovered(colors[ImGuiCol_ResizeGrip]);
				colors[ImGuiCol_ResizeGripActive] = Active(colors[ImGuiCol_ResizeGrip]);
				colors[ImGuiCol_Tab] = Lighten(GetColor(BackGroundColor), 0.4f);
				colors[ImGuiCol_TabHovered] = Hovered(GetColor(MainColor, Alpha60));
				colors[ImGuiCol_TabActive] = Active(GetColor(MainColor, Alpha60));
				colors[ImGuiCol_TabUnfocused] = Lighten(GetColor(BackGroundColor), 0.4f);
				colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabActive];
				colors[ImGuiCol_DockingPreview] = Darken(colors[ImGuiCol_HeaderActive], 0.2f);
				colors[ImGuiCol_DockingEmptyBg] = Darken(colors[ImGuiCol_HeaderActive], 0.6f);
				colors[ImGuiCol_PlotLines] = GetColor(HighlightColor);
				colors[ImGuiCol_PlotLinesHovered] = Hovered(colors[ImGuiCol_PlotLines]);
				colors[ImGuiCol_PlotHistogram] = GetColor(HighlightColor);
				colors[ImGuiCol_PlotHistogramHovered] = Hovered(colors[ImGuiCol_PlotHistogram]);
				colors[ImGuiCol_TextSelectedBg] = GetColor(HighlightColor, Alpha40);
				colors[ImGuiCol_DragDropTarget] = GetColor(HighlightColor, Alpha80);;
				colors[ImGuiCol_NavHighlight] = GetColor(White);
				colors[ImGuiCol_NavWindowingHighlight] = GetColor(White, Alpha80);
				colors[ImGuiCol_NavWindowingDimBg] = GetColor(White, Alpha20);
				colors[ImGuiCol_ModalWindowDimBg] = GetColor(Black, Alpha60);

				ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_Right;
			}
	};

	namespace vk
	{
		struct ImguiPushConstantBlock
		{
			Vector2f scale;
			Vector2f translate;
		};

#ifdef IMGUI_VK_IMPL
		class ImGuiContext
		{
			public:
				ImGuiContext() : m_ContextPtr(&ContextSingleton::getInstance()), m_CommandBuffer(Level::Secondary) {}
				inline ImGuiContext(Window* p_Window) 
					: m_WindowPtr(p_Window), m_ContextPtr(&ContextSingleton::getInstance()), m_CommandBuffer(Level::Secondary)
				{
					IMGUI_CHECKVERSION();
					ImGui::CreateContext();

					// Color scheme
					ImGuiStyle& st = ImGui::GetStyle();
					st.FrameBorderSize = 0.0f;
					st.FramePadding = ImVec2(3.0f, 1.0f);
					st.ItemSpacing = ImVec2(8.0f, 2.0f);
					st.WindowBorderSize = 0.1f;
					st.TabBorderSize = 0.0f;
					st.WindowRounding = 0.0f;
					st.ChildRounding = 2.0f;
					st.FrameRounding = 1.0f;
					st.ScrollbarRounding = 1.0f;
					st.GrabRounding = 2.0f;
					st.TabRounding = 0.0f;
					st.ChildBorderSize = 0.3f;
					st.ScaleAllSizes(0.8);

					//EditorColorScheme::ApplyTheme();
					auto& colors = ImGui::GetStyle().Colors;
					colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

					// Headers
					colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
					colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
					colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

					// Buttons
					colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
					colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
					colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

					// Frame BG
					colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
					colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
					colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

					// Tabs
					colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
					colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
					colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
					colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
					colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

					// Title
					colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
					colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
					colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

					// Dimensions
					ImGuiIO& io = ImGui::GetIO();
					io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
					io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
					io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       	// Not implemented yet (In official ImGui)
					io.FontGlobalScale = 0.95;

					io.Fonts->AddFontFromFileTTF("resources/Fonts/OpenSans-Regular.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesDefault());
				}

				~ImGuiContext() {};

				struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

				inline void updateViewports()
				{
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
				}

				inline void init(RenderPass& p_RenderPass)
				{
					VkDescriptorPoolSize pool_sizes[] =
					{
						{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
						{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
						{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
						{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
						{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
						{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
						{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
						{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
						{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
						{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
						{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
					};

					//Size of pool to limit size
					VkDescriptorPoolCreateInfo poolInfo = {};
					poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
					poolInfo.poolSizeCount = 11;
					poolInfo.pPoolSizes = pool_sizes;
					poolInfo.maxSets = static_cast<uint32_t>(2);

					if (vkCreateDescriptorPool(m_ContextPtr->m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
						Utils::Logger::logMSG("Failed to create descriptor pool\n", "VkImGui", Utils::Severity::Error);
					else
						Utils::Logger::logMSG("success!, created descriptor pool\n", "VkImGui", Utils::Severity::Debug);

					ImGui_ImplGlfw_InitForVulkan(m_WindowPtr->getGLFWWindow(), true);
					ImGui_ImplVulkan_InitInfo init_info = {};
					init_info.Instance = m_ContextPtr->m_Instance;
					init_info.PhysicalDevice = m_ContextPtr->m_PhysicalDevice.m_VkHandle;
					init_info.Device = m_ContextPtr->m_Device;
					init_info.QueueFamily = m_ContextPtr->m_PhysicalDevice.m_QueueFamily.graphicsFamily;
					init_info.Queue = m_ContextPtr->m_GraphicsQueue;
					init_info.PipelineCache = VK_NULL_HANDLE;
					init_info.DescriptorPool = m_DescriptorPool;
					init_info.Allocator = nullptr;
					init_info.MinImageCount = 2;
					init_info.ImageCount = 2;
					init_info.CheckVkResultFn = VK_NULL_HANDLE;
					ImGui_ImplVulkan_Init(&init_info, p_RenderPass.m_RenderPass);

					m_RenderPass = &p_RenderPass;

					auto cmd = CommandBuffer::beginSingleTimeCmds();
					ImGui_ImplVulkan_CreateFontsTexture(cmd.vkHandle());
					CommandBuffer::endSingleTimeCmds(cmd);      

				}

				void recreateSwapchain()
				{

				}

			private:
				friend class Renderer;

				Window* m_WindowPtr;

				Context* m_ContextPtr;
				VkDescriptorPool m_DescriptorPool;
				CommandBuffer m_CommandBuffer;
				RenderPass* m_RenderPass;
		};
#endif
#ifndef IMGUI_VK_IMPL
		class ImGuiContext
		{
		public:
			ImGuiContext() : commandBuffer(Level::Secondary) {}//, renderPass(RenderPassType::Graphics) {};
			inline ImGuiContext(Window* p_CurrentWindow, VkCommandBufferInheritanceInfo* p_InheritanceInfo)
				: commandBuffer(Level::Secondary), inheritanceInfo(p_InheritanceInfo), currentWindow(p_CurrentWindow), m_ContextPtr(&ContextSingleton::getInstance())//, renderPass(RenderPassType::Graphics)
			{
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();

				// Color scheme
				ImGuiStyle& st = ImGui::GetStyle();
				//st.FrameBorderSize = 0.0f;
				//st.FramePadding = ImVec2(3.0f, 1.0f);
				//st.ItemSpacing = ImVec2(8.0f, 2.0f);
				//st.WindowBorderSize = 0.1f;
				//st.TabBorderSize = 0.0f;
				//st.WindowRounding = 0.0f;
				//st.ChildRounding = 2.0f;
				//st.FrameRounding = 1.0f;
				//st.ScrollbarRounding = 1.0f;
				//st.GrabRounding = 2.0f;
				//st.TabRounding = 0.0f;
				//st.ChildBorderSize = 0.3f;
				st.ScaleAllSizes(0.8);

				//EditorColorScheme::ApplyTheme();
				auto& colors = ImGui::GetStyle().Colors;
				colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

				// Headers
				colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
				colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
				colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

				// Buttons
				colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
				colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
				colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

				// Frame BG
				colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
				colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
				colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

				// Tabs
				colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
				colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
				colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
				colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
				colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

				// Title
				colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
				colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
				colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

				// Dimensions
				ImGuiIO& io = ImGui::GetIO();
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
				io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
				io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Not implemented yet
				io.FontGlobalScale = 0.95;

				io.Fonts->AddFontFromFileTTF("data/Fonts/OpenSans-Regular.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesDefault());
			}
			~ImGuiContext() {};

			struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

			inline void updateViewports()
			{
				//ImGui::UpdatePlatformWindows();
				//ImGui::RenderPlatformWindowsDefault();
			}

			inline void init(RenderPass& p_RenderPass)
			{
				renderPass = &p_RenderPass;
				ImGui_ImplGlfw_InitForVulkan(currentWindow->getGLFWWindow(), true);

				Vector2i size;
				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->GetTexDataAsRGBA32(&fontData, &size.x, &size.y);

				imguiImg.initImage(size, fontData, Channels::RGBA, SamplerMode::ClampToEdge);

				imguiUniforms.setUniform("font", imguiImg, 0);
				imguiUniforms.create();

				vao.fill(vertices, indices);

				PipelineInfo pipelineInfo;
				pipelineInfo.p_CullMode = CullMode::None;
				pipelineInfo.p_FrontFace = FrontFace::CounterClockwise;
				pipelineInfo.p_MSAASamples = currentWindow->maxMSAASamples;
				pipelineInfo.p_Viewport = Viewport(currentWindow->m_WindowSize, { 0, 0 });
				pipelineInfo.p_Scissor = Scissor(currentWindow->m_WindowSize, { 0, 0 });
				pipelineInfo.p_SampleRateShading = false;
				pipelineInfo.p_PolygonMode = PolygonMode::Fill;
				pipelineInfo.p_IATopology = IATopoogy::TriList;
				pipelineInfo.p_AlphaBlending = true;
				pipelineInfo.p_UsePushConstants = true;
				pipelineInfo.p_PushConstantSize = sizeof(ImguiPushConstantBlock);
				pipelineInfo.p_PushConstantOffset = 0;
				pipelineInfo.p_PushConstantShaderStage = VK_SHADER_STAGE_VERTEX_BIT;

				imguiShader.setShader("data/Shaders/Imgui/vert.spv", "data/Shaders/Imgui/frag.spv");
				pipelineInfo.p_Shader = &imguiShader;
				pipelineInfo.p_RenderPass = renderPass;

				_ImguiLayout = { {
						{ ShaderDataType::ImVec2f, 0, "inPosition" },
						{ ShaderDataType::ImVec2f, 1, "inUv" },
						{ ShaderDataType::ImVec4f, 2, "inColor" }
					},0
				};

				vertices.setLayout(_ImguiLayout);

				pipelineInfo.p_DescriptorSetLayout = UniformManager::createDscSetLayout(UniformManager::createDscSetLayoutBindings(0, 0, 0, 1));
				pipelineInfo.p_AttributeDescription.push_back(VertexBuffer::getAttributes(&_ImguiLayout).first);
				pipelineInfo.p_BindingDescription.push_back(VertexBuffer::getAttributes(&_ImguiLayout).second);

				imguiPipeline.create(pipelineInfo);
			}

			inline void genCmdBuffers()
			{
				ImGuiIO& io = ImGui::GetIO();

				if (currentWindow->windowResized())
					io.DisplaySize = ImVec2(currentWindow->getWindowSize().x, currentWindow->getWindowSize().y);

				ImDrawData* imDrawData = ImGui::GetDrawData();

				uint32_t vtxOffset = 0;
				uint32_t idxOffset = 0;

				pushConstBlock.scale = Vector2f(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
				pushConstBlock.translate = Vector2f(-1.0f);

				commandBuffer.cmdBegin(*inheritanceInfo);
				commandBuffer.cmdSetViewport(Viewport({ (int)io.DisplaySize.x, (int)io.DisplaySize.y }, { 0, 0 }));

				vkCmdPushConstants(commandBuffer.m_CommandBuffer, imguiPipeline.m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ImguiPushConstantBlock), &pushConstBlock);

				if (imDrawData)
				{
					if (imDrawData->CmdListsCount > 0) {
						commandBuffer.cmdBindPipeline(imguiPipeline);
						commandBuffer.cmdBindUniformBuffer(imguiUniforms);
						commandBuffer.cmdBindVertexArray16BitIdx(vao);

						for (uint32_t i = 0; i < imDrawData->CmdListsCount; i++)
						{
							const ImDrawList* cmdList = imDrawData->CmdLists[i];
							for (uint32_t j = 0; j < cmdList->CmdBuffer.Size; j++)
							{
								const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];

								if (pcmd->TextureId) {
									Image* img = (Image*)pcmd->TextureId;
									if (img->isComplete()) {
										VkDescriptorSet* dsc = (VkDescriptorSet*)&img->id;
										vkCmdBindDescriptorSets(
											commandBuffer.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
											imguiPipeline.m_PipelineLayout, 0, 1, dsc, 0, nullptr
										);
									}
								}
								else
									commandBuffer.cmdBindUniformBuffer(imguiUniforms);

								commandBuffer.cmdSetScissor(Scissor(
									{ (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y) },
									{ std::max((int32_t)(pcmd->ClipRect.x), 0), std::max((int32_t)(pcmd->ClipRect.y), 0) }
								));

								commandBuffer.cmdDrawIndexed(vtxOffset, idxOffset, idxOffset + pcmd->ElemCount);
								idxOffset += pcmd->ElemCount;
							}
							vtxOffset += cmdList->VtxBuffer.Size;
						}
					}
				}

				commandBuffer.cmdEnd();

			}

			inline void updateBuffers()
			{
				ImDrawData* imDrawData = ImGui::GetDrawData();

				if (!imDrawData)
					return;

				if (imDrawData->TotalVtxCount == 0 || imDrawData->TotalIdxCount == 0)
					return;

				vtxBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
				idxBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

				if (vtxCount != imDrawData->TotalVtxCount) {
					vtxCount = imDrawData->TotalVtxCount;
					vertices.allocateHV(vtxBufferSize); // Host visible buffer
				}
				if (idxCount != imDrawData->TotalIdxCount) {
					idxCount = imDrawData->TotalIdxCount;
					indices.allocateHV(idxBufferSize);
				}

				uint32_t VtxOffset = 0;
				uint32_t IdxOffset = 0;
				uint32_t idCount = 0;
				for (uint32_t i = 0; i < imDrawData->CmdListsCount; i++)
				{
					const ImDrawList* cmdList = imDrawData->CmdLists[i];
					vertices.copy<ImDrawVert>(cmdList->VtxBuffer.Data, VtxOffset, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
					indices.copy<ImDrawIdx>(cmdList->IdxBuffer.Data, IdxOffset, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
					VtxOffset += cmdList->VtxBuffer.Size;
					IdxOffset += cmdList->IdxBuffer.Size;
				}

				vertices.flush();
				vao.update(0);

				indices.flush();
			}

		private:
			friend class Renderer;
			friend class BaseRenderer;
			friend class ForwardRenderer;
			friend class DeferredRenderer;

			ImguiPushConstantBlock pushConstBlock;

			Window* currentWindow;
			VertexArray		vao;
			IndexBuffer		indices;
			VertexBuffer	vertices;
			Image			imguiImg;
			UniformManager	imguiUniforms;
			struct TextureIDs { UniformManager mng; ImTextureID ID; };
			std::vector<TextureIDs>	imguiImageUniforms;
			Pipeline		imguiPipeline;
			Shader			imguiShader;
			VkDeviceSize	vtxBufferSize = 0;
			VkDeviceSize	vtxCount = 0;
			VkDeviceSize	idxBufferSize = 0;
			VkDeviceSize	idxCount = 0;
			RenderPass* renderPass;
			BufferLayout _ImguiLayout;
			unsigned char* fontData;

			VkCommandBufferInheritanceInfo* inheritanceInfo;
			CommandBuffer	commandBuffer;
			std::vector<ImguiPushConstantBlock> imguiPushConstants;
			Context* m_ContextPtr;
			VkDescriptorPool descriptorPool;
		};
#endif
	}
}