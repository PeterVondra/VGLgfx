#pragma once

#include "Platform/Definitions.h"

#ifdef VGL_RENDER_API_VULKAN
#define VGL_IMGUI_VK_IMPL
#include "Platform/VkIncludes.h"
#elif defined VGL_RENDER_API_OPENGL
#include "Platform/GlIncludes.h"
#endif

#include "Application/RenderAPI.h"