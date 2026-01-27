cmake_minimum_required(VERSION 3.10)

project(slab_optimizer_server)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(/showIncludes)

include_directories(include)

add_executable(optimizer_server
    src/server.cpp
    src/parser.cpp
    src/optimizer.cpp
)

target_compile_definitions(optimizer_server PRIVATE CPPHTTPLIB_MULTIPART_FORM_DATA)

set_target_properties(optimizer_server PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:CONSOLE"
)
