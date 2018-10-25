cmake_minimum_required(VERSION 3.5)
project(gf3d)

# Requirements
include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

aux_source_directory(src/ SOURCE_FILES)

include_directories(
	include
	${CMAKE_SOURCE_DIR}/gfc/include
	${CMAKE_SOURCE_DIR}/gfc/simple_json/include
	${CMAKE_SOURCE_DIR}/gfc/simple_logger/include
	${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} SDL2_image png jpeg z vulkan m

	${CMAKE_SOURCE_DIR}/gfc/libs/libgfc.a
	${CMAKE_SOURCE_DIR}/gfc/simple_json/libs/libsj.a
	${CMAKE_SOURCE_DIR}/gfc/simple_logger/libs/libsl.a

	${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})
