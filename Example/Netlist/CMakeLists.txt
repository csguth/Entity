add_executable(NetlistTest NetlistTest.cpp Netlist.hpp)
target_link_libraries(NetlistTest Entity)
add_test(NetlistTest NetlistTest)
