`define     CNTR(r, i)  always @(posedge clk, negedge rst_n) \
                            if(!rst_n) \
                                r <= i;\ 
                            else if(start)\ 
                                r <= i;\ 
                            else if(state == CALC)


`timescale 1ns/1ps
`default_nettype none

module conv #( parameter    DSIZE = 256,
                            KSIZE = 3)
(
    input                       clk,
    input                       rst_n,

    // Input data information
    input   [7:0]               di_w,
    input   [7:0]               di_h,
    input   [7:0]               di_x_stop,
    input   [7:0]               di_y_stop,

    // Stride information
    input   [3:0]               stride_x,
    input   [3:0]               stride_y,
    
    // The kernel information
    input   [8*KSIZE*KSIZE-1:0] k,
    input   [3:0]               k_w,
    input   [3:0]               k_h,
    
    // Input Data Memory Ports
    input   [$clog2(DSIZE)-3:0] mi_addr,
    input   [31:0]              mi_data,
    input                       mi_wr,

    // Output Data Memory Ports
    input   [$clog2(DSIZE)-3:0] mo_addr,
    output  [31:0]              mo_data,

    // Control and status
    input                       start,
    output                      done
);

    localparam [1:0]    IDLE = 2'b00,
                        CALC = 2'b01;

    reg [7:0]   DI[DSIZE-1:0];
    reg [7:0]   DO[DSIZE-1:0];

    wire [7:0]  di_addr, do_addr, k_addr;
    
    wire [7:0] kernel [KSIZE][KSIZE];
    
    genvar ri, ci;
    generate
        for(ri=0; ri<KSIZE; ri=ri+1)
            for(ci=0; ci<KSIZE; ci=ci+1)    
                assign kernel[ri][ci] = k[(ri*KSIZE+ci)*8+7:(ri*KSIZE+ci)*8];
    endgenerate

    // The Control FSM
    reg [1:0] state, nstate;

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
    reg[7:0]    di_x_cntr, di_y_cntr;
    reg[3:0]    k_x_cntr, k_y_cntr;

    wire    kernel_row_done = (k_x_cntr == (k_w - 1));
    wire    kernel_last_col = (k_y_cntr == (k_h - 1));
    wire    kernel_done     = (kernel_row_done && kernel_last_col);
    wire    di_row_done     = (di_x_cntr == di_x_stop) && kernel_done;
    wire    di_last_col     = (di_y_cntr == di_y_stop);
    wire    di_done         = (di_last_col) && di_row_done; 
    
    `CNTR(k_x_cntr, 0)
        if(kernel_row_done)
            k_x_cntr <= 0;
        else
            k_x_cntr <= k_x_cntr + 'b1;              
    
    `CNTR(k_y_cntr, 0)
        if(kernel_done)
            k_y_cntr <= 'b0;    
        else if(kernel_row_done)
            k_y_cntr <= k_y_cntr + 'b1;
    
    `CNTR(di_x_cntr, 0)
        if(di_row_done)
            di_x_cntr <= 0;
        else if(kernel_done)
            di_x_cntr <= di_x_cntr + stride_x;

    `CNTR(di_y_cntr, 0)
        if(di_done)
            di_y_cntr <= 0;
        else if(di_row_done)
            di_y_cntr <= di_y_cntr + stride_y;

    assign di_addr  =   di_x_cntr + di_y_cntr * di_w;
    assign k_addr   =   k_x_cntr + k_w * k_y_cntr; 
    
    // The MAC with saturation
    wire[15:0] mul;
    assign mul = DI[di_addr] * kernel[k_y_cntr][k_x_cntr];
    
    reg[15:0] acc;
    always @(posedge clk, negedge rst_n) 
        if(!rst_n) 
            acc <= 0;
        else if(kernel_done)
            acc <= 0;
        else
            acc <= acc + mul;

    // check whether we are >127 or <-128
    wire u = acc[15] & (~&acc[15:7]);
    wire o = ~acc[15] & (|acc[15:7]);
        
    // Change later
    always @(posedge clk)
        if(kernel_done)
            DO[di_addr] <= u ? 8'h80 : o ? 8'd127 : acc;

    assign done = di_done;

endmodule