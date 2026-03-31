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

module CoalesceAddrGen #(
    parameter ID_WIDTH   = 4,
    parameter ADDR_WIDTH = 32,
    parameter DATA_WIDTH = 32,
    parameter MAX_BEATS_TO_COALESCE = 8
) 
(
    input  wire                                 aclk,
    input  wire                                 resetn,

    input  wire [ID_WIDTH - 1 : 0]              s_mem_axi_axid,
    input  wire [ADDR_WIDTH - 1 : 0]            s_mem_axi_axaddr,
    input  wire [ 7 : 0]                        s_mem_axi_axlen, // How many beats are in this transaction
    input  wire [ 2 : 0]                        s_mem_axi_axsize, // The increment during one cycle. Means, 0 incs addrStart by 1, 2 by 4 and so on
    input  wire [ 1 : 0]                        s_mem_axi_axburst, // 0 fixed, 1 incr, 2 wrapping
    input  wire                                 s_mem_axi_axlock,
    input  wire [ 3 : 0]                        s_mem_axi_axcache,
    input  wire [ 2 : 0]                        s_mem_axi_axprot, 
    input  wire                                 s_mem_axi_axvalid,
    output reg                                  s_mem_axi_axready,

    // Output
    output reg  [ID_WIDTH - 1 : 0]              m_mem_axi_axid,
    output reg  [ADDR_WIDTH - 1 : 0]            m_mem_axi_axaddr,
    output reg  [ 7 : 0]                        m_mem_axi_axlen, // How many beats are in this transaction
    output wire [ 2 : 0]                        m_mem_axi_axsize, // The increment during one cycle. Means, 0 incs addrStart by 1, 2 by 4 and so on
    output wire [ 1 : 0]                        m_mem_axi_axburst, // 0 fixed, 1 incr, 2 wrapping
    output wire                                 m_mem_axi_axlock,
    output wire [ 3 : 0]                        m_mem_axi_axcache,
    output wire [ 2 : 0]                        m_mem_axi_axprot, 
    output reg                                  m_mem_axi_axvalid,
    input  wire                                 m_mem_axi_axready
);
    localparam TIMEOUT_MAX = (DATA_WIDTH / 8) + 1; // In cycles
    localparam ADDR_BOUNDARY_MASK = ~(4096 - 1); // AXI requires a 4KB boundary
    localparam AXSIZE = $clog2(DATA_WIDTH / 8);

    reg [ 7 : 0]                axlen;
    reg [ADDR_WIDTH - 1 : 0]    axaddr;
    reg [ADDR_WIDTH - 1 :0]     axaddr_last;
    reg                         coalescing_running;
    reg [ 7 : 0]                timeout;

    reg [ADDR_WIDTH - 1 : 0]    skid_addr;
    reg                         skid_valid;

    assign m_mem_axi_axburst = 1; // INCR burst
    assign m_mem_axi_axsize = AXSIZE[0 +: 3];
    assign m_mem_axi_axlock = 0;
    assign m_mem_axi_axcache = 4'b0011;
    assign m_mem_axi_axprot = 0;

    always @(posedge aclk) 
    begin
        if (!resetn) 
        begin
           s_mem_axi_axready <= 1;
           m_mem_axi_axvalid <= 0;
           skid_valid <= 0;
           coalescing_running <= 0;
           axlen <= 0; 
           timeout <= 0;
        end 
        else 
        begin : coalescing_logic
            reg boundary_check_failed;
            reg timeout_occurred;
            reg max_beats_reached;
            reg addr_order_failed;

            boundary_check_failed = 0;
            timeout_occurred = 0;
            max_beats_reached = 0;
            addr_order_failed = 0;

            timeout <= timeout + 1;

            if (m_mem_axi_axvalid && m_mem_axi_axready)
            begin
                m_mem_axi_axvalid <= 0;
            end

            timeout_occurred = (timeout >= TIMEOUT_MAX) && coalescing_running;
            max_beats_reached = (axlen >= (MAX_BEATS_TO_COALESCE - 1)) && coalescing_running;

            // New address (don't coalesce when a flush is already pending)
            if (s_mem_axi_axvalid && s_mem_axi_axready && coalescing_running
                && !timeout_occurred && !max_beats_reached && !skid_valid)
            begin
                timeout <= 0;
                // Check boundary
                if ((s_mem_axi_axaddr & ADDR_BOUNDARY_MASK) == (axaddr & ADDR_BOUNDARY_MASK)) // Same 4KB boundary
                begin
                    if ((axaddr_last + (1 << AXSIZE)) == s_mem_axi_axaddr)
                    begin
                        axlen <= axlen + 1;
                        axaddr_last <= s_mem_axi_axaddr;
                    end
                    else
                    begin
                        addr_order_failed = 1;
                    end
                end
                else
                begin
                    boundary_check_failed = 1;
                end
            end

            if (boundary_check_failed || timeout_occurred || max_beats_reached || addr_order_failed || !coalescing_running || skid_valid)
            begin
                if (m_mem_axi_axvalid && !m_mem_axi_axready && coalescing_running)
                begin
                    if (!skid_valid)
                    begin
                        // Skid: save incoming beat, stall input
                        skid_addr <= s_mem_axi_axaddr;
                        skid_valid <= s_mem_axi_axvalid;
                        s_mem_axi_axready <= !s_mem_axi_axvalid;
                    end
                    // else: output busy and skid full, stall until downstream accepts
                end
                else
                begin
                    if (coalescing_running)
                    begin
                        m_mem_axi_axaddr <= axaddr;
                        m_mem_axi_axlen <= axlen;
                        m_mem_axi_axvalid <= 1;
                    end

                    if (skid_valid)
                    begin
                        axaddr <= skid_addr;
                        axaddr_last <= skid_addr;
                        coalescing_running <= 1;
                        skid_valid <= 0;
                    end
                    else
                    begin
                        axaddr <= s_mem_axi_axaddr;
                        axaddr_last <= s_mem_axi_axaddr;
                        coalescing_running <= s_mem_axi_axvalid;
                    end
                    axlen <= 0;
                    s_mem_axi_axready <= 1;
                    timeout <= 0;

                    if (s_mem_axi_axvalid && (s_mem_axi_axlen != 0))
                    begin
                        $error("Only single beat transactions are supported for coalescing!");
                        $finish;
                    end
                    if (s_mem_axi_axvalid && (s_mem_axi_axsize != AXSIZE[0 +: 3]))
                    begin
                        $error("Only transactions with size matching DATA_WIDTH are supported for coalescing!");
                        $finish;
                    end
                end
            end
        end
    end
endmodule