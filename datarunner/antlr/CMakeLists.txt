# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)


if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/archive/antlr4-cpp-runtime-4.13.1-source.zip)
	file(DOWNLOAD https://www.antlr.org/download/antlr4-cpp-runtime-4.13.1-source.zip
		${CMAKE_CURRENT_LIST_DIR}/archive/antlr4-cpp-runtime-4.13.1-source.zip
		EXPECTED_MD5 C875C148991AACD043F733827644A76F)
endif()

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime/cmake/ExternalAntlr4Cpp.cmake.patch)
	file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_LIST_DIR}/archive/antlr4-cpp-runtime-4.13.1-source.zip DESTINATION ${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime)
	configure_file(${CMAKE_CURRENT_LIST_DIR}/ExternalAntlr4Cpp.cmake.patch ${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime/cmake/ExternalAntlr4Cpp.cmake.patch)
	# find patch program for patch ExternalAntlr4Cpp.cmake later.
	if(WIN32)
		# prioritize Git Patch on Windows as other Patches may be very old and incompatible.
		find_package(Git)
		if(Git_FOUND)
			get_filename_component(GIT_DIR ${GIT_EXECUTABLE} DIRECTORY)
			get_filename_component(GIT_DIR ${GIT_DIR} DIRECTORY)
		endif()
	endif()

	find_program(PATCH
		NAMES patch
		HINTS ${GIT_DIR}
		PATH_SUFFIXES usr/bin
		)
	# patch ExternalAntlr4Cpp.cmake
	execute_process(COMMAND ${PATCH} ${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime/cmake/ExternalAntlr4Cpp.cmake ${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime/cmake/ExternalAntlr4Cpp.cmake.patch)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime/cmake)

# compiler must be 17
set(CMAKE_CXX_STANDARD 20)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# Specify the version of the antlr4 library needed for this project.
# By default the latest version of antlr4 will be used.  You can specify a
# specific, stable version by setting a repository tag value or a link
# to a zip file containing the libary source.
# set(ANTLR4_TAG 4.13.1)
# set(ANTLR4_ZIP_REPOSITORY https://github.com/antlr/antlr4/archive/refs/tags/4.13.1.zip)

# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antlr4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
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

set(ANTLR_EXECUTABLE ${ANTLR4_JAR_LOCATION})
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

# Call macro to add lexer and grammar to your build dependencies.
antlr_target(Cparser C.g4)

# include generated files in project environment
include_directories(${ANTLR_Cparser_OUTPUT_DIR})

# add generated grammar to demo binary target
add_library(antlr main.cpp
               ${ANTLR_Cparser_CXX_OUTPUTS})
target_link_libraries(antlr antlr4_static)
