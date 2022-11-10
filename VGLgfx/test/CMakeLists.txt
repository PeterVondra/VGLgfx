cmake_minimum_required(VERSION 3.0) 
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

ADD_CUSTOM_TARGET(debug
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the debug mode.")

ADD_CUSTOM_TARGET(release
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the release mode.")

project(VGLgfx)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(
	VGLgfxTest

	VGLgfxTest.cpp

	Editor/Editor.h
	Editor/Editor.cpp

	include/Platform/VkIncludes.h
	#include/Platform/GLIncludes.h
	include/VGL_Internal.h

	${headers}
	${sources}

	lib/VulkanMemoryAllocator/src/vk_mem_alloc.h

	include/GFXDefinitions.h
	include/Platform/Definitions.h

	include/Utils/ThreadPool.h
	include/Utils/Logger.h
	include/Utils/Logger.cpp
	include/Utils/Cache.h
	include/Utils/FileGUI.h
	include/Utils/FileGUI.cpp

	data/CMakeResources/icon.rc

	lib/robin_hood.h
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
 "include/VGL_Logging.h")

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