add_executable(serialMonitor serialMonitor.cpp)

find_package(glad CONFIG REQUIRED)
target_link_libraries(serialMonitor PRIVATE glad::glad)

# this is heuristically generated, and may not be correct
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(serialMonitor PRIVATE glfw)

# this is heuristically generated, and may not be correct
find_package(imgui CONFIG REQUIRED)
target_link_libraries(serialMonitor PRIVATE imgui::imgui)

find_package(implot CONFIG REQUIRED)
target_link_libraries(serialMonitor PRIVATE implot::implot)

find_package(asio CONFIG REQUIRED)
target_link_libraries(serialMonitor PRIVATE asio asio::asio)
