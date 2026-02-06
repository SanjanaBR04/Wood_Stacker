cmake_minimum_required(VERSION 3.10)

project(slab_optimizer_server)

# Ensure C++17 is used and required for MaxRects/Filesystem features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Explicitly find Threads for httplib
find_package(Threads REQUIRED)

# 2. Include directories
include_directories(include)

# 3. Define the executable and its source files
add_executable(optimizer_server
    src/server.cpp
    src/parser.cpp
    src/optimizer.cpp
)

# 4. Link necessary libraries (Threads is the big one missing in yours)
target_link_libraries(optimizer_server PRIVATE 
    Threads::Threads
)

# 5. Define the multipart flag for httplib
target_compile_definitions(optimizer_server PRIVATE CPPHTTPLIB_MULTIPART_FORM_DATA)

# 6. Apply Windows-only flags safely
if(MSVC)
    add_compile_options(/showIncludes)
    set_target_properties(optimizer_server PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()