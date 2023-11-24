add_executable(DataRunner glfw_opengl3.cpp datarunner.cpp)

set_target_properties(DataRunner PROPERTIES COMPILE_FLAGS "/bigobj")

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(imgui ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PRIVATE glfw)

target_link_libraries(DataRunner PUBLIC imgui)

add_subdirectory(implot)
target_link_libraries(DataRunner PUBLIC implot)

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
