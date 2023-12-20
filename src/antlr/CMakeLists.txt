# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

set(WITH_DEMO OFF)
set(ANTLR4_INSTALL ON)
set(ANTLR_BUILD_CPP_TESTS OFF)
set(ANTLR_BUILD_SHARED OFF)
set(RUNTIME_PATH "${CMAKE_CURRENT_LIST_DIR}/antlr4/runtime/Cpp")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/antlr4/runtime/Cpp)

# set variable pointing to the antlr tool that supports C++
set(Java_JAVA_EXECUTABLE  "C:/Program Files/Java/jdk-19/bin/java.exe")
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
	antlr4_jar
	URL https://www.antlr.org/download/antlr-4.13.1-complete.jar
	URL_MD5 78AF96AF276609AF0BFB3F1E2BFAEF89
	DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/archive
	DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(antlr4_jar)

file(REAL_PATH "${CMAKE_CURRENT_LIST_DIR}/archive/antlr-4.13.1-complete.jar" ANTLR4_JAR_LOCATION)
get_target_property(ANTLR4_INC antlr4_static INCLUDE_DIRECTORIES)

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_LIST_DIR}/output
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/C.g4
  COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR4_JAR_LOCATION} ${CMAKE_CURRENT_LIST_DIR}/C.g4
			-o ${CMAKE_CURRENT_LIST_DIR}/output
			-no-listener
			-Dlanguage=Cpp
			-visitor
)
add_custom_target(
  CparserFiles
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/output
)
file(GLOB CparserFilesList "${CMAKE_CURRENT_LIST_DIR}/output/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/output/*.h")
add_library(antlr parser.cpp parser.hpp ${CparserFilesList})
add_dependencies(antlr CparserFiles)
target_include_directories(antlr PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${ANTLR4_INC} ${CMAKE_CURRENT_LIST_DIR}/output)
target_link_libraries(antlr PUBLIC antlr4_static hello_imgui)
