cmake_minimum_required(VERSION 2.8)
project(Entity)
set(CMAKE_CXX_STANDARD 14)

# Include Boost as an imported target
find_package(Boost REQUIRED)
add_library(boost INTERFACE IMPORTED)
set_property(TARGET boost PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(LINK_FLAGS "${LINK_FLAGS} -fprofile-arcs -ftest-coverage")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()


# External 3rd party libs that we include
INCLUDE_DIRECTORIES(3rdParty/catch/include)


add_subdirectory(Entity/Core)
add_subdirectory(Entity/Graph)
add_subdirectory(Example/Particle)
add_subdirectory(Example/Travel)
add_subdirectory(Example/Netlist)
