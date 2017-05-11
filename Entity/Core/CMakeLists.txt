add_library(Entity STATIC System.hpp Property.hpp TupleVector.hpp SystemWithDeletion.hpp Composition.hpp KeyWrapper.hpp)
set_target_properties(Entity PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(Entity boost)
target_include_directories(Entity PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/3rdParty/range-v3/include)

# Every library has unit tests, of course
add_executable(SystemTest SystemTest.cpp)
target_link_libraries(SystemTest Entity)
add_test(SystemTest SystemTest)

add_executable(PropertyTest PropertyTest.cpp)
target_link_libraries(PropertyTest Entity)
add_test(PropertyTest PropertyTest)

add_executable(TupleVectorTest TupleVectorTest.cpp)
target_link_libraries(TupleVectorTest Entity)
add_test(TupleVectorTest TupleVectorTest)

add_executable(HierarchyTest HierarchyTest.cpp)
target_link_libraries(HierarchyTest Entity)
add_test(HierarchyTest HierarchyTest)

add_executable(TupleVectorBenchmark TupleVectorBenchmark.cpp)
target_link_libraries(TupleVectorBenchmark Entity)

file(GLOB HEADER_FILES "*.hpp")
install(FILES ${HEADER_FILES} DESTINATION include/Entity)
