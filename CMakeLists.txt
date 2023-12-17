cmake_minimum_required(VERSION 3.10)

project(DataRunner VERSION 0.0)

add_subdirectory(src)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES ${PROJECT_BINARY_DIR}/src/Release/fa-solid-900.otf
        ${PROJECT_BINARY_DIR}/src/Release/glfw3.dll
        ${PROJECT_BINARY_DIR}/src/Release/NotoSansSC-Medium.otf
        ${PROJECT_BINARY_DIR}/src/Release/Roboto-Medium.ttf
        ${PROJECT_BINARY_DIR}/src/Release/imgui.ini
        DESTINATION bin)

include(InstallRequiredSystemLibraries)
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)
