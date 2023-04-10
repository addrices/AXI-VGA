#include "emu.h"
#include <memory>
#include <iostream>
#include <verilated.h>
#include "vgatest.h"
vluint64_t main_time = 0; // Current simulation time

double sc_time_stamp()
{                   // Called by $time in Verilog
  return main_time; // converts to double, to match
                    // what SystemC does
}

int main(int argc, char **argv)
{
  Verilated::commandArgs(argc, argv);
#ifdef __TRACE__
  Verilated::traceEverOn(true);
  Verilated::mkdir("logs");
#endif
  auto dut = std::make_shared<emu>();

  auto vtest = std::make_shared<vga_test>(dut);
  vtest->test1();
  return 0;
}