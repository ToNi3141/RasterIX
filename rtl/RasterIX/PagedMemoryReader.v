// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2023 ToNi3141

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

// Reads a page from the memory and streams it via the AXIS interface.
// It expects on the s_axis addresses. Each address on the m_axis results
// in one read page and streamed to the s_axis.
// The last m_axis_tlast page will trigger a s_axis_tlast on the last
// beat.
module PagedMemoryReader #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // The size of a page in bytes
    parameter PAGE_SIZE = 2048
) (
    input  wire                         aclk,
    input  wire                         resetn,

    output reg                          m_axis_tvalid,
    output reg                          m_axis_tlast,
    output reg  [DATA_WIDTH - 1 : 0]    m_axis_tdata,

    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [31 : 0]                s_axis_tdata,

    // Memory interface
    output reg  [ID_WIDTH - 1 : 0]      m_mem_axi_arid,
    output reg  [ADDR_WIDTH - 1 : 0]    m_mem_axi_araddr,
    output reg  [ 7 : 0]                m_mem_axi_arlen,
    output reg  [ 2 : 0]                m_mem_axi_arsize,
    output reg  [ 1 : 0]                m_mem_axi_arburst,
    output reg                          m_mem_axi_arlock,
    output reg  [ 3 : 0]                m_mem_axi_arcache,
    output reg  [ 2 : 0]                m_mem_axi_arprot,
    output reg                          m_mem_axi_arvalid,
    input  wire                         m_mem_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]      m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]    m_mem_axi_rdata,
    input  wire [ 1 : 0]                m_mem_axi_rresp,
    input  wire                         m_mem_axi_rlast,
    input  wire                         m_mem_axi_rvalid,
    output reg                          m_mem_axi_rready
);
    localparam BYTES_PER_BEAT = DATA_WIDTH / 8;
    localparam LG_BEAT_SIZE = $clog2(BYTES_PER_BEAT);

    
    localparam BEATS = 512 / BYTES_PER_BEAT;

    localparam INCREMENT = BEATS << LG_BEAT_SIZE;
    initial 
    begin
        if (PAGE_SIZE < 1024)
        begin
            $error("PAGE_SIZE must be at least 1024 bytes");
        end
    end
    
    initial 
    begin
        m_mem_axi_arid = 0;
        m_mem_axi_arlen = BEATS[0 +: 8] - 1;
        m_mem_axi_arsize = LG_BEAT_SIZE[2 : 0];
        m_mem_axi_arburst = 1;
        m_mem_axi_arlock = 0;
        m_mem_axi_arcache = 0;
        m_mem_axi_arprot = 0;
    end

    localparam RECEIVE_ADDR = 0;
    localparam REQUEST_MEMORY = 1;
    localparam WAIT_FOR_MEMORY = 2;
    localparam LAST_TRANSFER = 3;

    reg  [15 : 0]               index;
    reg  [ADDR_WIDTH - 1 : 0]   addr;
    reg  [ 1 : 0]               state;
    reg                         lastTransfer;
    reg  [15 : 0]               transfersRequested;
    reg  [15 : 0]               transfersReceived;

    // Memory Request
    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            s_axis_tready <= 1;

            m_mem_axi_arvalid <= 0;

            lastTransfer <= 0;

            transfersRequested <= 0;
            
            state <= RECEIVE_ADDR;
        end
        else
        begin
            case (state)
                RECEIVE_ADDR:
                begin
                    if (s_axis_tvalid)
                    begin
                        addr <= s_axis_tdata;
                        index <= 0;
                        s_axis_tready <= 0;
                        lastTransfer <= s_axis_tlast;
                        state <= REQUEST_MEMORY;
                    end
                end
                REQUEST_MEMORY:
                begin
                    m_mem_axi_araddr <= addr + { { (ADDR_WIDTH - 16) { 1'b0 } }, index };
                    index <= index + INCREMENT[15 : 0];
                    transfersRequested <= transfersRequested + 1;
                    m_mem_axi_arvalid <= 1;
                    state <= WAIT_FOR_MEMORY;
                end
                WAIT_FOR_MEMORY:
                begin
                    if (m_mem_axi_arready)
                    begin
                        m_mem_axi_arvalid <= 0;
                        if (index == PAGE_SIZE)
                        begin
                            state <= LAST_TRANSFER;
                        end
                        else
                        begin
                            state <= REQUEST_MEMORY;
                        end
                    end
                end
                LAST_TRANSFER:
                begin
                    // Stall here till the last data has transferred through the read channel
                    if (!lastTransfer)
                    begin
                        s_axis_tready <= 1;
                        state <= RECEIVE_ADDR;
                    end
                end
            endcase
        end

        if (!resetn)
        begin
            m_axis_tvalid <= 0;
            m_mem_axi_rready <= 1;

            transfersReceived <= 0;
        end
        else
        begin
            m_axis_tvalid <= m_mem_axi_rvalid;
            m_axis_tdata <= m_mem_axi_rdata;
            if (m_mem_axi_rvalid)
            begin
                if (m_mem_axi_rlast)
                begin
                    transfersReceived <= transfersReceived + 1;
                end
                if (m_mem_axi_rlast && lastTransfer && (transfersRequested == (transfersReceived + 1)))
                begin
                    lastTransfer <= 0;
                    transfersRequested <= 0;
                    transfersReceived <= 0;
                    m_axis_tlast <= 1;
                end
                else
                begin
                    m_axis_tlast <= 0;
                end
            end
        end
    end

endmodule