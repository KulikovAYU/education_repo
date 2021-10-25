cmake_minimum_required(VERSION 3.12)

set(CUSTOM_ALLOC_PROJECT_NAME CustomAllocator)

#headers
set (INC_FILES 
includes/MMap.hpp
includes/Allocator.hpp

)

#src files
set (SRC_FILES 

src/main.cpp)

#set startup sln
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY  VS_STARTUP_PROJECT ${CUSTOM_ALLOC_PROJECT_NAME})

add_executable(${CUSTOM_ALLOC_PROJECT_NAME} 
               ${INC_FILES}
               ${SRC_FILES})


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)


set(CMAKE_SUPPRESS_REGENERATION true)

target_compile_features(${CUSTOM_ALLOC_PROJECT_NAME} PUBLIC cxx_std_20)


