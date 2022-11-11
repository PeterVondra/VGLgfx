cmake_minimum_required(VERSION 3.0) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

ADD_CUSTOM_TARGET(debug
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the debug mode.")

ADD_CUSTOM_TARGET(release
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the release mode.")

### Require out-of-source builds
#file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
#if(EXISTS "${LOC_PATH}")
#    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
#endif()

project(VGLgfx)

add_compile_definitions(VGL_RENDER_API_VULKAN GLEW_STATIC IMGUI_UNLIMITED_FRAME_RATE)

set(
	HEADER_FILES

	include/VGLgfx.h

	include/Platform/VkIncludes.h
	#include/Platform/GLIncludes.h
	include/VGL_Internal.h
	include/VGL_Logger.h

	include/GFXDefinitions.h
	include/Platform/Definitions.h

	# Buffer layouts and handling
	include/Platform/BufferLayout.h
	
	# Base glfw window
	include/Platform/BaseWindow.h

	# Math library
	include/Math/Math.h
	include/Math/Matrix.h
	include/Math/Vector.h
	include/Math/Quaternion.h
	include/Math/Camera.h
	include/Math/Transform.h
	
	# General application handling, e.g. events, windows, renderAPI handling...
	include/Application/RenderAPI.h
	include/Application/InputManager.h
	include/Application/Application.h
	include/Application/Layer.h
	include/Application/LayerStack.h
	
	include/RenderPipelines.h
	include/Scene.h
	
	# Event system
	include/Events/Event.h
	include/Events/ApplicationEvent.h
	include/Events/KeyEvent.h
	include/Events/MouseEvent.h
	
	include/VGL-3D/Mesh/Material.h
	include/VGL-3D/Mesh/Mesh.h
	include/VGL-3D/Mesh/OBJ_Loader.h
	include/VGL-3D/Mesh/VGL-Assimp.h
	include/VGL-3D/Cuboid.h
	include/VGL-3D/Grid.h
	include/VGL-3D/Shapes.h
	include/VGL-3D/SkyBox.h
	
	# Utilites
	include/Utils/Logger.h
	include/Utils/FileGUI.h
	
	#Entity component system
	include/ECS/Component.h
	include/ECS/System.h
	include/ECS/ECS.h
	include/DefaultECSComponents.h
	
	include/Octree.h
	include/Font.h
	
	#include/Platform/OpenGL/GlWindow.h
	#include/Platform/OpenGL/GlRenderer.h
	#include/Platform/OpenGL/GlFramebufferAttachment.h
	#include/Platform/OpenGL/GlDefinitions.h
	#include/Platform/OpenGL/GlBuffers.h
	
	include/Platform/Vulkan/VkDescriptorLayoutCache.h
	include/Platform/Vulkan/VkContext.h
	include/Platform/Vulkan/VkWindow.h
	
	include/Platform/Vulkan/VkRenderer.h
	include/Platform/Vulkan/VkRenderPass.h
	include/Platform/Vulkan/VkDescriptor.h
	include/Platform/Vulkan/VkFramebuffer.h
	include/Platform/Vulkan/VkFramebufferAttachment.h
	include/Platform/Vulkan/VkDefinitions.h
	include/Platform/Vulkan/VkCommandBuffer.h 
	include/Platform/Vulkan/VkImage.h 
	include/Platform/Vulkan/VkBuffers.h
	include/Platform/Vulkan/VkShader.h
	include/Platform/Vulkan/VkGraphics_Internal.h
	include/Platform/Vulkan/VkImGui.h
)

set(
	SOURCE_FILES
	
	include/VGL_Logger.cpp

	# Buffer layouts and handling
	include/Platform/BufferLayout.cpp

	# Base glfw window
	include/Platform/BaseWindow.cpp

	# Math library
	include/Math/Math.cpp
	include/Math/Matrix.cpp
	include/Math/Vector.cpp
	include/Math/Quaternion.cpp
	include/Math/Camera.cpp
	include/Math/Transform.cpp
	
	# General application handling, e.g. events, windows, renderAPI
	include/Application/InputManager.cpp
	include/Application/Application.cpp
	include/Application/Layer.cpp
	include/Application/LayerStack.cpp

	include/RenderPipelines.cpp
	include/Scene.cpp

	# Event system
	include/Events/Event.cpp
	include/Events/ApplicationEvent.cpp
	include/Events/KeyEvent.cpp
	include/Events/MouseEvent.cpp

	# 3D
	include/VGL-3D/Mesh/Material.cpp
	include/VGL-3D/Mesh/Mesh.cpp
	include/VGL-3D/Mesh/OBJ_Loader.cpp
	include/VGL-3D/Cuboid.cpp
	include/VGL-3D/Grid.cpp
	include/VGL-3D/Shapes.cpp
	include/VGL-3D/SkyBox.cpp

	# Utilites
	include/Utils/Logger.h
	include/Utils/Logger.cpp
	include/Utils/FileGUI.h
	include/Utils/FileGUI.cpp

	# Entity component system
	include/ECS/Component.cpp
	include/ECS/System.cpp
	include/ECS/ECS.cpp

	include/Octree.cpp
	include/Font.cpp

	#include/Platform/OpenGL/GlWindow.cpp
	#include/Platform/OpenGL/GlRenderer.cpp
	#include/Platform/OpenGL/GlFramebufferAttachment.cpp
	#include/Platform/OpenGL/GlBuffers.cpp

	include/Platform/Vulkan/VkContext.cpp
	include/Platform/Vulkan/VkWindow.cpp
	include/Platform/Vulkan/VkRenderer.cpp
	include/Platform/Vulkan/VkRenderPass.cpp
	include/Platform/Vulkan/VkDescriptor.cpp
	include/Platform/Vulkan/VkFramebuffer.cpp
	include/Platform/Vulkan/VkFramebufferAttachment.cpp
	include/Platform/Vulkan/VkCommandBuffer.cpp 
	include/Platform/Vulkan/VkImage.cpp 
	include/Platform/Vulkan/VkBuffers.cpp
	include/Platform/Vulkan/VkShader.cpp
	include/Platform/Vulkan/VkGraphics_Internal.cpp
)

set(
	LIB_SRC_FILES 

	${PROJECT_SOURCE_DIR}/libs/PPK_ASSERT/ppk_assert.h
	${PROJECT_SOURCE_DIR}/libs/PPK_ASSERT/ppk_assert.cpp

	${PROJECT_SOURCE_DIR}/libs/VulkanMemoryAllocator/include/vk_mem_alloc.h
	${PROJECT_SOURCE_DIR}/libs/ImGuiFileDialog/dirent/dirent.h
	${PROJECT_SOURCE_DIR}/libs/ImGuiFileDialog/ImGuiFileDialog.h
	${PROJECT_SOURCE_DIR}/libs/ImGuiFileDialog/ImGuiFileDialog.cpp
	${PROJECT_SOURCE_DIR}/libs/imgui/imgui.h
	${PROJECT_SOURCE_DIR}/libs/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/libs/imgui/imconfig.h
	${PROJECT_SOURCE_DIR}/libs/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/libs/imgui/imgui_internal.h
	${PROJECT_SOURCE_DIR}/libs/imgui/imgui_tables.cpp
	${PROJECT_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/libs/imgui/imstb_rectpack.h
	${PROJECT_SOURCE_DIR}/libs/imgui/imstb_textedit.h
	${PROJECT_SOURCE_DIR}/libs/imgui/imstb_truetype.h
	${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_vulkan.h
	${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_vulkan.cpp
	${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.h
	${PROJECT_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp

	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImGuizmo.h
	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImGuizmo.cpp
	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImCurveEdit.h
	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImCurveEdit.cpp
	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImGradient.h
	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImGradient.cpp
	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImSequencer.h
	${PROJECT_SOURCE_DIR}/libs/ImGuizmo/ImSequencer.cpp
)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/glfw/)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/soil/)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/freetype/)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/glslang/)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/yaml-cpp/)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/spdlog/)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_library(VGLgfx STATIC ${HEADER_FILES} ${SOURCE_FILES} ${LIB_SRC_FILES})	
install(
	TARGETS VGLgfx
	PUBLIC_HEADER DESTINATION ${PROJECT_SOURCE_DIR}/build/VGLgfx/include/
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/build/
	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/build/VGLgfx/
	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/build/VGLgfx/
)

target_include_directories( 
	VGLgfx PRIVATE 
	Vulkan_INCLUDE_DIR		
	OpenGL_INCLUDE_DIR

	${PROJECT_SOURCE_DIR}/lib/glfw/include
	${PROJECT_SOURCE_DIR}/lib/soil/include
	${PROJECT_SOURCE_DIR}/lib/freetype/include
	${PROJECT_SOURCE_DIR}/lib/glslang
	${PROJECT_SOURCE_DIR}/lib/yaml-cpp/include
	${PROJECT_SOURCE_DIR}/lib/spdlog/include
)

target_link_libraries( 
	VGLgfx

	glfw
	Vulkan::Vulkan 
	#glew
	OpenGL::GL 
	soil 
	freetype 
	glslang 
	SPIRV 
	yaml-cpp
	spdlog
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test)

add_executable(
	VGLgfxTest

	${PROJECT_SOURCE_DIR}/test/VGLgfxTest.cpp
	data/CMakeResources/icon.rc
)

target_include_directories(VGLgfxTest PRIVATE ${PROJECT_SOURCE_DIR}/include/)

target_link_libraries(VGLgfxTest VGLgfx)