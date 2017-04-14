add_library(Graph STATIC Graph.hpp)
set_target_properties(Graph PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(Graph Entity)
target_include_directories(Graph PUBLIC ${PROJECT_SOURCE_DIR}/3rdParty/range-v3/include)

# Every library has unit tests, of course
add_executable(GraphTest GraphTest.cpp)
target_link_libraries(GraphTest Graph)
add_test(GraphTest GraphTest)

file(GLOB HEADER_FILES "*.hpp")
install(FILES ${HEADER_FILES} DESTINATION include/Graph)
