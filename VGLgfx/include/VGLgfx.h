#pragma once

#define VGL_INTERNAL_LOGGING_ENABLED
#define VGL_INTERNAL_LOG_LEVEL_TRACE
#define VGL_INTERNAL_ASSERT_ENABLED
#define VGL_INTERNAL_ASSERT_LEVEL_FATAL
#define VGL_INTERNAL_ASSERT_LEVEL_ERROR
#define VGL_INTERNAL_ASSERT_LEVEL_WARNING

#define VGL_RENDER_API_VULKAN
#define IMGUI_VK_IMPL

#include "Application/Application.h"
#include "Math/Camera.h"
#include "RenderPipelines.h"
#include "Scene.h"
#include "VGL-3D/Mesh/OBJ_Loader.h"