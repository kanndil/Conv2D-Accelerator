`timescale 1ns/1ps
`include "tb_macros.vh"

module conv_tb;

    // Parameters
    parameter DSIZE         = 1024;
    parameter KSIZE         = 4'd3;
    parameter DATA_WIDTH    = 8'd8;
    parameter DATA_HEIGHT   = 8'd8;
    parameter STRIDE_X      = 4'd1;
    parameter STRIDE_Y      = 4'd1;
    parameter KERNEL_WIDTH  = 4'd3;
    parameter KERNEL_HEIGHT = 4'd3;
    parameter RESULT_WIDTH = (DATA_WIDTH -  KERNEL_WIDTH)  / STRIDE_X;
    parameter RESULT_HEIGHT = (DATA_HEIGHT -  KERNEL_HEIGHT)  / STRIDE_Y;

    integer i, j,file_id;
    integer timer_start=0;
    integer timer_stop=0;

    
    reg start = 0;
     // Declare the register with appropriate size
    reg [8*KSIZE*KSIZE-1:0] kernel;
    reg [7:0] image [255:0]; 


    reg  rst_n = 0;
    `TB_CLK(clk, 10)
    `TB_DUMP("conv_tb.vcd", conv_tb, 0) 

    reg [$clog2(DSIZE):0] mem_input_addr = 0;
    reg [31:0]  mem_input_data = 0;
    reg mem_input_wr = 0;

    reg [$clog2(DSIZE):0]mem_output_addr = 0;
    wire [31:0] mem_output_data;
    wire done;

    conv #(.DSIZE(DSIZE),  .KSIZE(KSIZE)) DUV (
        .clk(clk),  
        .rst_n(rst_n),
        .data_width(DATA_WIDTH),    
        .data_height(DATA_HEIGHT),   
        .result_width(RESULT_WIDTH),   
        .result_height(RESULT_HEIGHT),
        .stride_x(STRIDE_X),    
        .stride_y(STRIDE_Y),
        .kernel(kernel),    
        .kernel_width(KERNEL_WIDTH),   
        .kernel_height(KERNEL_HEIGHT),
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

    reg [31:0] count = 0;
    always @(posedge clk) begin
        count = count + 1;
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

        timer_start = count;
        for (i = 0; i < 256; i +=4) begin
            mem_input_addr = i;
            //$display("address: %d", mem_input_addr);
            mem_input_data = {image[i+3], image[i+2], image[i+1], image[i]};
            #10;
        end
        
        timer_stop = count;

        $display("data input time: %d", timer_stop - timer_start);

        mem_input_wr = 0;
        $display("Data writen"); // Display the binary value  

        start = 1;      
        timer_start = count;
        #10;    
        start = 0;


        // Wait for the operation to complete
        wait (done);
        timer_stop = count;

        $display("compute time: %d", timer_stop - timer_start);

        timer_start = count;
        
        `PRINT_MEM_OUTPUT(RESULT_HEIGHT, RESULT_WIDTH, DATA_WIDTH, mem_output_addr, mem_output_data)
        `WRITE_MEM_OUTPUT_TO_FILE(RESULT_HEIGHT, RESULT_WIDTH, DATA_WIDTH, mem_output_addr, mem_output_data, "verilog_module_output.txt")
        

        timer_stop = count;
        $display("data output time: %d", timer_stop - timer_start);
        $display("Test Done");
        #10000;
        $finish;
    end
endmodule



