#include "cmem.h"
#include "vgatest.h" 
using namespace std;

uint8_t hw_empty_out8;
uint8_t hw_empty_in8;

vga_test::vga_test(std::shared_ptr<emu> p){
  AxiMem = std::make_shared<CMem>(0x100000,0x90000000);
  AxiMem->set_v();
  vga_emu = p;
  r_data_tmp = 0;
  r_addr_tmp = 0;
  r_state = Ridle;
  r_burst_len_tmp = 0;

  w_state = Widle;
  w_data_tmp = 0;
  w_addr_tmp = 0;
  w_burst_len_tmp = 0;
}

void vga_test::reset_vga_emu(){
  vga_emu->clock = 0;
  vga_emu->resetn = 0;
  main_time+=2;
  vga_emu->eval();
  vga_emu->clock = 1;
  main_time+=2;
  vga_emu->eval();
  vga_emu->clock = 0;
  vga_emu->resetn = 1;
  main_time+=2;
  vga_emu->eval();
  vga_emu->clock = 1;
  main_time+=2;
  vga_emu->eval();
}

void vga_test::next_cycle(){
  vga_emu->clock = 0;
  main_time+=2;
  vga_emu->eval();
  vga_emu->clock = 1;
  main_time+=2;
  vga_emu->eval();
}

bool vga_test::CtrlAxiWrite(uint32_t data, uint32_t addr, uint32_t strb){
  vga_emu->io_slave_awvalid = 1;
  vga_emu->io_slave_awaddr = addr;
  while(vga_emu->io_slave_awready != 1)
    next_cycle();
  next_cycle();
  vga_emu->io_slave_awvalid = 0;

  vga_emu->io_slave_wvalid = 1;
  vga_emu->io_slave_wdata = data;
  vga_emu->io_slave_wstrb = strb;
  while(vga_emu->io_slave_wready != 1)
    next_cycle();
  next_cycle();
  vga_emu->io_slave_wvalid = 0;

  vga_emu->io_slave_bready = 1;
  while(vga_emu->io_slave_bvalid != 1)
    next_cycle();
  uint32_t b_resp = vga_emu->io_slave_bresp;
  next_cycle();
  vga_emu->io_slave_bready = 0;
  
  if(b_resp == 0 || b_resp == 1)
    return true;
  else
    return false;
}

uint32_t vga_test::CtrlAxiRead(uint32_t addr){
  vga_emu->io_slave_arvalid = 1;
  vga_emu->io_slave_araddr = addr;
  while(vga_emu->io_slave_arready != 1)
    next_cycle();
  next_cycle();
  vga_emu->io_slave_arvalid = 0;

  vga_emu->io_slave_rready = 1;
  while(vga_emu->io_slave_rvalid != 1)
    next_cycle();
  uint32_t data = vga_emu->io_slave_rdata;
  uint32_t resp = vga_emu->io_slave_rresp;
  next_cycle();
  if(resp == 0 || resp == 1)
    return data;
  else
    return 0;
}

void vga_test::MAxi_eval(){
  // W事件更新
  switch (w_state)
  {
  case Widle:
    vga_emu->io_master_awready = 1;
    vga_emu->io_master_wready = 0;
    vga_emu->io_master_bvalid = 0;
    break;
  case Waw:
    vga_emu->io_master_awready = 0;
    vga_emu->io_master_wready = 1;
    vga_emu->io_master_bvalid = 0;
    break;
  case Ww:
    vga_emu->io_master_awready = 0;
    vga_emu->io_master_wready = 0;
    vga_emu->io_master_bvalid = 1;
    break;
  default:
    break;
  }

  if (w_state == Widle && vga_emu->io_master_awvalid == 1 && vga_emu->io_master_awready)
  {
    w_addr_tmp = vga_emu->io_master_awaddr;
    // tracef("WIdle w_addr_tmp:%x\n", w_addr_tmp);
    w_burst_len = vga_emu->io_master_awlen;
    tracef("!!!WR begin addr:%x len:%x\n", w_addr_tmp, w_burst_len);
    w_burst_len_tmp = 0;
    w_state = Waw;
  }
  else if (w_state == Waw && vga_emu->io_master_wvalid == 1 && vga_emu->io_master_wready == 1)
  {
    w_data_tmp = vga_emu->io_master_wdata;
    AxiMem->Write32(w_addr_tmp, w_data_tmp);
    // if (vga_emu->W_LAST == 1)
    // {
    // tracef("time:%ld\n", main_time);
    // tracef("write addr:%x data:%x\n", w_addr_tmp, w_data_tmp[0]);
    // }
    w_addr_tmp = w_addr_tmp + 4;
    if (vga_emu->io_master_wlast)
    {
      w_state = Ww;
    }
  }
  else if (w_state == Ww && vga_emu->io_master_bvalid && vga_emu->io_master_bready)
  {
    // tracef("Ww w_addr_tmp:%x\n", w_addr_tmp);
    w_state = Widle;
  }

  // R事件更新
  vga_emu->io_master_rdata = r_data_tmp;
  switch (r_state)
  {
  case Ridle:
    vga_emu->io_master_arready = 1;
    vga_emu->io_master_rvalid = 0;
    break;
  case Rar:
    vga_emu->io_master_arready = 0;
    vga_emu->io_master_rvalid = 1;
    break;
  default:
    break;
  }
  vga_emu->io_master_rlast = 0;
  if (r_state == Ridle && vga_emu->io_master_arready == 1 && vga_emu->io_master_arvalid == 1)
  {
    // tracef("!!!AR begin addr:%x len:%x\n",vga_emu->io_master_araddr, vga_emu->io_master_arlen);
    r_addr_tmp = vga_emu->io_master_araddr;
    r_data_tmp = AxiMem->Read32(r_addr_tmp);
    // printf("f data %x addr %x\n", r_data_tmp, r_addr_tmp);
    r_burst_len = vga_emu->io_master_arlen;
    r_burst_len_tmp = 0;
    r_state = Rar;
  }
  else if (r_state == Rar && vga_emu->io_master_rready == 1 && vga_emu->io_master_rvalid == 1)
  {
    if (r_burst_len == r_burst_len_tmp)
    {
      r_state = Ridle;
      vga_emu->io_master_rlast = 1;
    }
    else
    {
      vga_emu->io_master_rlast = 0;
    }
    r_addr_tmp += 4;
    r_data_tmp = AxiMem->Read32(r_addr_tmp);
    // printf("data:%x addr:%x\n", r_data_tmp, r_addr_tmp);
    // tracef("r_burst_len_tmp:%d r_burst_len:%d\n", r_burst_len_tmp, r_burst_len);
    r_burst_len_tmp++;
  }
  // vga_emu->tb->eval();
}

void vga_test::test1(){
  reset_vga_emu();
  CtrlAxiWrite(0x90000000,0x80000004,15);
  CtrlAxiWrite(1,0x80000000,1);
  for(int i = 0;i < 1000000;i++){
    MAxi_eval();
    next_cycle();
  }
}