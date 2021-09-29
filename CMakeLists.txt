cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME EducationProj)
project(${PROJECT_NAME})

add_subdirectory(Concurrency/RadixSort)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RadixSort)