# Every library has unit tests, of course
add_executable(SystemTest SystemTest.cpp)
target_link_libraries(SystemTest entity catch)
add_test(SystemTest SystemTest)

add_executable(PropertyTest PropertyTest.cpp)
target_link_libraries(PropertyTest entity catch)
add_test(PropertyTest PropertyTest)

add_executable(TupleVectorTest TupleVectorTest.cpp)
target_link_libraries(TupleVectorTest entity catch)
add_test(TupleVectorTest TupleVectorTest)

add_executable(HierarchyTest HierarchyTest.cpp)
target_link_libraries(HierarchyTest entity catch)
add_test(HierarchyTest HierarchyTest)

add_executable(GraphTest GraphTest.cpp)
target_link_libraries(GraphTest entity catch)
add_test(GraphTest GraphTest)

add_executable(TupleVectorBenchmark TupleVectorBenchmark.cpp)
target_link_libraries(TupleVectorBenchmark entity catch)
