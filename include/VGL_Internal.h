#pragma once

#include "Application/RenderAPI.h"
#include "Platform/Definitions.h"

#ifdef VGL_USING_VULKAN
#include "Platform/VkIncludes.h"
#elif defined VLG_USING_OPENGL
#include "Platform/GlIncludes.h"
#endif