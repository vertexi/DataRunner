cmake_minimum_required(VERSION 3.12)
project(DataRunner)

# add_executable(DataRunner glfw_opengl3.cpp datarunner.cpp)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add_executable(DataRunner imgui_bundle_test.cpp)
add_subdirectory(imgui_bundle)
add_subdirectory(demo_utils)
imgui_bundle_add_app(DataRunner imgui_bundle_test.cpp)
target_link_libraries(DataRunner PUBLIC demo_utils)

target_compile_features(DataRunner PRIVATE cxx_std_20)

if(MSVC)
    set_target_properties(DataRunner PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++20")
    option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
endif(MSVC)

# set_target_properties(DataRunner PROPERTIES COMPILE_FLAGS "/bigobj")
set_target_properties(DataRunner PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(DataRunner PROPERTIES CXX_EXTENSIONS OFF)

add_subdirectory(exprtk)
target_link_libraries(DataRunner PUBLIC exprtk)

if(NOT EMSCRIPTEN)
    find_package(asio CONFIG REQUIRED)
    target_link_libraries(DataRunner PRIVATE asio asio::asio)

    find_package(libusb CONFIG REQUIRED)
    target_include_directories(DataRunner PRIVATE ${LIBUSB_INCLUDE_DIRS})
    target_link_libraries(DataRunner PRIVATE ${LIBUSB_LIBRARIES})
endif()

add_custom_command(TARGET DataRunner PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/datarunner/res/ $<TARGET_FILE_DIR:DataRunner>)
