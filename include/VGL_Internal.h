#pragma once

#include "Platform/Definitions.h"

#ifdef VGL_USING_VULKAN
#include "Platform/VkIncludes.h"
#elif defined VGL_USING_OPENGL
#include "Platform/GlIncludes.h"
#endif

#include "Application/RenderAPI.h"