`include "tb_macros.vh"

module conv_tb;

wire done;
reg start = 0;

`TB_CLK(clk, 10)
`TB_SRSTN(rst_n, clk, 333)
`TB_DUMP("conv_tb.vcd", conv_tb, 0)
`TB_FINISH(10_000)        

conv DUV (
    .clk(clk),
    .rst_n(rst_n),

    // Input data information
    .di_w(6),
    .di_h(8),
    .di_x_stop(3),
    .di_y_stop(5),

    // Stride information
    .stride_x(1),
    .stride_y(1),
    
    // The kernel information
    .k(),
    .k_w(3),
    .k_h(3),
    
    // Input Data Memory Ports
    .mi_addr(),
    .mi_data(),
    .mi_wr(),

    // Output Data Memory Ports
    .mo_addr(),
    .mo_data(),

    // Control and status
    .start(start),
    .done(done)
);

initial begin
    @(posedge rst_n);
    @(posedge clk);
    #1 start = 1;
    @(posedge clk);
    #1 start = 0;
end
endmodule