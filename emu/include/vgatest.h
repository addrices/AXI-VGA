#pragma once
#include "cmem.h"
#include "emu.h"

extern vluint64_t main_time;
class vga_test{
private:
  std::shared_ptr<CMem> AxiMem;
  std::shared_ptr<emu> vga_emu;

  enum WState{
    Widle = 0,
    Waw,
    Ww
  };
  enum RState{
    Ridle = 0,
    Rar
  };
  WState w_state;
  RState r_state;
  IData r_data_tmp;
  IData w_data_tmp;
  IData r_addr_tmp;
  IData w_addr_tmp;
  int r_burst_len;
  int r_burst_len_tmp;
  int w_burst_len;
  int w_burst_len_tmp;

public:
  vga_test(std::shared_ptr<emu> p);

  void reset_vga_emu();
  void next_cycle();
  bool CtrlAxiWrite(uint32_t data, uint32_t addr, uint32_t strb);
  uint32_t CtrlAxiRead(uint32_t addr);
  void MAxi_eval();

  void test1();

};