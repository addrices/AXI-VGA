#pragma once
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "util.h"
using namespace std;

class CMem
{
private:
  uint32_t *Data; // array to store mem's data
  uint32_t Size;  // this mem has Size bytes
  string filename;
  uint32_t BeginAddr;
  uint32_t get_value(string tmp);

public:
  /*  addr: Mem's begin addr;
   *  size: Mem's begin addr
   *  
   */

  CMem(uint32_t size, uint32_t addr, string filename);

  CMem(uint32_t size, uint32_t addr);

  uint32_t Read32(uint32_t addr);

  uint64_t Read64(uint32_t addr);

  bool Write32(uint32_t addr, uint32_t data);

  void write2file(string writefilename);

  void range_assign(uint32_t Addr, uint32_t Size, uint8_t *data);

  void range_assign(uint32_t Addr, uint32_t Size, uint32_t *data);

  bool check_hard(uint32_t Addr, uint32_t Size, uint8_t *data);

  float check_soft(uint32_t Addr, uint32_t Size, uint8_t *data);

  void scan_mem(uint32_t begin_addr, uint32_t size);
  void scan_mem();
  void set_v();
};