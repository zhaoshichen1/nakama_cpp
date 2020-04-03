cmake_minimum_required(VERSION 3.15)
project(openfuck)

add_subdirectory(nakama-cpp-sdk)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_STANDARD 17)

set(NAKAMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nakama-cpp-sdk CACHE STRING "NAKAMA dir")

include_directories(${NAKAMA_DIR}/include)

if(APPLE)
    link_directories(${NAKAMA_DIR}/libs/mac)
    link_directories(${NAKAMA_DIR}/shared-libs/mac)
endif()

#link_libraries(nakama-cpp)

add_executable(openfuck main.cpp main.cpp main.h)

target_include_directories(openfuck PUBLIC ${NAKAMA_DIR}/include)
target_link_libraries(openfuck ext_nakama-cpp)