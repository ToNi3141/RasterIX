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

// Based on the data on the fetch interface, it will request one vector 
// of pixels from the memory. To trigger a request, it will check the 
// vector boundaries. For instance, if the PIXEL_WIDTH is 16 and the 
// DATA_WIDTH is 32, then it will ignore the least significant bit on 
// the fetch interface and only fetches a new vector when the more 
// significant bits change.
// Performance: 1 pixel per cycle, 1 pixel vector every two cycles
module MemoryReadRequestGenerator #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of ID signal
    parameter ID_WIDTH = 8
) (
    input  wire                             aclk,
    input  wire                             resetn,

    /////////////////////////
    // Fragment Interface
    /////////////////////////

    // Fetch interface
    input  wire                             s_fetch_tvalid,
    input  wire                             s_fetch_tlast,
    output reg                              s_fetch_tready,
    input  wire [ADDR_WIDTH - 1 : 0]        s_fetch_taddr,

    /////////////////////////
    // Memory Interface
    /////////////////////////

    // Address channel
    output reg  [ID_WIDTH - 1 : 0]          m_mem_axi_arid,
    output reg  [ADDR_WIDTH - 1 : 0]        m_mem_axi_araddr,
    output reg  [ 7 : 0]                    m_mem_axi_arlen,
    output reg  [ 2 : 0]                    m_mem_axi_arsize,
    output reg  [ 1 : 0]                    m_mem_axi_arburst,
    output reg                              m_mem_axi_arlock,
    output reg  [ 3 : 0]                    m_mem_axi_arcache,
    output reg  [ 2 : 0]                    m_mem_axi_arprot,
    output reg                              m_mem_axi_arvalid,
    input  wire                             m_mem_axi_arready
);
    localparam DATA_WIDTH_LG = $clog2(DATA_WIDTH / 8);
    localparam ADDR_TAG_POS = DATA_WIDTH_LG;
    localparam ADDR_TAG_WIDTH = ADDR_WIDTH - ADDR_TAG_POS;

    reg [ADDR_TAG_WIDTH - 1 : 0]    lastAddrTag;
    reg [ADDR_TAG_WIDTH - 1 : 0]    newAddrTagSkid;
    reg                             lastSkid;
    reg                             lastFetch;
    reg                             memRequest;
    reg [ADDR_WIDTH - 1 : 0]        memRequestAddr;
    wire newMemRequest = (((lastAddrTag != s_fetch_taddr[ADDR_TAG_POS +: ADDR_TAG_WIDTH])));


    // Fetch handling
    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            lastAddrTag <= ~0;
            newAddrTagSkid <= ~0;
            lastSkid <= 0;
            lastFetch <= 0;

            s_fetch_tready <= 1;
        end
        else
        begin
            if (s_fetch_tready)
            begin
                if (s_fetch_tvalid)
                begin
                    // Safe current address in the skid buffer when the memory request handling is busy
                    if (newMemRequest && memRequest)
                    begin
                        s_fetch_tready <= 0;
                        newAddrTagSkid <= s_fetch_taddr[ADDR_TAG_POS +: ADDR_TAG_WIDTH];
                        lastSkid <= s_fetch_tlast;
                    end
                    else
                    begin
                        // If the boundaries of the tag are exceeded, trigger a new write request
                        if (newMemRequest)
                        begin
                            memRequestAddr <= { s_fetch_taddr[ADDR_TAG_POS +: ADDR_TAG_WIDTH], { (ADDR_TAG_POS){ 1'b0 } } };
                            memRequest <= 1;
                        end
                        
                        // If this is the last signal, set the lastAddrTag to max to enable a new request, when new addresses are comming
                        if (s_fetch_tlast)
                        begin
                            lastAddrTag <= ~0;
                        end
                        else
                        begin
                            lastAddrTag <= s_fetch_taddr[ADDR_TAG_POS +: ADDR_TAG_WIDTH];
                        end
                    end
                end
            end
            else
            begin
                if (!memRequest)
                begin
                    memRequestAddr <= { newAddrTagSkid, { (ADDR_TAG_POS){ 1'b0 } } };
                    memRequest <= 1;
                    s_fetch_tready <= 1;
                    if (lastSkid)
                    begin
                        lastAddrTag <= ~0;
                    end
                    else
                    begin
                        lastAddrTag <= newAddrTagSkid;
                    end
                end
            end
        end
         
        // Memory request handling
        if (!resetn)
        begin
            memRequest <= 0;

            m_mem_axi_arid <= 0;
            m_mem_axi_arlen <= 0; // Use always one beat. If the performance too slow, then we could increase the DATA_WIDTH and use an external bus converter
            m_mem_axi_arsize <= DATA_WIDTH_LG[0 +: 3];
            m_mem_axi_arburst <= 1;
            m_mem_axi_arlock <= 0;
            m_mem_axi_arcache <= 4'b0011;
            m_mem_axi_arprot <= 0;
            m_mem_axi_arvalid <= 0;
        end
        else
        begin
            // Memory request handling
            if (!m_mem_axi_arvalid)
            begin
                // If no pending memory request is available, then check if a new request has to be issued
                if (memRequest)
                begin
                    m_mem_axi_araddr <= memRequestAddr;
                    m_mem_axi_arvalid <= 1;
                    memRequest <= 0;
                end
            end
            else
            begin
                // Reset pending request when the arready signal was asserted
                if (m_mem_axi_arready)
                begin
                    m_mem_axi_arvalid <= 0;
                end
            end
        end
    end
endmodule