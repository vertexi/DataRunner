add_executable(DataRunner glfw_opengl3.cpp datarunner.cpp)

set_target_properties(DataRunner PROPERTIES COMPILE_FLAGS "/bigobj")

add_subdirectory(implot)
target_link_libraries(DataRunner PUBLIC implot)

add_subdirectory(exprtk)
target_link_libraries(DataRunner PUBLIC exprtk)

find_package(glad CONFIG REQUIRED)
target_link_libraries(DataRunner PRIVATE glad::glad)

# this is heuristically generated, and may not be correct
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(DataRunner PRIVATE glfw)

# this is heuristically generated, and may not be correct
find_package(imgui CONFIG REQUIRED)
target_link_libraries(DataRunner PRIVATE imgui::imgui)

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
