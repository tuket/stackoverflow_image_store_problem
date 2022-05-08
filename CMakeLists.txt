cmake_minimum_required(VERSION 3.8)
project(image_store)

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/glad)
add_subdirectory(libs/glfw)
add_subdirectory(libs/stb)

add_executable(image_store
	src/main.cpp
	src/utils.hpp src/utils.cpp
	src/shaders.hpp src/shaders.cpp
)
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT image_store)
target_link_libraries(image_store
	glad
	glfw
	stb
)
set_target_properties(
    image_store PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")