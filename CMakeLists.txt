cmake_minimum_required(VERSION 3.10)

project(freedi VERSION 1.0)

set(requiredlibs)
find_package(CURL)
if (CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIR})
	set(requiredlibs ${requiredlibs} ${CURL_LIBRARIES})
else (CURL_FOUND)
	message(FATAL_ERROR "libcurl not found")
endif(CURL_FOUND)

add_executable(freedi freedi.c cJSON.c cJSON.h)

target_link_libraries(freedi ${CURL_LIBRARIES})

target_include_directories(freedi PUBLIC "${PROJECT_BINARY_DIR}")
