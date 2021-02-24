# Every library has unit tests, of course
add_executable(
    tests
    SystemTest.cpp
    PropertyTest.cpp
    #TupleVectorTest.cpp
    HierarchyTest.cpp
    GraphTest.cpp
    main.cpp
)
target_link_libraries(tests entity)
#add_executable(benchmark TupleVectorBenchmark.cpp)
#target_link_libraries(benchmark entity)
