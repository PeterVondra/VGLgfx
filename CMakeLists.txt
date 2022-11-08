cmake_minimum_required(VERSION 3.0) 
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(UNIX)
ADD_CUSTOM_TARGET(debug
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the debug mode.")

ADD_CUSTOM_TARGET(release
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the release mode.")
endif(UNIX)

project(VGL-ENGINE)

add_compile_definitions(VGL_USING_VULKAN GLEW_STATIC IMGUI_UNLIMITED_FRAME_RATE)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)

#add_subdirectory(${PROJECT_SOURCE_DIR}/lib/glew-cmake/build/cmake/)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/glfw-3.3.8/)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/soil/)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/freetype/)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/glslang/)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/yaml-cpp/)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(Vulkan_FOUND)
	message(STATUS "Vulkan found")
endif(Vulkan_FOUND)

if(OpenGL_FOUND)
	message(STATUS "OpenGL found")
endif(OpenGL_FOUND)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

set(
	headers
	
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
	
	
	#Entity component system
	include/ECS/Component.h
	include/ECS/System.h
	include/ECS/ECS.h
	include/DefaultECSComponents.h
	
	include/Octree.h
	include/Font.h
	
	# Buffer layouts and handling
	include/BufferLayout.h
	
	include/Platform/BaseWindow.h
	
	#include/Platform/OpenGL/GlWindow.h
	#include/Platform/OpenGL/GlRenderer.h
	#include/Platform/OpenGL/GlFramebufferAttachment.h
	#include/Platform/OpenGL/GlDefinitions.h
	#include/Platform/OpenGL/GlBuffers.h
	
	include/Platform/Vulkan/VkDescriptorLayoutCache.h
	include/Platform/Vulkan/VkContext.h
	include/Platform/Vulkan/VkWindow.h
	include/Platform/Vulkan/VkRenderInfo.h
	include/Platform/Vulkan/VkRenderer.h
	include/Platform/Vulkan/VkRenderPass.h
	include/Platform/Vulkan/VkDescriptorSetManager.h
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
	sources

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

	include/VGL-3D/Mesh/Material.cpp
	include/VGL-3D/Mesh/Mesh.cpp
	include/VGL-3D/Mesh/OBJ_Loader.cpp
	include/VGL-3D/Cuboid.cpp
	include/VGL-3D/Grid.cpp
	include/VGL-3D/Shapes.cpp
	include/VGL-3D/SkyBox.cpp

	# Entity component system
	include/ECS/Component.cpp
	include/ECS/System.cpp
	include/ECS/ECS.cpp

	include/Octree.cpp
	include/Font.cpp

	# Buffer layouts and handling
	include/BufferLayout.cpp

	include/Platform/BaseWindow.cpp

	#include/Platform/OpenGL/GlWindow.cpp
	#include/Platform/OpenGL/GlRenderer.cpp
	#include/Platform/OpenGL/GlFramebufferAttachment.cpp
	#include/Platform/OpenGL/GlBuffers.cpp

	include/Platform/Vulkan/VkContext.cpp
	include/Platform/Vulkan/VkWindow.cpp
	include/Platform/Vulkan/VkRenderer.cpp
	include/Platform/Vulkan/VkRenderPass.cpp
	include/Platform/Vulkan/VkDescriptorSetManager.cpp
	include/Platform/Vulkan/VkFramebuffer.cpp
	include/Platform/Vulkan/VkFramebufferAttachment.cpp
	include/Platform/Vulkan/VkCommandBuffer.cpp 
	include/Platform/Vulkan/VkImage.cpp 
	include/Platform/Vulkan/VkBuffers.cpp
	include/Platform/Vulkan/VkShader.cpp
	include/Platform/Vulkan/VkGraphics_Internal.cpp
)

add_executable(
	vgl

	main.cpp

	Editor/Editor.h
	Editor/Editor.cpp

	include/Platform/VkIncludes.h
	#include/Platform/GLIncludes.h
	include/VGL_Internal.h

	${headers}
	${sources}

	lib/VulkanMemoryAllocator/src/vk_mem_alloc.h

	include/GDefinitions.h
	include/Platform/Definitions.h

	include/Utils/ThreadPool.h
	include/Utils/Logger.h
	include/Utils/Logger.cpp
	include/Utils/Cache.h
	include/Utils/FileGUI.h
	include/Utils/FileGUI.cpp

	data/CMakeResources/icon.rc

	lib/ImGuiFileDialog/dirent.h
	lib/ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h
	lib/ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.cpp
	lib/imgui/imgui.h
	lib/imgui/imgui.cpp
	lib/imgui/imconfig.h
	lib/imgui/imgui_demo.cpp
	lib/imgui/imgui_draw.cpp
	lib/imgui/imgui_internal.h
	lib/imgui/imgui_tables.cpp
	lib/imgui/imgui_widgets.cpp
	lib/imgui/imstb_rectpack.h
	lib/imgui/imstb_textedit.h
	lib/imgui/imstb_truetype.h
	lib/imgui/backends/imgui_impl_vulkan.h
	lib/imgui/backends/imgui_impl_vulkan.cpp
	lib/imgui/backends/imgui_impl_glfw.h
	lib/imgui/backends/imgui_impl_glfw.cpp

	lib/ImGuizmo/ImGuizmo.h
	lib/ImGuizmo/ImGuizmo.cpp
	lib/ImGuizmo/ImCurveEdit.h
	lib/ImGuizmo/ImCurveEdit.cpp
	lib/ImGuizmo/ImGradient.h
	lib/ImGuizmo/ImGradient.cpp
	lib/ImGuizmo/ImSequencer.h
	lib/ImGuizmo/ImSequencer.cpp

	#lib/ImGuiColorTextEdit/TextEditor.h
	#lib/ImGuiColorTextEdit/TextEditor.cppy
)

#if(WIN32)
	target_include_directories( 
		vgl PRIVATE 
		Vulkan_INCLUDE_DIR
    #${PROJECT_SOURCE_DIR}/lib/glew-cmake/include		
		OpenGL_INCLUDE_DIR

		${PROJECT_SOURCE_DIR}/lib/glfw-3.3.8/include
		${PROJECT_SOURCE_DIR}/lib/soil/include
		${PROJECT_SOURCE_DIR}/lib/freetype/include
		${PROJECT_SOURCE_DIR}/lib/glslang
		${PROJECT_SOURCE_DIR}/lib/yaml-cpp/include
	)

	target_link_libraries( 
		vgl
		glfw
		Vulkan::Vulkan 
		#glew
		OpenGL::GL 
		soil 
		freetype 
		glslang 
		SPIRV 
		yaml-cpp  
	)
#endif(WIN32)
