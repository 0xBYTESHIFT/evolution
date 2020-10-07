cmake_minimum_required(VERSION 3.11.0)
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

include(FetchContent)
FetchContent_Declare(neur
  GIT_REPOSITORY https://github.com/0xBYTESHIFT/neur
  #GIT_TAG develop   # it's much better to use a specific Git revision or Git tag for reproducibility
)
FetchContent_MakeAvailable(neur)
if(NOT neur_POPULATED)
  FetchContent_Populate(neur)
  add_subdirectory(${neur_SOURCE_DIR} ${neur_BINARY_DIR})
endif()

FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  #GIT_TAG develop   # it's much better to use a specific Git revision or Git tag for reproducibility
)
FetchContent_MakeAvailable(imgui)

include_directories(
    include/
    ui/
    ${imgui_SOURCE_DIR}/
    ${imgui_SOURCE_DIR}/examples
    ${neur_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    )

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s -DNDEBUG -march=native")
add_executable(evolution main.cpp
    ui/ui.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/examples/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/examples/imgui_impl_sdl.cpp
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
