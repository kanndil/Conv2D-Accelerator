`define     TB_CLK(clk, period)                 reg clk=0; always #(period/2) clk <= !clk;
`define     TB_SRSTN(rstn, clk, duration)       reg rstn=0; initial begin #duration; @(posedge clk) rstn <= 1; end
`define     TB_ESRST(rst, pol, clk, duration)   reg rst=1'bx;\ 
                                                event e_assert_reset, e_reset_done; \
                                                initial forever begin \
                                                    @(e_assert_reset); \
                                                    rst = pol; \
                                                    #duration; \
                                                    @(posedge clk) rst = ~pol; \
                                                    -> e_reset_done; \
                                                end
`define     TB_DUMP(file, mod, level)           initial begin $dumpfile(file); $dumpvars(level, mod); end
`define     TB_FINISH(length)                   initial begin #``length``; $display("Verification Failed: Timeout");$finish; end
`define     TB_WAIT_FOR_CLOCK_CYC(clk, count)   repeat(count) begin @(posedge clk) end
`define     TB_TEST_EVENT(name)                 event e_``name``_start, e_``name``_done;
`define     TB_TEST_BEGIN(name)                 initial  begin : name \
                                                @(e_``name``_start);
`define     TB_TEST_END(name)                   -> e_``name``_done; \
                                                end 