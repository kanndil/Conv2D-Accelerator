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




`define PRINT_MEM_OUTPUT(result_height, result_width,  DATA_WIDTH, mem_output_addr, mem_output_data) \
    begin \
        for (i = 0; i <= result_height; i += 1) begin \
            for (j = 0; j <= result_width; j += 1) begin \
                mem_output_addr = i * DATA_WIDTH + j; \
                $write(" %d", mem_output_data[7:0]); \
            end \
            $write("\n"); \
        end \
    end


`define WRITE_MEM_OUTPUT_TO_FILE(result_height, result_width, DATA_WIDTH, mem_output_addr, mem_output_data, filename) \
    begin \
        file_id = $fopen(filename, "w"); \
        if (file_id == 0) begin \
            $display("Error: Could not open file."); \
            $finish; \
        end \
        for (i = 0; i <= result_height; i += 1) begin \
            for (j = 0; j <= result_width; j += 1) begin \
                mem_output_addr = i * DATA_WIDTH + j; \
                $fwrite(file_id, " %d", mem_output_data[7:0]); \
            end \
            $fwrite(file_id, "\n"); \
        end \
        $fclose(file_id); \
    end
