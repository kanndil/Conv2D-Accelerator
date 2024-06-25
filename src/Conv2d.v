`timescale 1ns/1ps
`default_nettype none

`define CNTR(r, i)  always @(posedge clk or negedge rst_n) \
                        if(!rst_n) \
                            r <= i; \
                        else if(start) \
                            r <= i; \
                        else if(state == CALC)

module conv #(parameter DSIZE = 1024, KSIZE = 5) // This is the maximum kernel size
(
    input                       clk,        // System clock
    input                       rst_n,      // Asynchronous reset active low

    // Input data information
    input   [7:0]               data_width,     // Input data width
    input   [7:0]               data_height,    // Input data height
    input   [7:0]               result_width,      // result_width  = (data_width -  kernel_width)  / stride_x; (to save HW)
    input   [7:0]               result_height,     // result_height = (data_height - kernel_height) / stride_y; (to save HW)

    // Stride information
    input   [3:0]               stride_x,       // Stride length in x direction 
    input   [3:0]               stride_y,       // Stride length in y direction

    // The kernel information
    input   [8*KSIZE*KSIZE-1:0] kernel,         // Convolution Kernel
                                                // KSIZE^2 x 8 bits
    input   [3:0]               kernel_width,        
    input   [3:0]               kernel_height,        

    // Input Data Memory Ports
    input   [$clog2(DSIZE):0]   mi_addr,        // $clog2(DSIZE): the ceiling of the base-2 logarithm of DSIZE,
                                                // gives the number of bits needed to address DSIZE unique locations.
    input   [31:0]              mi_data,        // Input data 32 bit word
    input                       mi_wr,

    // Output Data Memory Ports
    input   [$clog2(DSIZE):0]   mo_addr,    
    output  [31:0]              mo_data,

    // Control and status
    input                       start,
    output                      done
);

    localparam [1:0] IDLE = 2'b00, CALC = 2'b01;

    reg [7:0]   DI  [DSIZE-1:0];
    reg [7:0]   DO  [DSIZE-1:0];

    wire [7:0]  di_addr, k_addr;

    /* 
        +----+           +----+
        |    v           |    v
        +------+  start  +------+
        | IDLE |-------->| CALC |
        +------+         +------+
        ^                 |
        |       done      |
        +-----------------+
    */

    // The Control FSM
    reg [1:0] state  = IDLE;
    reg [1:0] nstate = IDLE;

    always @(posedge clk or negedge rst_n)
        if(!rst_n)
            state <= IDLE;
        else
            state <= nstate;

    always @* begin
        case(state)
            IDLE:   
                if(start)
                    nstate = CALC;
                else
                    nstate = IDLE;
            CALC:   
                if(done)
                    nstate = IDLE;
                else
                    nstate = CALC;
            default: 
                nstate = state;
        endcase
    end

    // DI Write 
    always @(posedge clk)
        if(mi_wr)
            {DI[mi_addr+3], DI[mi_addr+2], DI[mi_addr+1], DI[mi_addr]} <= mi_data;

    // DO Read
    assign mo_data = {DO[mo_addr+3], DO[mo_addr+2], DO[mo_addr+1], DO[mo_addr]};

    // Address Counters
    reg[7:0]    di_x_cntr = 0;
    reg[7:0]    di_y_cntr = 0;
    reg[3:0]    k_x_cntr  = 0;
    reg[3:0]    k_y_cntr  = 0;

    wire kernel_row_done = (k_x_cntr == (kernel_width - 1));
    wire kernel_last_col = (k_y_cntr == (kernel_height - 1));
    wire kernel_done     = (kernel_row_done && kernel_last_col);
    wire di_row_done     = (di_x_cntr >= result_width) && kernel_done;
    wire di_last_col     = (di_y_cntr >= result_height) && di_row_done ;
    wire di_done         = (di_last_col && di_row_done);

    `CNTR(k_x_cntr, 0)
        if (kernel_row_done)       k_x_cntr <= 0;
        else                       k_x_cntr <= k_x_cntr + 1;

    `CNTR(k_y_cntr, 0)
        if (kernel_done)           k_y_cntr <= 0;
        else if (kernel_row_done)  k_y_cntr <= k_y_cntr + 1;

    `CNTR(di_x_cntr, 0)
        if (di_row_done)           di_x_cntr <= 0;
        else if (kernel_done)      di_x_cntr <= di_x_cntr + 1;

    `CNTR(di_y_cntr, 0)
        if (di_last_col)           di_y_cntr <= 0;
        else if(di_row_done)       di_y_cntr <= di_y_cntr + 1;

    wire [7:0] image_x = di_x_cntr * stride_x + k_x_cntr;
    wire [7:0] image_y = di_y_cntr * stride_y + k_y_cntr;

    assign di_addr  = image_x + data_width * image_y;
    assign k_addr   = k_x_cntr + kernel_width * k_y_cntr;
    wire [7:0] do_addr = di_x_cntr + data_width * di_y_cntr;


    reg kernel_done_delayed=0; 
    reg [7:0] do_addr_delayed=0;
    reg di_done_delayed =0; 
    reg di_done_delayed_2 =0;
    always @(posedge clk, negedge rst_n) 
        if(!rst_n) begin
                kernel_done_delayed <= 0;
                do_addr_delayed <= 0;
                di_done_delayed <= 0;
                di_done_delayed_2<=0;
        end
        else begin
            kernel_done_delayed <= kernel_done;
            do_addr_delayed     <= do_addr;
            di_done_delayed     <= di_done;
            di_done_delayed_2 <=di_done_delayed;
        end


    wire [15:0] mul;  reg[15:0] acc = 0;


    assign mul = DI[di_addr] * kernel[k_addr*8];

    always @(posedge clk, negedge rst_n) 
         if(!rst_n) 
             acc <= 0;
        else if(state == CALC) begin
            if(kernel_done_delayed)
                acc <= mul;
            else
                acc <= acc + mul;
        end

    // check whether we are >127 or <-128
     wire u = acc[15] & (~&acc[15:7]);
     wire o = ~acc[15] & (|acc[15:7]);

     // Change later
     always @(posedge clk)
         if(kernel_done_delayed)
            DO[do_addr_delayed] <= u ? 8'h80 : o ? 8'd127 : acc;
            
    assign done = di_done_delayed_2;

endmodule
