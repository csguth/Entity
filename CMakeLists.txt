cmake_minimum_required(VERSION 2.8)
project(Entity)
set(CMAKE_CXX_STANDARD 14)

add_definitions(-DBOOST_USE_WINDOWS_H)
add_definitions(-DNOMINMAX)
add_definitions(-DBOOST_SP_USE_STD_ATOMIC)

# Include Boost as an imported target
find_package(Boost REQUIRED)
add_library(boost INTERFACE IMPORTED)
set_property(TARGET boost PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

# External 3rd party libs that we include
INCLUDE_DIRECTORIES(3rdParty/catch/include)

add_subdirectory(Entity)
add_subdirectory(Example/Graph)
add_subdirectory(Example/Particle)
