cmake_minimum_required(VERSION 3.18)
project(creator_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Target Windows 10/11 for Asio
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

include(FetchContent)

# 1. Fetch Asio
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-30-2
)
FetchContent_MakeAvailable(asio)

set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "Asio include directory" FORCE)
include_directories(${ASIO_INCLUDE_DIR})

# 2. Fetch Crow
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG        v1.2.0
)
FetchContent_MakeAvailable(Crow)

# 3. Fetch nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)

# Executable Setup
add_executable(creator_backend main.cpp)

find_package(Threads REQUIRED)

target_link_libraries(creator_backend PRIVATE 
    Crow::Crow 
    nlohmann_json::nlohmann_json
    Threads::Threads
)