cmake_minimum_required(VERSION 3.0) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

### Require out-of-source builds
#file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
#if(EXISTS "${LOC_PATH}")
#    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
#endif()

#############################################################################################
#	VGLgfx Static Library
#############################################################################################

ADD_CUSTOM_TARGET(debug PRIVATE
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the debug mode.")

ADD_CUSTOM_TARGET(release PRIVATE
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the release mode.")


project(VGLgfx)

add_compile_definitions(VGL_RENDER_API_VULKAN GLEW_STATIC IMGUI_UNLIMITED_FRAME_RATE)

set(
	HEADER_FILES

	include/VGLgfx/VGLgfx.h

	include/VGLgfx/Platform/VkIncludes.h
	#include/VGLgfx/Platform/GLIncludes.h
	include/VGLgfx/VGL_Internal.h
	include/VGLgfx/VGL_Logger.h

	include/VGLgfx/GFXDefinitions.h
	include/VGLgfx/Platform/Definitions.h

	# Buffer layouts and handling
	include/VGLgfx/Platform/BufferLayout.h
	
	# Base glfw window
	include/VGLgfx/Platform/BaseWindow.h

	# Math library
	include/VGLgfx/Math/Math.h
	include/VGLgfx/Math/Matrix.h
	include/VGLgfx/Math/Vector.h
	include/VGLgfx/Math/Quaternion.h
	include/VGLgfx/Math/Camera.h
	include/VGLgfx/Math/Transform.h
	
	# General application handling, e.g. events, windows, renderAPI handling...
	include/VGLgfx/Application/RenderAPI.h
	include/VGLgfx/Application/InputManager.h
	include/VGLgfx/Application/Application.h
	include/VGLgfx/Application/Layer.h
	include/VGLgfx/Application/LayerStack.h
	
	include/VGLgfx/RenderPipelines.h
	include/VGLgfx/Scene.h
	
	# Event system
	include/VGLgfx/Events/Event.h
	include/VGLgfx/Events/ApplicationEvent.h
	include/VGLgfx/Events/KeyEvent.h
	include/VGLgfx/Events/MouseEvent.h
	
	include/VGLgfx/VGL-3D/Mesh/Material.h
	include/VGLgfx/VGL-3D/Mesh/Mesh.h
	include/VGLgfx/VGL-3D/Mesh/OBJ_Loader.h
	include/VGLgfx/VGL-3D/Mesh/VGL-Assimp.h
	include/VGLgfx/VGL-3D/Cuboid.h
	include/VGLgfx/VGL-3D/Grid.h
	include/VGLgfx/VGL-3D/Shapes.h
	include/VGLgfx/VGL-3D/SkyBox.h
	
	# Utilites
	include/VGLgfx/Utils/Logger.h
	include/VGLgfx/Utils/FileGUI.h
	
	#Entity component system
	include/VGLgfx/ECS/Component.h
	include/VGLgfx/ECS/System.h
	include/VGLgfx/ECS/ECS.h
	include/VGLgfx/DefaultECSComponents.h
	
	include/VGLgfx/Octree.h
	include/VGLgfx/Font.h
	
	#include/VGLgfx/Platform/OpenGL/GlWindow.h
	#include/VGLgfx/Platform/OpenGL/GlRenderer.h
	#include/VGLgfx/Platform/OpenGL/GlFramebufferAttachment.h
	#include/VGLgfx/Platform/OpenGL/GlDefinitions.h
	#include/VGLgfx/Platform/OpenGL/GlBuffers.h
	
	include/VGLgfx/Platform/Vulkan/VkDescriptorLayoutCache.h
	include/VGLgfx/Platform/Vulkan/VkContext.h
	include/VGLgfx/Platform/Vulkan/VkWindow.h
	
	include/VGLgfx/Platform/Vulkan/VkRenderer.h
	include/VGLgfx/Platform/Vulkan/VkRenderPass.h
	include/VGLgfx/Platform/Vulkan/VkDescriptor.h
	include/VGLgfx/Platform/Vulkan/VkFramebuffer.h
	include/VGLgfx/Platform/Vulkan/VkFramebufferAttachment.h
	include/VGLgfx/Platform/Vulkan/VkDefinitions.h
	include/VGLgfx/Platform/Vulkan/VkCommandBuffer.h 
	include/VGLgfx/Platform/Vulkan/VkImage.h 
	include/VGLgfx/Platform/Vulkan/VkBuffers.h
	include/VGLgfx/Platform/Vulkan/VkShader.h
	include/VGLgfx/Platform/Vulkan/VkGraphics_Internal.h
	include/VGLgfx/Platform/Vulkan/VkImGui.h
)

set(
	SOURCE_FILES
	
	include/VGLgfx/VGL_Logger.cpp

	# Buffer layouts and handling
	include/VGLgfx/Platform/BufferLayout.cpp

	# Base glfw window
	include/VGLgfx/Platform/BaseWindow.cpp

	# Math library
	include/VGLgfx/Math/Math.cpp
	include/VGLgfx/Math/Matrix.cpp
	include/VGLgfx/Math/Vector.cpp
	include/VGLgfx/Math/Quaternion.cpp
	include/VGLgfx/Math/Camera.cpp
	include/VGLgfx/Math/Transform.cpp
	
	# General application handling, e.g. events, windows, renderAPI
	include/VGLgfx/Application/InputManager.cpp
	include/VGLgfx/Application/Application.cpp
	include/VGLgfx/Application/Layer.cpp
	include/VGLgfx/Application/LayerStack.cpp

	include/VGLgfx/RenderPipelines.cpp
	include/VGLgfx/Scene.cpp

	# Event system
	include/VGLgfx/Events/Event.cpp
	include/VGLgfx/Events/ApplicationEvent.cpp
	include/VGLgfx/Events/KeyEvent.cpp
	include/VGLgfx/Events/MouseEvent.cpp

	# 3D
	include/VGLgfx/VGL-3D/Mesh/Material.cpp
	include/VGLgfx/VGL-3D/Mesh/Mesh.cpp
	include/VGLgfx/VGL-3D/Mesh/OBJ_Loader.cpp
	include/VGLgfx/VGL-3D/Cuboid.cpp
	include/VGLgfx/VGL-3D/Grid.cpp
	include/VGLgfx/VGL-3D/Shapes.cpp
	include/VGLgfx/VGL-3D/SkyBox.cpp

	# Utilites
	include/VGLgfx/Utils/Logger.h
	include/VGLgfx/Utils/Logger.cpp
	include/VGLgfx/Utils/FileGUI.h
	include/VGLgfx/Utils/FileGUI.cpp

	# Entity component system
	include/VGLgfx/ECS/Component.cpp
	include/VGLgfx/ECS/System.cpp
	include/VGLgfx/ECS/ECS.cpp

	include/VGLgfx/Octree.cpp
	include/VGLgfx/Font.cpp

	#include/VGLgfx/Platform/OpenGL/GlWindow.cpp
	#include/VGLgfx/Platform/OpenGL/GlRenderer.cpp
	#include/VGLgfx/Platform/OpenGL/GlFramebufferAttachment.cpp
	#include/VGLgfx/Platform/OpenGL/GlBuffers.cpp

	include/VGLgfx/Platform/Vulkan/VkContext.cpp
	include/VGLgfx/Platform/Vulkan/VkWindow.cpp
	include/VGLgfx/Platform/Vulkan/VkRenderer.cpp
	include/VGLgfx/Platform/Vulkan/VkRenderPass.cpp
	include/VGLgfx/Platform/Vulkan/VkDescriptor.cpp
	include/VGLgfx/Platform/Vulkan/VkFramebuffer.cpp
	include/VGLgfx/Platform/Vulkan/VkFramebufferAttachment.cpp
	include/VGLgfx/Platform/Vulkan/VkCommandBuffer.cpp 
	include/VGLgfx/Platform/Vulkan/VkImage.cpp 
	include/VGLgfx/Platform/Vulkan/VkBuffers.cpp
	include/VGLgfx/Platform/Vulkan/VkShader.cpp
	include/VGLgfx/Platform/Vulkan/VkGraphics_Internal.cpp
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

set_target_properties( VGLgfx
    PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

set(VGLGFX_DATA_DIR ${PROJECT_SOURCE_DIR}/data)

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

#install(
#	TARGETS VGLgfx
#	PUBLIC_HEADER DESTINATION ${PROJECT_SOURCE_DIR}/build/VGLgfx/include/
#	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/build/
#	ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/build/VGLgfx/
#	LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/build/VGLgfx/
#)


#############################################################################################
#	VGLgfx Test Application
#############################################################################################

add_executable(
	VGLgfxTest3D

	${PROJECT_SOURCE_DIR}/test/VGLgfxTest3D.cpp
	data/CMakeResources/icon.rc
)

set_target_properties( VGLgfxTest3D
    PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest3D/"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest3D/"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest3D/"
)

target_include_directories(VGLgfxTest3D PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(VGLgfxTest3D VGLgfx)

file(COPY ${VGLGFX_DATA_DIR} DESTINATION "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest3D/")

#############################################################################################
#	VGLgfx 2D Test Application
#############################################################################################

add_executable(
	VGLgfxTest2D

	${PROJECT_SOURCE_DIR}/test/VGLgfxTest2D.cpp
	data/CMakeResources/icon.rc
)

set_target_properties( VGLgfxTest2D
    PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest2D/"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest2D/"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest2D/"
)

target_include_directories(VGLgfxTest2D PRIVATE ${PROJECT_SOURCE_DIR}/include/)

target_link_libraries(VGLgfxTest2D VGLgfx)

file(COPY ${VGLGFX_DATA_DIR} DESTINATION "${PROJECT_SOURCE_DIR}/bin/VGLgfxTest2D/")

#############################################################################################
#	VGLgfx Voxel Cone Tracing Application
#############################################################################################

add_executable(
	VGLvxgi

	${PROJECT_SOURCE_DIR}/samples/vxgi.cpp
	data/CMakeResources/icon.rc
)

set_target_properties( VGLvxgi
    PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VXGI/"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VXGI/"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/VXGI/"
)

target_include_directories(VGLvxgi PRIVATE ${PROJECT_SOURCE_DIR}/include/)

target_link_libraries(VGLvxgi VGLgfx)

file(COPY ${VGLGFX_DATA_DIR} DESTINATION "${PROJECT_SOURCE_DIR}/bin/VXGI/")