#include "cmem.h"

uint32_t CMem::get_value(string tmp)
{
  LOG_ASSERT(tmp.length() == 8, "tmp's length need to be 2");
  uint32_t re = 0;
  for (int i = 0; i < 8; i++)
  {
    char a = tmp.c_str()[i];
    LOG_ASSERT((a >= '0' && a <= '9') || (a >= 'A' && a <= 'F'), "%c\n", a);
    uint32_t a_v;
    if (a >= '0' && a <= '9')
      a_v = a - '0';
    else
      a_v = a - 'A' + 10;
    re = re * 16 + a_v;
  }
  return re;
}

CMem::CMem(uint32_t size, uint32_t addr, string filename)
{
  LOG_ASSERT(addr % 4096 == 0, "addr need to align 4kb\n");
  LOG_ASSERT(size % 4 == 0, "size need to be times of 4\n");
  Size = size;
  int Array_size = Size / 4;
  Data = new uint32_t[Array_size];
  BeginAddr = addr;
  ifstream initfile(filename, ios::in);
  LOG_ASSERT(initfile.is_open(), "%s is not open!!\n", filename.c_str());
  string temp;
  int Array_n = 0;
  int line_num = 0;
  while (getline(initfile, temp))
  {
    line_num++;
    if (Array_n == Array_size)
      LOG_ASSERT(false, "initfile is too big\n");
    LOG_ASSERT(temp.length() == 8, "line %d length is not right\n", line_num);
    Data[Array_n] = get_value(temp);
    Array_n += 1;
  }
  for (int i = Array_n; i < Array_size; i++)
  {
    Data[i] = 0;
  }
  initfile.close();
}

CMem::CMem(uint32_t size, uint32_t addr)
{
  LOG_ASSERT(addr % 4096 == 0, "addr need to align 4kb\n");
  LOG_ASSERT(size % 4 == 0, "size need to be times of 4\n");
  Size = size;
  Data = new uint32_t[size];
  BeginAddr = addr;
}

uint32_t CMem::Read32(uint32_t addr)
{
  int n = (addr - BeginAddr) / 4;
  // printf("%x %",addr,BeginAddr);
  LOG_ASSERT(n < Size && n >= 0, "0x%x not in this Mem(n:%d)\n", addr, n);
  return Data[n];
}

uint64_t CMem::Read64(uint32_t addr)
{
  int n = (addr - BeginAddr) / 4;
  LOG_ASSERT(n < Size - 1 && n >= 0, "0x%x not in this Mem\n", addr);
  uint64_t re = Data[n + 1];
  return (re << 32) + Data[n];
}

bool CMem::Write32(uint32_t addr, uint32_t data)
{
  int n = (addr - BeginAddr) / 4;
  LOG_ASSERT(n < Size - 1 && n >= 0, "0x%x not in this Mem\n", addr);
  Data[n] = data;
  return true;
}

void CMem::write2file(string writefilename)
{
  ofstream cMemfile(writefilename);
  if (cMemfile.is_open() == true)
  {
    tracef("isopen\n");
  }
  else
  {
    tracef("notopen\n");
  }
  if (cMemfile.is_open() == true)
  {
    tracef("cMem is writing filename:%s\n", writefilename.c_str());
    for (uint32_t i = 0; i < Size / 4; i++)
    {
      cMemfile << setiosflags(ios::uppercase) << hex << setw(8) << setfill('0') << Data[i] << endl;
    }
  }
  else
  {
    LOG_ASSERT(1, "filename %s can't open.\n", writefilename.c_str());
  }
  cMemfile.close();
  return;
}

void CMem::range_assign(uint32_t Addr, uint32_t Size, uint8_t *data)
{
  LOG_ASSERT(Addr % 4 == 0, "Addr must be times of 4\n");
  LOG_ASSERT(Size % 4 == 0, "Size must be times of 4\n");
  int b_sub = (Addr - BeginAddr) / 4;
  for (int i = 0; i < Size / 4; i++)
  {
    uint32_t ByteData = ((uint32_t)data[4 * i + 3] << 24) + ((uint32_t)data[4 * i + 2] << 16) + ((uint32_t)data[4 * i + 1] << 8) + (uint32_t)data[4 * i];
    Data[b_sub + i] = ByteData;
  }
  return;
}

void CMem::range_assign(uint32_t Addr, uint32_t Size, uint32_t *data)
{
  LOG_ASSERT(Addr % 4 == 0, "Addr must be times of 4\n");
  LOG_ASSERT(Size % 4 == 0, "Size must be times of 4\n");
  int b_sub = (Addr - BeginAddr) / 4;
  for (int i = 0; i < Size / 4; i++)
  {
    uint32_t ByteData = data[i];
    Data[b_sub + i] = ByteData;
  }
  return;
}
bool CMem::check_hard(uint32_t Addr, uint32_t Size, uint8_t *data)
{
  LOG_ASSERT(Addr % 4 == 0, "Addr must be times of 4\n");
  LOG_ASSERT(Size % 4 == 0, "Size must be times of 4\n");
  int b_sub = (Addr - BeginAddr) / 4;
  for (int i = 0; i < Size / 4; i++)
  {
    uint32_t ByteData = ((uint32_t)data[4 * i + 3] << 24) + ((uint32_t)data[4 * i + 2] << 16) + ((uint32_t)data[4 * i + 1] << 8) + (uint32_t)data[4 * i];
    // printf("ByteData:%x Data:%x\n", ByteData, Data[b_sub + i]);
    if (Data[b_sub + i] != ByteData)
    {
      warnf("Addr:%x RightData:%x MemData:%x\n", Addr + i * 4, ByteData, Data[b_sub + i]);
      return false;
    }
  }
  return true;
}
float CMem::check_soft(uint32_t Addr, uint32_t Size, uint8_t *data)
{
  LOG_ASSERT(Addr % 4 == 0, "Addr must be times of 4\n");
  LOG_ASSERT(Size % 4 == 0, "Size must be times of 4\n");
  int b_sub = (Addr - BeginAddr) / 4;
  uint32_t right_num = 0;
  for (int i = 0; i < Size / 4; i++)
  {
    uint32_t ByteData = ((uint32_t)data[4 * i + 3] << 24) + ((uint32_t)data[4 * i + 2] << 16) + ((uint32_t)data[4 * i + 1] << 8) + (uint32_t)data[4 * i];
    uint8_t b1 = (Data[b_sub + i] << 24) >> 24;
    uint8_t b2 = (Data[b_sub + i] << 16) >> 24;
    uint8_t b3 = (Data[b_sub + i] << 8) >> 24;
    uint8_t b4 = Data[b_sub + i] >> 24;
    if (b1 == data[4 * i])
      right_num++;
    if (b2 == data[4 * i + 1])
      right_num++;
    if (b3 == data[4 * i + 2])
      right_num++;
    if (b4 == data[4 * i + 3])
      right_num++;
  }
  float right_rate = right_num / Size;
  printf("right_rate:%f\n", right_rate);
  return right_rate;
}
void CMem::scan_mem(uint32_t begin_addr, uint32_t size)
{
  LOG_ASSERT(begin_addr >= BeginAddr && begin_addr + size <= BeginAddr + size && begin_addr % 4 == 0 && size % 4 == 0, "begin_addr:%x size:%d not right\n", begin_addr, size);
  int sub = (begin_addr - BeginAddr) / 4;
  for (int i = sub; i < sub + (size / 4); i++)
  {
    printf("addr:%x data:%x\n", begin_addr + i * 4, Data[i]);
  }
  return;
};
void CMem::scan_mem()
{
  scan_mem(BeginAddr, Size);
};

void CMem::set_v()
{
  int a = 1;
  // for(int i = 0;i < 0x25800;i++){
  //   uint32_t p = a | (a<<4) | (a<<8) | (a<<16) | (a<<20) | (a<<24);
  //   a = (a+1) % 15;
  //   Write32(0x90000000 + i*4,p);
  // }
  for(int i = 0; i < 480;i++){
    for(int j = 0; j < 320;j++){
      int addr = (i*320+j) * 4;
      int c = (i%3)*4;
      uint32_t p = (15 << (c+16)) | (15 << c);
      if(i != 479){
        Write32(0x90000000 + addr, p);
      }
      else{
        
        Write32(0x90000000 + addr, (14 << (c+16)) | (14 << c));
      }
    }
  }
  return;
}