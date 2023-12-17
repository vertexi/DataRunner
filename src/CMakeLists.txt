cmake_minimum_required(VERSION 3.24)
project(DataRunner)

# add_executable(DataRunner glfw_opengl3.cpp datarunner.cpp)
# add_executable(DataRunner imgui_bundle_test.cpp)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# suppress MSVC C4819: The file contains a character that cannot be represented in the current code page
if(MSVC)
    add_compile_options("/utf-8")
endif(MSVC)

add_subdirectory(imgui_bundle)
add_subdirectory(demo_utils)
imgui_bundle_add_app(DataRunner
    datarunner_gui.cpp
    # bgfx_test.cpp
    block_Nahimic.cpp
    block_Nahimic.hpp
    data.cpp
    data.hpp
    gui.cpp
    gui.hpp
    tcp.cpp
    tcp.hpp
    datarunner_core.cpp
    datarunner_core.hpp)
target_link_libraries(DataRunner PUBLIC demo_utils)

target_compile_features(DataRunner PRIVATE cxx_std_20)

# if(EMSCRIPTEN)
#     target_link_options(DataRunner PUBLIC -sERROR_ON_UNDEFINED_SYMBOLS=0)
# endif()
if(MSVC)
    set_target_properties(DataRunner PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
    target_compile_definitions(DataRunner PRIVATE "-D_WIN32_WINNT=0x0601")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++20")
    option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
    # set MSVC warning level to 4
    string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
    find_library(DETOURS_LIBRARY detours REQUIRED)

    target_include_directories(DataRunner PRIVATE ${DETOURS_INCLUDE_DIRS})
    target_link_libraries(DataRunner PRIVATE ${DETOURS_LIBRARY})
endif(MSVC)

# set_target_properties(DataRunner PROPERTIES COMPILE_FLAGS "/bigobj")
set_target_properties(DataRunner PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(DataRunner PROPERTIES CXX_EXTENSIONS OFF)

add_subdirectory(exprtk)
target_link_libraries(DataRunner PUBLIC exprtk)

add_subdirectory(antlr)
target_link_libraries(DataRunner PUBLIC antlr)

if(NOT EMSCRIPTEN)
    find_package(asio CONFIG REQUIRED)
    target_link_libraries(DataRunner PRIVATE asio asio::asio)

    find_package(libusb CONFIG REQUIRED)
    target_include_directories(DataRunner PRIVATE ${LIBUSB_INCLUDE_DIRS})
    target_link_libraries(DataRunner PRIVATE ${LIBUSB_LIBRARIES})
endif()

# copy assets
add_custom_command(TARGET DataRunner PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:DataRunner>)
add_custom_command(TARGET DataRunner PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/demos_assets $<TARGET_FILE_DIR:DataRunner>/demos_assets)
add_custom_command(TARGET DataRunner PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:DataRunner>/assets)