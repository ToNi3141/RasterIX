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

// Reads stream with a vector of pixels from the memory interface. Based on
// the index from the fetch interface, it serializes the vector into pixels
// available on the fragment interface.
// Performance: 1 pixel per cycle
module FramebufferSerializer #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Sets the width of a pixel to fix the memory alignment
    parameter PIXEL_WIDTH = 16
) (
    input  wire                             aclk,
    input  wire                             resetn,

    /////////////////////////
    // Fragment Interface
    /////////////////////////

    // Framebuffer Interface
    output reg                              m_frag_tvalid,
    input  wire                             m_frag_tready,
    output reg  [PIXEL_WIDTH - 1 : 0]       m_frag_tdata,
    output reg  [ADDR_WIDTH - 1 : 0]        m_frag_taddr,
    output reg                              m_frag_tlast,

    // Fetch interface
    input  wire                             s_fetch_tvalid,
    input  wire                             s_fetch_tlast,
    output reg                              s_fetch_tready,
    input  wire [ADDR_WIDTH - 1 : 0]        s_fetch_taddr,

    /////////////////////////
    // Memory Interface
    /////////////////////////

    // Data channel
    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_mem_axi_rdata,
    input  wire [ 1 : 0]                    m_mem_axi_rresp,
    input  wire                             m_mem_axi_rlast,
    input  wire                             m_mem_axi_rvalid,
    output reg                              m_mem_axi_rready
);
    localparam ADDR_BYTE_POS_POS = $clog2(PIXEL_WIDTH / 8);
    localparam ADDR_BYTE_POS_WIDTH = $clog2(DATA_WIDTH / PIXEL_WIDTH);
    localparam ADDR_TAG_POS = $clog2(DATA_WIDTH / 8);
    localparam ADDR_TAG_WIDTH = ADDR_WIDTH - ADDR_TAG_POS;

    reg [ADDR_TAG_WIDTH - 1 : 0]    addrTag;
    reg [ADDR_WIDTH - 1 : 0]        addrSkid;
    reg                             lastSkid;
    reg [DATA_WIDTH - 1 : 0]        cacheLine;
    

    localparam STATE_NORMAL = 0;
    localparam STATE_SKID_MEM_BUSY = 1;
    localparam STATE_SKID_DEST_BUSY = 2;

    reg [ 2 : 0]                    state;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            m_frag_tvalid <= 0;
            m_frag_tlast <= 0;

            s_fetch_tready <= 1;

            m_mem_axi_rready <= 0;

            addrTag <= ~0;

            state <= STATE_NORMAL;

        end
        else
        begin : Fetch
            reg tagOk;
            tagOk = (addrTag == s_fetch_taddr[ADDR_TAG_POS +: ADDR_TAG_WIDTH]) && s_fetch_tvalid;
            
            if (m_mem_axi_rready)
            begin
                m_mem_axi_rready <= 0;
            end

            case (state)
                STATE_NORMAL: 
                begin
                    // Valid data available 
                    if (tagOk)
                    begin
                        // The destination can consume data
                        if (m_frag_tready || !m_frag_tvalid) // Dest is ready or does not have valid data
                        begin
                            s_fetch_tready <= 1;
                            m_frag_tdata <= cacheLine[PIXEL_WIDTH * s_fetch_taddr[ADDR_BYTE_POS_POS +: ADDR_BYTE_POS_WIDTH] +: PIXEL_WIDTH];
                            m_frag_taddr <= s_fetch_taddr;
                            m_frag_tvalid <= 1;
                            m_frag_tlast <= s_fetch_tlast;
                            if (s_fetch_tlast)
                            begin
                                addrTag <= ~0;
                            end
                            state <= STATE_NORMAL;
                        end
                        // The destination stalls
                        else
                        begin
                            s_fetch_tready <= 0;
                            addrSkid <= s_fetch_taddr;
                            lastSkid <= s_fetch_tlast;
                            state <= STATE_SKID_DEST_BUSY;
                        end
                    end
                    // No valid data available and data needs to be fetched from the memory
                    else
                    begin 
                        // Only fetch new data from memory, when a new address is available
                        if (s_fetch_tvalid) 
                        begin
                            // Try to read new data from the memory bus
                            if (m_mem_axi_rvalid && !m_mem_axi_rready) // Only read from the memory when data is available a complete ready cycle was executed
                            begin
                                cacheLine <= m_mem_axi_rdata;
                                addrTag <= s_fetch_taddr[ADDR_TAG_POS +: ADDR_TAG_WIDTH];
                                m_mem_axi_rready <= 1;
                                // The destination can consume data
                                if (m_frag_tready || !m_frag_tvalid) // Dest is ready or does not have valid data
                                begin
                                    s_fetch_tready <= 1;
                                    m_frag_tdata <= m_mem_axi_rdata[PIXEL_WIDTH * s_fetch_taddr[ADDR_BYTE_POS_POS +: ADDR_BYTE_POS_WIDTH] +: PIXEL_WIDTH];
                                    m_frag_taddr <= s_fetch_taddr;
                                    m_frag_tvalid <= 1;
                                    m_frag_tlast <= s_fetch_tlast;
                                    if (s_fetch_tlast)
                                    begin
                                        addrTag <= ~0;
                                    end
                                    state <= STATE_NORMAL;
                                end
                                // The destination stalls
                                else
                                begin
                                    s_fetch_tready <= 0;
                                    addrSkid <= s_fetch_taddr;
                                    lastSkid <= s_fetch_tlast;
                                    state <= STATE_SKID_DEST_BUSY;
                                end
                            end
                            // Go into skid state and try there again to fetch new data
                            else
                            begin
                                // When the destination consumed the pending data, stall it now because no data to serialize is available
                                if (m_frag_tvalid && m_frag_tready)
                                begin
                                    m_frag_tvalid <= 0;
                                end
                                s_fetch_tready <= 0;
                                addrSkid <= s_fetch_taddr;
                                lastSkid <= s_fetch_tlast;
                                state <= STATE_SKID_MEM_BUSY;
                            end
                        end
                        // Otherwise handle the signals from the destination
                        else
                        begin
                            // When the destination consumed the pending data, stall it now because no new address is available to serialize the data
                            if (m_frag_tvalid && m_frag_tready)
                            begin
                                m_frag_tvalid <= 0;
                            end
                        end
                    end
                end
                STATE_SKID_MEM_BUSY:
                begin
                    if (m_mem_axi_rvalid) // Wait till the memory is ready
                    begin
                        cacheLine <= m_mem_axi_rdata;
                        addrTag <= addrSkid[ADDR_TAG_POS +: ADDR_TAG_WIDTH];
                        m_mem_axi_rready <= 1;
                        // The destination can consume data
                        if (m_frag_tready || !m_frag_tvalid) // Dest is ready or does not have valid data
                        begin
                            s_fetch_tready <= 1;
                            m_frag_tdata <= m_mem_axi_rdata[PIXEL_WIDTH * addrSkid[ADDR_BYTE_POS_POS +: ADDR_BYTE_POS_WIDTH] +: PIXEL_WIDTH];
                            m_frag_taddr <= addrSkid;
                            m_frag_tvalid <= 1;
                            m_frag_tlast <= lastSkid;
                            if (lastSkid)
                            begin
                                addrTag <= ~0;
                            end
                            state <= STATE_NORMAL;
                        end
                        // The destination stalls
                        else
                        begin
                            s_fetch_tready <= 0;
                            // addrSkid contains already the skidded address 
                            state <= STATE_SKID_DEST_BUSY;
                        end
                    end 
                    else
                    if (m_frag_tvalid && m_frag_tready)
                    begin
                        m_frag_tvalid <= 0;
                    end
                end
                STATE_SKID_DEST_BUSY:
                begin
                    if (m_frag_tready || !m_frag_tvalid) // Dest is ready or does not have valid data
                    begin
                        s_fetch_tready <= 1;
                        m_frag_tdata <= cacheLine[PIXEL_WIDTH * addrSkid[ADDR_BYTE_POS_POS +: ADDR_BYTE_POS_WIDTH] +: PIXEL_WIDTH];
                        m_frag_taddr <= addrSkid;
                        m_frag_tvalid <= 1;
                        m_frag_tlast <= lastSkid;
                        if (lastSkid)
                        begin
                            addrTag <= ~0;
                        end
                        state <= STATE_NORMAL;
                    end
                end

                default: 
                begin
                end
            endcase
        end
    end 
endmodule