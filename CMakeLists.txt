cmake_minimum_required(VERSION 3.1)
project(Entity)
set(CMAKE_CXX_STANDARD 17)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})

find_package(Boost REQUIRED)

add_library(range INTERFACE)
target_include_directories(range INTERFACE 3rdparty/range-v3/include)

add_library(entity INTERFACE)
target_include_directories(entity INTERFACE include ${Boost_INCLUDE_DIRS})
target_link_libraries(entity INTERFACE range)

set(Entity_SOURCES_LIST "")
add_subdirectory(include/Entity)
#
#  HACK: have the files showing in the IDE, under the name 'moduleA_ide'
#
add_custom_target(entity_IDE SOURCES ${Entity_SOURCES_LIST})

add_subdirectory(src)
