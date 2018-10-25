cmake_minimum_required(VERSION 3.5)
project(gf3d)

# Requirements
include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)


# ALL SOURCES
set(SOURCE_FILES
        src/game.c
        src/gf3d_camera.c
        src/gf3d_commands.c
        src/gf3d_extensions.c
        src/gf3d_matrix.c
        src/gf3d_mesh.c
        src/gf3d_model.c
        src/gf3d_obj_load.c
        src/gf3d_pipeline.c
        src/gf3d_shaders.c
        src/gf3d_swapchain.c
        src/gf3d_texture.c
        src/gf3d_types.c
        src/gf3d_validation.c
        src/gf3d_vector.c
        src/gf3d_vgraphics.c
        src/gf3d_vqueues.c
        src/simple_logger.c
        )

include_directories(include libs/include ${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# GL, GLFW, DL, M
target_link_libraries(${PROJECT_NAME}
        dl m glfw GL vulkan
        pthread ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${CMAKE_SOURCE_DIR}/libs/libsj.a)

