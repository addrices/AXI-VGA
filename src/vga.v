module vga_ctrl(
  input clock,
  input resetn,
  input         io_master_awready,
  output        io_master_awvalid,
  output [31:0] io_master_awaddr,
  output [3:0]  io_master_awid,
  output [7:0]  io_master_awlen,
  output [2:0]  io_master_awsize,
  output [1:0]  io_master_awburst,
  input         io_master_wready,
  output        io_master_wvalid,
  output [31:0] io_master_wdata,
  output [3:0]  io_master_wstrb,
  output        io_master_wlast,
  output        io_master_bready,
  input         io_master_bvalid,
  input  [1:0]  io_master_bresp,
  input  [3:0]  io_master_bid,
  input         io_master_arready,
  output        io_master_arvalid,
  output [31:0] io_master_araddr,
  output [3:0]  io_master_arid,
  output [7:0]  io_master_arlen,
  output [2:0]  io_master_arsize,
  output [1:0]  io_master_arburst,
  output        io_master_rready,
  input         io_master_rvalid,
  input  [1:0]  io_master_rresp,
  input  [31:0] io_master_rdata,
  input         io_master_rlast,
  input  [3:0]  io_master_rid,

  output        io_slave_awready,
  input         io_slave_awvalid,
  input  [31:0] io_slave_awaddr,
  input  [2:0]  io_slave_awprot,
  output        io_slave_wready,
  input         io_slave_wvalid,
  input  [31:0] io_slave_wdata,
  input  [3:0]  io_slave_wstrb,
  input         io_slave_bready,
  output        io_slave_bvalid,
  output [1:0]  io_slave_bresp,
  output        io_slave_arready,
  input         io_slave_arvalid,
  input  [31:0] io_slave_araddr,
  input  [2:0]  io_slave_arprot,
  input         io_slave_rready,
  output        io_slave_rvalid,
  output [1:0]  io_slave_rresp,
  output [31:0] io_slave_rdata,

  output hsync,
  output vsync,
  output [3:0]vga_r,
  output [3:0]vga_g,
  output [3:0]vga_b,
  output error_led
);

  initial begin
    if ($test$plusargs("trace") != 0)  begin
        $display("[%0t] Tracing to logs/vlt_dump.vcd...\n", $time);
        $dumpfile("logs/vlt_dump.vcd");
        $dumpvars();
    end
    $display("[%0t] Model running...\n", $time);
  end

  parameter h_frontporch = 96;
	parameter h_active = 144;
	parameter h_backporch = 784;
	parameter h_total = 800;

	parameter v_frontporch = 2;
	parameter v_active = 35;
	parameter v_backporch = 515;
	parameter v_total = 525;

  reg   [9:0] x_cnt;
  reg   [9:0] y_cnt;
  wire        h_valid;
  wire        v_valid;
  wire        valid;
  wire        rgb_valid;
  wire        vga_begin;
  assign vga_begin = io_slave_wdata == 1 & waddr[3:2] == 0 & io_slave_wvalid & io_slave_wready && waddr[31:4] == 28'h8000000;

  always @(negedge resetn or posedge clock) begin
    if(!resetn || status[0] == 0)
      x_cnt <= 1;
    else begin
      if(x_cnt == h_total)
        x_cnt <= 1;
      else
        x_cnt <= x_cnt + 10'd1;
    end
  end

  always @(negedge resetn or posedge clock) begin
    if(!resetn || status[0] == 0)
      y_cnt <= 1;
    else if(status[0] != 0) begin
      if(y_cnt == v_total & x_cnt == h_total) 
        y_cnt <= 1;
      else if(x_cnt == h_total)
        y_cnt <= y_cnt + 10'd1;
    end
  end

  //同步信号
  assign hsync = (x_cnt > h_frontporch);
  assign vsync = (y_cnt > v_frontporch);
  //消隐信号
  assign h_valid = (x_cnt > h_active) & (x_cnt <= h_backporch);
  assign v_valid = (y_cnt > v_active) & (y_cnt <= v_backporch);
  assign valid = h_valid & v_valid;
  //计算当前有效像素坐标
  // assign h_addr = valid ? (x_cnt - h_active - 1):{10{1'b0}};
  // assign v_addr = valid ? (y_cnt - v_active - 1):{10{1'b0}};

  //axi-slave 使用一个AXILite
  parameter [1:0] sWidle = 0, sWdata = 1, sWresp = 2;
  parameter [1:0] sRidle = 0, sRdata = 1;
  reg [1:0] sWstate;
  reg [1:0] sRstate;
  // state
  reg [31:0] status [0:3];
  reg [31:0] waddr,raddr;
  integer i = 0;
  always @(negedge resetn or posedge clock) begin
    if(!resetn) begin
      waddr <= 0;
      raddr <= 0;
      for(i = 0;i <= 3;i = i+1)begin
        status[i] <= 0;
      end
    end
    else begin
      if(io_slave_awvalid & io_slave_awready)begin
        waddr <= io_slave_awaddr;  
      end
      if(io_slave_wvalid & io_slave_wready && waddr[31:4] == 28'h8000000)begin
        status[waddr[3:2]] <= io_slave_wdata;
      end
      if(io_slave_arvalid & io_slave_arready)begin
        raddr <= io_slave_araddr;
      end
    end
  end

  always @(negedge resetn or posedge clock) begin
    if(!resetn) begin
      sWstate <= sWidle;
      sRstate <= sRidle;
    end
    else begin
      case(sWstate)
        sWidle: sWstate <= io_slave_awvalid & io_slave_awready? sWdata : sWstate;
        sWdata: sWstate <= io_slave_wvalid & io_slave_wready? sWresp : sWstate;
        sWresp: sWstate <= io_slave_bvalid & io_slave_bready? sWidle : sWstate;
        default: sWstate <= sWstate;
      endcase
      case(sRstate)
        sRidle: sRstate <= io_slave_arvalid & io_slave_arready? sRdata : sRstate;
        sRdata: sRstate <= io_slave_rvalid & io_slave_rready? sRidle : sRstate;
        default: sRstate <= sRstate;
      endcase
    end
  end

  //axi-master 使用一个AXI4
  parameter [1:0] mWidle = 0, mWdata = 1, mWresp = 2;
  parameter [2:0] mRidle = 0, mRwait1 = 1, mRdata1 = 2, mRwait2 = 3, mRdata2 = 4;
  reg [1:0] mWstate;
  reg [2:0] mRstate;
  wire b0_in_bg;
  wire b1_in_bg;
  reg b0_in_flag; 
  reg b1_in_flag; 
  reg  [8:0] b_waddr; //buffer的写入地址
  reg  [31:0] axi_addr_offset;
  wire [31:0] axi_addr;
  wire [9:0] x_offset;
  wire [8:0] b_raddr; //buffer的读取地址
  wire [8:0] b0_addr;
  wire [8:0] b1_addr;
  wire [31:0] b0_outdata;
  wire [31:0] b1_outdata;
  wire [31:0] outdata; 
  assign outdata = y_cnt[0] == 0 ? b0_outdata : b1_outdata;
  assign b0_addr = b0_in_flag ? b_waddr : b_raddr;
  assign b1_addr = b1_in_flag ? b_waddr : b_raddr;
  assign x_offset = x_cnt - h_active;
  assign b_raddr = x_offset[9:1];
  assign axi_addr = status[1] + axi_addr_offset;
  assign b0_in_bg = y_cnt >= v_active && y_cnt < v_backporch & y_cnt[0] == 1 && x_cnt == 1 && status[0] == 1; //在偶数行的开始时读取下一行。
  assign b1_in_bg = y_cnt >= v_active && y_cnt < v_backporch & y_cnt[0] == 0 && x_cnt == 1 && status[0] == 1; //在扫描奇数行的时候读取下一行。
  assign error_led = (b0_in_bg | b1_in_bg) & mRstate != mRidle; //扫描来不及。
  assign vga_r = valid ? (x_cnt[0] == 1 ? outdata[3:0] : outdata[19:16]) : 0;
  assign vga_g = valid ? (x_cnt[0] == 1 ? outdata[7:4] : outdata[23:20]) : 0;
  assign vga_b = valid ? (x_cnt[0] == 1 ? outdata[11:8] : outdata[27:24]): 0;
  always @(negedge resetn or posedge clock) begin
    if(!resetn) begin
      mWstate <= mWidle;
      mRstate <= mRidle;
    end
    else begin
      case(mRstate)
        mRidle: mRstate <= b0_in_bg | b1_in_bg? mRwait1 : mRstate;
        mRwait1: mRstate <= io_master_arvalid & io_master_arready? mRdata1 : mRstate;
        mRdata1: mRstate <= io_master_rlast & io_master_rready & io_master_rvalid? mRwait2 : mRstate;
        mRwait2: mRstate <= io_master_arvalid & io_master_arready? mRdata2 : mRstate;
        mRdata2: mRstate <= io_master_rlast & io_master_rready & io_master_rvalid? mRidle : mRstate; 
        default: mRstate <= mRstate;
      endcase
    end
  end

  always @(negedge resetn or posedge clock) begin
    if(!resetn)begin
      axi_addr_offset <= 0;
      b0_in_flag <= 0;
      b1_in_flag <= 0;
      b_waddr <= 0;
    end
    else if(vga_begin) begin
      axi_addr_offset <= 0;
      b0_in_flag <= 0;
      b1_in_flag <= 0;
      b_waddr <= 0;
    end
    else if(mRstate == mRidle) begin
      if(b0_in_bg == 1)begin
        b0_in_flag <= 1;
        b_waddr <= 0;
      end
      else if(b1_in_bg == 1)begin
        b1_in_flag <= 1;
        b_waddr <= 0;
      end
    end
    else if(mRstate == mRwait1 || mRstate == mRwait2) begin
      if(io_master_arvalid & io_master_arready)begin
        if(axi_addr_offset == 32'h95D80) begin
          axi_addr_offset <= 0;
        end
        else begin
          axi_addr_offset <= axi_addr_offset + 32'h280;
        end
      end
    end
    else begin
      if(io_master_rvalid & io_master_rready) begin
        if(mRstate == mRdata2 & io_master_rlast & io_master_rready & io_master_rvalid & y_cnt == v_backporch-1) begin
          b_waddr <= 0;
        end
        else begin
          b_waddr <= b_waddr + 1;
        end
      end
      if(io_master_rlast & io_master_rready & io_master_rvalid & mRstate == mRdata2)begin
        b0_in_flag <= 0;
        b1_in_flag <= 0;
      end
    end
  end

  bram_buffer buffer0(b0_addr, clock, 1, io_master_rvalid & io_master_rready & b0_in_flag, io_master_rdata, b0_outdata);
  bram_buffer buffer1(b1_addr, clock, 1, io_master_rvalid & io_master_rready & b1_in_flag, io_master_rdata, b1_outdata);

  // always@(posedge clock) begin
  //   if(io_master_rvalid & io_master_rready & b0_in_flag) begin
  //     $display("write addr:%x data:%x",b0_addr,io_master_rdata);
  //   end
  // end

  assign io_master_awvalid = 0;
  assign io_master_awaddr = 0;
  assign io_master_awid = 0;
  assign io_master_awlen = 0;
  assign io_master_awsize = 2;
  assign io_master_wvalid = 0;
  assign io_master_wdata = 0;
  assign io_master_wstrb = 0;
  assign io_master_wlast = 0;
  assign io_master_bready = 0;
  assign io_master_arvalid = mRstate == mRwait1 || mRstate == mRwait2 ? 1:0;
  assign io_master_araddr = axi_addr;
  assign io_master_arid = 0;
  assign io_master_arlen = 8'd159;
  assign io_master_arsize = 2;
  assign io_master_arburst = 1;
  assign io_master_rready =  mRstate == mRdata1 || mRstate == mRdata2 ? 1:0;

  assign io_slave_awready = sWstate == sWidle;
  assign io_slave_wready  = sWstate == sWdata;
  assign io_slave_bvalid  = sWstate == sWresp;
  assign io_slave_bresp   = 0;
  assign io_slave_arready = sRstate == sRidle;
  assign io_slave_rvalid  = sRstate == sRdata;
  assign io_slave_rresp   = 0;
  assign io_slave_rdata   = status[raddr[1:0]];


endmodule