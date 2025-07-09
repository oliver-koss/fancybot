cmake_minimum_required(VERSION 3.5)
project(project_name C)
set(CMAKE_C_CSTANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lm -lcjson")
include_directories(HEADER_FILES)
file(GLOB MAIN
    "json2gpx.c"
    )
    add_executable(json2gpx ${MAIN})