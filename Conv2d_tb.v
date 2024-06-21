`timescale 1ns/1ps
`include "tb_macros.vh"

module conv_tb;

    // Parameters
    parameter DSIZE = 256;
    parameter KSIZE = 3;

    wire done;
    reg start = 0;
     // Declare the register with appropriate size
    reg [8*KSIZE*KSIZE-1:0] k;

    // Initial block to assign values
    initial begin
        k = {8'd1, 8'd0, 8'b11111111, 8'd1, 8'd0, 8'b11111111, 8'd1, 8'd0, 8'b11111111}; 
        $display("Assigned value: %b", k); // Display the binary value
    end

    `TB_CLK(clk, 10)
    `TB_SRSTN(rst_n, clk, 333)
    `TB_DUMP("conv_tb.vcd", conv_tb, 0)
    `TB_FINISH(1000000)        

    conv #(
        .DSIZE(DSIZE),
        .KSIZE(KSIZE)
    ) DUV (
        .clk(clk),
        .rst_n(rst_n),

        // Input data information
        .data_width(8'd6),
        .data_hight(8'd8),
        .di_x_stop(8'd3),
        .di_y_stop(8'd5),

        // Stride information
        .stride_x(4'd1),
        .stride_y(4'd1),
        
        // The kernel information
        .kernel(k),
        .kernel_width(4'd3),
        .kernel_hight(4'd3),
        
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