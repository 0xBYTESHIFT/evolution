cmake_minimum_required(VERSION 3.11.0)
project(evolution VERSION 0.1.7)

include(CTest)
include(CheckCXXCompilerFlag)
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
macro(find_fetch package repo)
  find_package(${package} QUIET)
  if(NOT ${${package}_FOUND})
      message(STATUS "fetching ${package} from ${repo}")
      FetchContent_Declare(${package}
          GIT_REPOSITORY ${repo}
      )
      FetchContent_MakeAvailable(${package})
      FetchContent_GetProperties(${package})
      if(NOT ${${package}_POPULATED})
        FetchContent_Populate(${package})
        add_subdirectory(${${package}_SOURCE_DIR} ${${package}_BINARY_DIR})
      endif()
      list(APPEND CMAKE_PREFIX_PATH ${${package}_BINARY_DIR})
  endif()
endmacro()

find_fetch(neur https://github.com/0xBYTESHIFT/neur)

find_fetch(imgui https://github.com/ocornut/imgui)
set(imgui_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/imgui-src") #ISSUE:dirty hack

find_fetch(xtl https://github.com/xtensor-stack/xtl.git)
find_fetch(xtensor https://github.com/xtensor-stack/xtensor.git)
find_fetch(xsimd https://github.com/xtensor-stack/xsimd.git)
find_fetch(xtensor-blas https://github.com/xtensor-stack/xtensor-blas)

include_directories(
    include/
    ui/
    ${imgui_SOURCE_DIR}/
    ${imgui_SOURCE_DIR}/examples
    ${neur_SOURCE_DIR}/include
    ${xtl_SOURCE_DIR}/
    ${xtensor_SOURCE_DIR}/
    ${xsimd_SOURCE_DIR}/
    ${xtensor-blas_SOURCE_DIR}/
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
    xtl
    xsimd
    xtensor
    xtensor-blas
    ${Boost_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
