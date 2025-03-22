cmake_minimum_required(VERSION 3.10)
project(rest_api_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Boost components
find_package(Boost REQUIRED COMPONENTS system thread json)
find_package(Threads REQUIRED)

# Add the executable
add_executable(rest_api_server src/main.cpp)

# Link against Boost and system libraries
target_link_libraries(rest_api_server
    PRIVATE
    Boost::system
    Boost::thread
    Boost::json
    Threads::Threads
)

# Include directories
target_include_directories(rest_api_server
    PRIVATE
    ${Boost_INCLUDE_DIRS}
)