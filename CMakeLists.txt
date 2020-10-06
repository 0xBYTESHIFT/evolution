cmake_minimum_required(VERSION 3.0.0)
project(evolution VERSION 0.1.6)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system)

set(BLAS_VENDOR OpenBLAS)
find_package(OpenBLAS QUIET)
set(BLAS_LIB OpenBLAS_LIB)
set(BLAS_DIR OpenBLAS_DIR)
set(BLAS_INCLUDE_DIR OpenBLAS_INCLUDE_DIR)
if(NOT OpenBLAS_FOUND)
    message(FATAL_ERROR "no openblas")
endif()

include_directories(
    submodules/neur/include
    submodules/
    submodules/imgui/
    submodules/imgui/examples
    ${Boost_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    )

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s -DNDEBUG -march=native")
add_executable(evolution main.cpp
    ui.cpp
    submodules/imgui/imgui.cpp
    submodules/imgui/imgui_draw.cpp
    submodules/imgui/imgui_widgets.cpp
    submodules/imgui/imgui_demo.cpp
    submodules/imgui/examples/imgui_impl_opengl3.cpp
    submodules/imgui/examples/imgui_impl_sdl.cpp
)
target_link_libraries(evolution
    OpenBLAS::OpenBLAS
    pthread
    tbb
    ${Boost_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
