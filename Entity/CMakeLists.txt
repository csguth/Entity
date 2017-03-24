add_library(Entity System.cpp System.hpp Property.hpp Property.cpp)
# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(Entity PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


# Depend on a library that we defined in the top-level file
target_link_libraries(Entity boost)

# 'make install' to the correct location
install(TARGETS Entity EXPORT EntityConfig
    ARCHIVE  DESTINATION lib
    LIBRARY  DESTINATION lib
    RUNTIME  DESTINATION bin)  # This is for Windows
install(DIRECTORY include/ DESTINATION include)


# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into <prefix>/cmake.
install(EXPORT EntityConfig DESTINATION share/Entity/cmake)

# This makes the project importable from the build directory
export(TARGETS Entity FILE EntityConfig.cmake)

# Every library has unit tests, of course
add_executable(SystemTest SystemTest.cpp)
target_link_libraries(SystemTest Entity)
add_test(SystemTest SystemTest)

add_executable(PropertyTest PropertyTest.cpp)
target_link_libraries(PropertyTest Entity)
add_test(PropertyTest PropertyTest)
