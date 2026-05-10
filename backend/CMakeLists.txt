cmake_minimum_required(VERSION 3.16)

project(anonymous_realtime_chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
  asio
  URL https://downloads.sourceforge.net/project/asio/asio/1.28.2%20%28Stable%29/asio-1.28.2.tar.gz
)

FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
  FetchContent_Populate(asio)
endif()

set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/include" CACHE PATH "Pinned standalone Asio include directory" FORCE)
if(NOT EXISTS "${ASIO_INCLUDE_DIR}/asio.hpp")
  message(FATAL_ERROR "Pinned Asio header not found at ${ASIO_INCLUDE_DIR}/asio.hpp")
endif()

FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.2.1
)

FetchContent_MakeAvailable(Crow)

find_package(Threads REQUIRED)

add_executable(anon_chat
  src/main.cpp
  src/room.cpp
  src/user.cpp
)

target_link_libraries(anon_chat PRIVATE Crow::Crow Threads::Threads)
