add_executable(NetlistTest NetlistTest.cpp Netlist.hpp Verilog.hpp Views.hpp)
target_link_libraries(NetlistTest entity catch)
add_test(NetlistTest NetlistTest)
