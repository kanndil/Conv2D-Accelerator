`timescale 1ns/1ps
`include "tb_macros.vh"

module conv_tb;

    // Parameters
    parameter DSIZE = 256;
    parameter KSIZE = 4'd3;
    parameter DATA_WIDTH = 8'd8;
    parameter DATA_HEIGHT = 8'd8;

    wire done;
    reg start = 0;
     // Declare the register with appropriate size
    reg [8*KSIZE*KSIZE-1:0] kernel;
    reg [7:0] image [DATA_WIDTH*DATA_HEIGHT-1:0]; 
    integer i; // Declare integer variable for the loop


    `TB_CLK(clk, 10)
    `TB_SRSTN(rst_n, clk, 333)
    `TB_DUMP("conv_tb.vcd", conv_tb, 0)
    `TB_FINISH(1000000)   

    reg [$clog2(DSIZE)-3:0] mem_input_addr = 0;
    reg [31:0]  mem_input_data = 0;
    reg mem_input_wr = 0;

    reg [$clog2(DSIZE)-3:0]mem_output_addr = 0;
    wire [31:0] mem_output_data;

    conv #(.DSIZE(DSIZE),  .KSIZE(KSIZE)) DUV (
        //Clock and reset
        .clk(clk),  .rst_n(rst_n),

        // Input data information
        .data_width(DATA_WIDTH),    .data_hight(DATA_HEIGHT),   .di_x_stop(DATA_WIDTH-KSIZE),   .di_y_stop(DATA_HEIGHT-KSIZE),

        // Stride information
        .stride_x(4'd1),    .stride_y(4'd1),
        
        // The kernel information
        .kernel(kernel),    .kernel_width(KSIZE),   .kernel_hight(KSIZE),
        
        // Input Data Memory Ports
        .mi_addr(mem_input_addr), .mi_data(mem_input_data), .mi_wr(mem_input_wr),

        // Output Data Memory Ports
        .mo_addr(mem_output_addr), .mo_data(mem_output_data),

        // Control and status
        .start(start),  .done(done)
    );



    // Initial block to assign values to kernel and image
    initial begin
         kernel = {8'd1, 8'd0, 8'd1, 8'd1, 8'd0, 8'd1, 8'd1, 8'd0, 8'd1}; 
        $display("Kernel value: %d", kernel); // Display the binary value

        for (i = 0; i < DATA_WIDTH*DATA_HEIGHT; i +=1) begin
            image[i] = i;
            $display("Image value: %d", image[i]); // Display the binary value   
        end    
    end



    initial begin
        @(posedge rst_n);
        @(posedge clk);
        #1 start = 1;
        @(posedge clk);
        #1 start = 0;
    end
endmodule


