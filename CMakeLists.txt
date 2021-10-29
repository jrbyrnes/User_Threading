cmake_minimum_required(VERSION 3.4.3)

project(UserThreading)

set(CUR_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})

message("Building for " ${CUR_ARCH})


set(UTHREAD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UTHREAD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${UTHREAD_SOURCE_DIR}/include)

if (${CUR_ARCH} STREQUAL "x86_64")

    add_executable(ThreadingExample lib/threading.cpp)

    target_link_libraries(ThreadingExample -lpthread)

else()

    message(${CUR_ARCH} " not supported")

endif()