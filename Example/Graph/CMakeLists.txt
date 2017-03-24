add_executable(Graph main.cpp Graph.hpp)
target_link_libraries(Graph Entity)
target_include_directories(Graph PRIVATE ${PROJECT_SOURCE_DIR})
