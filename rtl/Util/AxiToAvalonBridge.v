// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2026 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// AXI4 slave to Avalon MM master bridge.
//
// - Fixed 16-bit data width (1:1 mapping, no width conversion).
// - Only INCR bursts are supported on the AXI side; they map directly to
//   Avalon burst transfers.
// - AXI byte addresses are right-shifted by 1 to produce Avalon word
//   addresses (16-bit / 2-byte words).
// - Read and write channels share the Avalon port via round-robin
//   arbitration.  When both channels have pending requests the one that
//   was NOT last served wins.
// - Single outstanding transaction per channel (no ID reordering).

module AxiToAvalonBridge #(
    parameter ADDR_WIDTH = 32,
    parameter ID_WIDTH   = 8
) (
    input  wire                         aclk,
    input  wire                         resetn,

    // -------------------------------------------------------------------
    // AXI4 Slave – Write Address Channel
    // -------------------------------------------------------------------
    input  wire [ID_WIDTH - 1 : 0]      s_axi_awid,
    input  wire [ADDR_WIDTH - 1 : 0]    s_axi_awaddr,
    input  wire [ 7 : 0]                s_axi_awlen,
    input  wire [ 2 : 0]                s_axi_awsize,
    input  wire [ 1 : 0]                s_axi_awburst,
    input  wire                         s_axi_awvalid,
    output reg                          s_axi_awready,

    // AXI4 Slave – Write Data Channel
    input  wire [15 : 0]                s_axi_wdata,
    input  wire [ 1 : 0]                s_axi_wstrb,
    input  wire                         s_axi_wlast,
    input  wire                         s_axi_wvalid,
    output reg                          s_axi_wready,

    // AXI4 Slave – Write Response Channel
    output reg [ID_WIDTH - 1 : 0]       s_axi_bid,
    output reg [ 1 : 0]                 s_axi_bresp,
    output reg                          s_axi_bvalid,
    input  wire                         s_axi_bready,

    // -------------------------------------------------------------------
    // AXI4 Slave – Read Address Channel
    // -------------------------------------------------------------------
    input  wire [ID_WIDTH - 1 : 0]      s_axi_arid,
    input  wire [ADDR_WIDTH - 1 : 0]    s_axi_araddr,
    input  wire [ 7 : 0]                s_axi_arlen,
    input  wire [ 2 : 0]                s_axi_arsize,
    input  wire [ 1 : 0]                s_axi_arburst,
    input  wire                         s_axi_arvalid,
    output reg                          s_axi_arready,

    // AXI4 Slave – Read Data Channel
    output wire [ID_WIDTH - 1 : 0]      s_axi_rid,
    output wire [15 : 0]                s_axi_rdata,
    output reg  [ 1 : 0]                s_axi_rresp,
    output wire                         s_axi_rlast,
    output wire                         s_axi_rvalid,
    input  wire                         s_axi_rready,

    // -------------------------------------------------------------------
    // Avalon MM Master
    // -------------------------------------------------------------------
    output reg                          avm_write,
    output reg                          avm_read,
    output reg  [ADDR_WIDTH - 1 : 0]    avm_address,
    output reg  [15 : 0]                avm_writedata,
    output reg  [ 1 : 0]                avm_byteenable,
    output reg  [ 7 : 0]                avm_burstcount,
    input  wire [15 : 0]                avm_readdata,
    input  wire                         avm_readdatavalid,
    input  wire                         avm_waitrequest
);
    localparam GRANT_WRITE = 1'b0;
    localparam GRANT_READ  = 1'b1;

    localparam W_IDLE       = 2'd0;
    localparam W_DATA       = 2'd1;
    localparam W_INIT       = 2'd2;
    localparam W_RESP       = 2'd3;

    localparam R_IDLE       = 2'd0;
    localparam R_REQUEST    = 2'd1;
    localparam R_COLLECT    = 2'd2;
    // Current owner of the Avalon port
    reg                         grant;

    // -- Write channel --
    reg  [ 1 : 0]               w_state;
    reg  [ID_WIDTH - 1 : 0]     w_id;
    reg  [15 : 0]               w_data_skid;
    reg  [ 1 : 0]               w_strb_skid;
    reg                         w_last_skid;
    reg                         w_skid_valid;

    // -- Read channel --
    reg  [ID_WIDTH - 1 : 0]     r_id;
    wire                        r_last; // last beat of the burst (rlast)
    reg  [ 1 : 0]               r_state;
    reg  [ 7 : 0]               r_beat_cnt;    // read-data beats delivered to AXI


    wire                        r_fifo_full;
    wire                        r_fifo_empty;

    sfifo #(
        .BW             (1 + 16),  // { rlast, rdata }
        .LGFLEN         (7),
        .OPT_ASYNC_READ (1'b0)
    ) r_fifo_inst (
        .i_clk          (aclk),
        .i_reset        (!resetn),

        .i_wr           (avm_readdatavalid),
        .i_data         ({ r_last, avm_readdata }),

        .o_full         (r_fifo_full),
        .o_fill         (),

        .i_rd           (s_axi_rready),
        .o_data         ({ s_axi_rlast, s_axi_rdata }),
        .o_empty        (r_fifo_empty)
    );
    assign r_last = (r_beat_cnt == avm_burstcount - 1);                        
    
    assign s_axi_rvalid = !r_fifo_empty;
    assign s_axi_rid    = r_id;

    always @(posedge aclk)
    begin
        // Read channel state machine
        if (!resetn) begin
            grant         <= GRANT_READ;
            r_state       <= R_IDLE;
            avm_read      <= 1'b0;
            s_axi_arready <= 1'b0;
            s_axi_rresp   <= 2'b00; // OKAY
        end 
        else
        begin
            case (r_state)
            R_IDLE:
            begin
                if (grant == GRANT_READ)
                begin
                    if (r_fifo_empty && s_axi_arvalid) 
                    begin
                        s_axi_arready  <= 1'b1;
                        r_id           <= s_axi_arid;
                        avm_address    <= s_axi_araddr >> 1;  // Convert byte address to word address
                        avm_burstcount <= s_axi_arlen + 1;    // AXI burst length is number of beats - 1
                        avm_read       <= 1'b1;
                        r_beat_cnt     <= 0;
                        r_state        <= R_REQUEST;
                    end
                    else
                    begin
                        grant <= GRANT_WRITE;
                    end
                end
            end
            R_REQUEST:
            begin
                s_axi_arready <= 1'b0;
                if (!avm_waitrequest) 
                begin
                    avm_read <= 1'b0;
                    r_state <= R_COLLECT;
                end
            end
            R_COLLECT:
            begin
                if (r_beat_cnt >= avm_burstcount)
                begin
                    r_state <= R_IDLE;
                    grant <= GRANT_WRITE;
                end
            end
            default: begin end
            endcase

            if (avm_readdatavalid)
            begin
                r_beat_cnt <= r_beat_cnt + 1;
            end
        end

        // Write channel state machine
        if (!resetn) begin
            w_state       <= W_IDLE;
            avm_write     <= 1'b0;
            s_axi_awready <= 1'b0;
            s_axi_wready  <= 1'b0;
            w_skid_valid  <= 1'b0;
            s_axi_bvalid  <= 1'b0;
        end
        else
        begin   
            case (w_state)
            W_IDLE:
            begin
                if (grant == GRANT_WRITE)
                begin
                    if (s_axi_awvalid && s_axi_wvalid && !s_axi_bvalid) 
                    begin
                        s_axi_awready  <= 1'b1;
                        s_axi_wready   <= 1'b1;
                        w_state        <= W_INIT;
                    end
                    else
                    begin
                        grant <= GRANT_READ;
                    end
                end
            end
            W_INIT:
            begin
                s_axi_awready <= 1'b0;

                w_id           <= s_axi_awid;
                avm_burstcount <= s_axi_awlen + 1;    // AXI burst length is number of beats - 1
                avm_address    <= s_axi_awaddr >> 1;  // Convert byte address to word address
                avm_byteenable <= s_axi_wstrb;
                avm_writedata  <= s_axi_wdata;
                avm_write      <= 1'b1;

                if (s_axi_wlast)
                begin
                    s_axi_wready <= 1'b0;
                    s_axi_bid    <= s_axi_awid;
                    s_axi_bvalid <= 1'b1;
                    s_axi_bresp  <= 2'b00; // OKAY
                    w_state      <= W_RESP;
                end
                else
                begin
                    w_state <= W_DATA;
                end
            end
            W_DATA:
            begin
                if (s_axi_wvalid && !w_skid_valid) 
                begin
                    if (avm_waitrequest && avm_write)
                    begin
                        w_data_skid  <= s_axi_wdata;
                        w_strb_skid  <= s_axi_wstrb;
                        w_last_skid  <= s_axi_wlast;
                        s_axi_wready <= 1'b0;
                        w_skid_valid <= 1'b1;
                    end
                    else
                    begin
                        avm_writedata  <= s_axi_wdata;
                        avm_byteenable <= s_axi_wstrb;
                        avm_write      <= 1'b1;
                        if (s_axi_wlast) 
                        begin
                            s_axi_wready <= 1'b0;
                            s_axi_bid    <= w_id;
                            s_axi_bvalid <= 1'b1;
                            s_axi_bresp  <= 2'b00; // OKAY
                            w_state      <= W_RESP;
                        end
                    end
                end
                else if (w_skid_valid)
                begin
                    if (!avm_waitrequest)
                    begin
                        avm_writedata  <= w_data_skid;
                        avm_byteenable <= w_strb_skid;
                        avm_write      <= 1'b1;
                        s_axi_wready   <= !w_last_skid;
                        if (w_last_skid)
                        begin
                            s_axi_bid    <= w_id;
                            s_axi_bvalid <= 1'b1;
                            s_axi_bresp  <= 2'b00; // OKAY
                            w_state      <= W_RESP;
                        end
                        w_skid_valid <= 1'b0;
                    end
                end
                else
                begin
                    if (!avm_waitrequest)
                    begin
                        avm_write <= 1'b0;
                    end
                end
            end
            W_RESP:
            begin
                s_axi_wready <= 1'b0;

                if (s_axi_bready) 
                begin
                    s_axi_bvalid <= 1'b0;
                end
                
                if (!avm_waitrequest)
                begin
                    avm_write <= 1'b0;
                end

                if (!s_axi_bvalid && !avm_waitrequest)
                begin
                    grant   <= GRANT_READ;
                    w_state <= W_IDLE;
                end
            end
            endcase
        end
    end
endmodule

