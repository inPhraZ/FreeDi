cmake_minimum_required(VERSION 3.10)

project(goodi VERSION 1.0)

set(requiredlibs)
find_package(CURL)
if (CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIR})
	set(requiredlibs ${requiredlibs} ${CURL_LIBRARIES})
else (CURL_FOUND)
	message(FATAL_ERROR "libcurl not found")
endif(CURL_FOUND)

add_executable(goodi goodi.c cJSON.c cJSON.h)

target_include_directories(goodi PUBLIC "${PROJECT_BINARY_DIR}")
