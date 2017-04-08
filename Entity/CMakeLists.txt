add_library(Entity INTERFACE)
# System.hpp Property.hpp TupleVector.hpp SystemWithDeletion.hpp
target_compile_definitions(Entity 
    INTERFACE LIBRARY_HEADER_ONLY
)
set_property(TARGET Entity PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/3rdParty/range-v3/include)

# Every library has unit tests, of course
add_executable(SystemTest SystemTest.cpp)
target_link_libraries(SystemTest Entity boost)
add_test(SystemTest SystemTest)

add_executable(PropertyTest PropertyTest.cpp)
target_link_libraries(PropertyTest Entity boost)
add_test(PropertyTest PropertyTest)

add_executable(TupleVectorTest TupleVectorTest.cpp)
target_link_libraries(TupleVectorTest Entity boost)
add_test(TupleVectorTest TupleVectorTest)

add_executable(TupleVectorBenchmark TupleVectorBenchmark.cpp)
target_link_libraries(TupleVectorBenchmark Entity boost)

file(GLOB HEADER_FILES "*.hpp")
install(FILES ${HEADER_FILES} DESTINATION include/Entity)
