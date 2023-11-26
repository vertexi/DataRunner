# add_executable(DataRunner glfw_opengl3.cpp datarunner.cpp)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(DataRunner bgfx_test.cpp)
target_compile_features(DataRunner PRIVATE cxx_std_17)
if(WIN32)
    set_target_properties(DataRunner PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
endif(WIN32)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++17")
    option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
endif(MSVC)

set_target_properties(DataRunner PROPERTIES COMPILE_FLAGS "/bigobj")
set_target_properties(DataRunner PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(DataRunner PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(DataRunner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bgfx/include ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bx/include ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bimg/include ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bx/include/compat/msvc)
target_include_directories(DataRunner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bgfx/3rdparty ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bgfx/examples/common)
target_link_libraries(DataRunner ${CMAKE_DL_LIBS})
target_link_directories(DataRunner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bgfx/.build/win64_vs2022/bin/)
target_link_libraries(DataRunner PUBLIC
    bgfxDebug
    bimgDebug
    bxDebug
    example-glueDebug
    example-commonDebug
    bimg_decodeDebug
)
set_target_properties(DataRunner PROPERTIES COMPILE_DEFINITIONS "BX_CONFIG_DEBUG=1")

file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/bgfx/bgfx/examples/runtime/ DESTINATION ${CMAKE_BINARY_DIR}/datarunner/Debug)

# Imgui
# set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
# add_library(imgui ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
# target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
# target_link_libraries(imgui PRIVATE glfw)

# target_link_libraries(DataRunner PUBLIC imgui)

# add_subdirectory(implot)
# target_link_libraries(DataRunner PUBLIC implot)

add_subdirectory(exprtk)
target_link_libraries(DataRunner PUBLIC exprtk)

find_package(glad CONFIG REQUIRED)
target_link_libraries(DataRunner PRIVATE glad::glad)

# this is heuristically generated, and may not be correct
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(DataRunner PRIVATE glfw)

# find_package(implot CONFIG REQUIRED)
# target_link_libraries(DataRunner PRIVATE implot::implot)

find_package(asio CONFIG REQUIRED)
target_link_libraries(DataRunner PRIVATE asio asio::asio)

find_package(libusb CONFIG REQUIRED)
target_include_directories(DataRunner PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(DataRunner PRIVATE ${LIBUSB_LIBRARIES})

add_custom_command(TARGET DataRunner PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/datarunner/res/ $<TARGET_FILE_DIR:DataRunner>)
