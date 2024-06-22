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
    reg [7:0] image [255:0]; 
    integer i; // Declare integer variable for the loop
    integer j;

    reg  rst_n = 0;
    `TB_CLK(clk, 10)
    //`TB_SRSTN(rst_n, clk, 333)
    `TB_DUMP("conv_tb.vcd", conv_tb, 0) 

    reg [$clog2(DSIZE):0] mem_input_addr = 0;
    reg [31:0]  mem_input_data = 0;
    reg mem_input_wr = 0;

    reg [$clog2(DSIZE):0]mem_output_addr = 0;
    wire [31:0] mem_output_data;

    conv #(.DSIZE(DSIZE),  .KSIZE(KSIZE)) DUV (
        .clk(clk),  
        .rst_n(rst_n),
        .data_width(DATA_WIDTH),    
        .data_height(DATA_HEIGHT),   
        .di_x_stop(DATA_WIDTH-KSIZE),   
        .di_y_stop(DATA_HEIGHT-KSIZE),
        .stride_x(4'd1),    
        .stride_y(4'd1),
        .kernel(kernel),    
        .kernel_width(KSIZE),   
        .kernel_height(KSIZE),
        .mi_addr(mem_input_addr), 
        .mi_data(mem_input_data), 
        .mi_wr(mem_input_wr),
        .mo_addr(mem_output_addr), 
        .mo_data(mem_output_data),
        .start(start),  
        .done(done)
    );

    // Initial block to assign values to kernel and image
    initial begin
        kernel = {8'd1, 8'd0, -8'd1, 8'd1, 8'd0, -8'd1, 8'd1, 8'd0, -8'd1}; 
        for (i = 0; i < 256; i +=1) 
            image[i] = i;    
    end

    initial begin
        $display("start test");
        // Initialize signals
        clk = 0;    rst_n = 0; start = 0;
        mem_input_addr = 0; mem_input_data = 0; mem_input_wr = 0;
        mem_output_addr = 0;

        // Reset pulse
        #10;
        rst_n = 1;  #10;    rst_n = 0;  #10;    rst_n = 1;
        #10;

        // Write the input data
        mem_input_wr = 1;

        for (i = 0; i < 256; i +=4) begin
            mem_input_addr = i;
            //$display("address: %d", mem_input_addr);
            mem_input_data = {image[i+3], image[i+2], image[i+1], image[i]};
            $display("data: %d", mem_input_data[31:24], mem_input_data[23:16], mem_input_data[15:8], mem_input_data[7:0]);
            #10;
        end
        
        mem_input_wr = 0;
        $display("Data writen"); // Display the binary value  

        start = 1;      #10;    start = 0;

        // Wait for the operation to complete
        wait (done);

        `PRINT_MEM_OUTPUT(DATA_HEIGHT, DATA_WIDTH, mem_output_addr, mem_output_data)

        $display("Test Done");
        #100;
        $finish;
    end
endmodule



