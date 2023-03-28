cmake_minimum_required(VERSION 3.0) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

ADD_CUSTOM_TARGET(debug1 PRIVATE
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the debug mode.")

ADD_CUSTOM_TARGET(release1 PRIVATE
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
	COMMENT "Creating the executable in the release mode.")

project(VGLgfxEditor)

find_package(Vulkan REQUIRED)

add_executable(
	VGLgfxEditor

	${PROJECT_SOURCE_DIR}/main.cpp
	${PROJECT_SOURCE_DIR}/VGLgfxEditor.cpp
	${PROJECT_SOURCE_DIR}/VGLgfxEditor.h
	#data/CMakeResources/icon.rc
)

set_target_properties( VGLgfxEditor
    PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
)

add_subdirectory(${PROJECT_SOURCE_DIR}/../VGLgfx/ ${PROJECT_SOURCE_DIR}/../VGLgfx/build)

target_include_directories(VGLgfxEditor PRIVATE ${PROJECT_SOURCE_DIR}/../VGLgfx/include)
target_link_libraries(VGLgfxEditor VGLgfx)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../VGLgfx/data DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/)