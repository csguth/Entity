add_library(Entity STATIC System.hpp Property.hpp)
set_target_properties(Entity PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(Entity boost)
target_include_directories(Entity PUBLIC ${PROJECT_SOURCE_DIR}/3rdParty/range-v3/include)

# Every library has unit tests, of course
add_executable(SystemTest SystemTest.cpp)
target_link_libraries(SystemTest Entity)
add_test(SystemTest SystemTest)

add_executable(PropertyTest PropertyTest.cpp)
target_link_libraries(PropertyTest Entity)
add_test(PropertyTest PropertyTest)

file(GLOB HEADER_FILES "*.hpp")
install(FILES ${HEADER_FILES} DESTINATION include/Entity)
