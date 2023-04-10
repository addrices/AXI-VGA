module bram_buffer #(parameter Bits = 32,parameter Add_Width = 9)(
    addr, clk, cen, wen, indata, outdata
);


parameter Depth = 1 << Add_Width;

input [Add_Width-1:0] addr;
input                 clk;
input                 cen;
input                 wen;
input  [Bits-1:0]     indata;
output [Bits-1:0]     outdata;

reg [Bits-1:0] ram [0:Depth-1];
reg [Bits-1:0] Q;

always@(posedge clk) begin
    if(cen & wen) begin
        ram[addr] <= indata;
    end
    Q <= cen && !wen ? ram[addr] : {$random};
end
assign outdata = Q;

endmodule