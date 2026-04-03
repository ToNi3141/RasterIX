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

// Collects pixels from the fragment interface and deserializes them into
// a vector of pixels.
// If the boundaries of the fragment addresses exceeds the vector, it will
// trigger a memory write request. For instance, if the DATA_WIDTH is 32 
// the PIXEL_WIDTH is 16, then it will deserialize two pixel together in 
// one vector. It will do it as long the one LSb changes. Of higher
// significant bits in the taddr change, then it will trigger a write request
// indifferent if the vector is fully deserialized, it will trigger a write
// request with an adapted strobe. It is the counter part to the
// FrameBufferReader.
// Performance: 1 pixel per cycle, 1 write request every two cycles
module FramebufferPacker #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = (DATA_WIDTH / 8),
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Size of the pixels
    parameter PIXEL_WIDTH = 16,
    localparam PIXEL_MASK_WIDTH = PIXEL_WIDTH / 8,
    localparam PIXEL_WIDTH_LG = $clog2(PIXEL_WIDTH / 8)
) (
    input   wire                            aclk,
    input   wire                            resetn,

    /////////////////////////
    // Fragment interface
    /////////////////////////

    // Framebuffer Interface
    input  wire                             s_frag_tvalid,
    input  wire                             s_frag_tlast,
    output reg                              s_frag_tready,
    input  wire [PIXEL_WIDTH - 1 : 0]       s_frag_tdata,
    input  wire [PIXEL_MASK_WIDTH - 1 : 0]  s_frag_tstrb,
    input  wire [ADDR_WIDTH - 1 : 0]        s_frag_taddr,

    /////////////////////////
    // Memory Interface
    /////////////////////////

    // Address channel
    output reg  [ID_WIDTH - 1 : 0]          m_mem_axi_awid,
    output reg  [ADDR_WIDTH - 1 : 0]        m_mem_axi_awaddr,
    output reg  [ 7 : 0]                    m_mem_axi_awlen, // How many beats are in this transaction
    output reg  [ 2 : 0]                    m_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addr by 1, 2 by 4 and so on
    output reg  [ 1 : 0]                    m_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrappig
    output reg                              m_mem_axi_awlock,
    output reg  [ 3 : 0]                    m_mem_axi_awcache,
    output reg  [ 2 : 0]                    m_mem_axi_awprot, 
    output reg                              m_mem_axi_awvalid,
    input  wire                             m_mem_axi_awready,

    // Data channel
    output reg  [DATA_WIDTH - 1 : 0]        m_mem_axi_wdata,
    output reg  [STRB_WIDTH - 1 : 0]        m_mem_axi_wstrb,
    output reg                              m_mem_axi_wlast,
    output reg                              m_mem_axi_wvalid,
    input  wire                             m_mem_axi_wready,

    // Write response channel
    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_bid,
    input  wire [ 1 : 0]                    m_mem_axi_bresp,
    input  wire                             m_mem_axi_bvalid,
    output reg                              m_mem_axi_bready
);
    localparam DATA_WIDTH_LG = $clog2(DATA_WIDTH / 8);
    localparam INDEX_BYTE_POS = PIXEL_WIDTH_LG;
    localparam INDEX_BYTE_WIDTH = $clog2(DATA_WIDTH / PIXEL_WIDTH);
    localparam INDEX_TAG_POS = DATA_WIDTH_LG;
    localparam INDEX_TAG_WIDTH = ADDR_WIDTH - INDEX_TAG_POS;

    // Workaround: Vivado may mis-synthesize variable-indexed LHS part-selects
    // (e.g. reg[var +: W] <= val). Use explicit shift+OR on the RHS instead.
    `define PLACE_PIXEL(pos, val)  ({{(DATA_WIDTH  - PIXEL_WIDTH){1'b0}},     (val)} << (PIXEL_WIDTH     * (pos)))
    `define PLACE_STROBE(pos, val) ({{(STRB_WIDTH  - PIXEL_MASK_WIDTH){1'b0}}, (val)} << (PIXEL_MASK_WIDTH * (pos)))

    reg [ADDR_WIDTH - 1 : 0]            memRequestAddr;
    reg [DATA_WIDTH - 1 : 0]            memRequestData;
    reg [STRB_WIDTH - 1 : 0]            memRequestStrb;
    reg                                 memRequest;

    reg  [INDEX_TAG_WIDTH - 1 : 0]      lastAddrTag;
    reg                                 optDataInLine;
    reg  [DATA_WIDTH - 1 : 0]           line;
    reg  [STRB_WIDTH - 1 : 0]           lineStrobe;

    reg  [INDEX_TAG_WIDTH - 1 : 0]      lastAddrTagSkid;
    reg  [PIXEL_WIDTH - 1 : 0]          pixelSkid;
    reg  [PIXEL_MASK_WIDTH - 1 : 0]     lineStrobeSkid;
    reg  [INDEX_BYTE_WIDTH - 1 : 0]     bytePosSkid;
    reg                                 lastSkid;
    reg                                 wasLast;

    wire [INDEX_TAG_WIDTH - 1 : 0]      tag;
    wire [INDEX_BYTE_WIDTH - 1 : 0]     bytePos;
            
    assign tag = s_frag_taddr[INDEX_TAG_POS +: INDEX_TAG_WIDTH];
    assign bytePos = s_frag_taddr[INDEX_BYTE_POS +: INDEX_BYTE_WIDTH];

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            memRequest <= 0;
            s_frag_tready <= 1;
            lastAddrTag <= 0;
            optDataInLine <= 0;
            lineStrobe <= 0;
            lineStrobeSkid <= 0;
            lastSkid <= 0;
            wasLast <= 0;
        end
        else
        begin
            if (s_frag_tready)
            begin
                if (s_frag_tvalid)
                begin
                    // if last tag equals the current tag of the current pixel
                    // then write the pixel into the line and update the strobes
                    if (lastAddrTag == tag)
                    begin
                        line       <= line       | `PLACE_PIXEL(bytePos,  s_frag_tdata);
                        lineStrobe <= lineStrobe | `PLACE_STROBE(bytePos, s_frag_tstrb);
                        optDataInLine <= 1;
                        wasLast <= s_frag_tlast;
                        // The last state needs a special handling. Go into the skid 
                        // state to trigger a write request.
                        if (s_frag_tlast)
                        begin
                            lastSkid <= 0;
                            lineStrobeSkid <= 0;
                            s_frag_tready <= 0;
                        end
                    end
                    else
                    begin
                        if (!memRequest)
                        begin
                            // Write request must be executed because the current tag is not equal to the last one
                            // This case is the fast case where no memory request is pending the memory request 
                            // can be executed immediately. The current pixel can then directly written into the 
                            // new line.
                            memRequestAddr <= { lastAddrTag, { DATA_WIDTH_LG { 1'b0 } } };
                            memRequestData <= line;
                            memRequestStrb <= lineStrobe;
                            memRequest <= optDataInLine;
                            
                            optDataInLine <= 1;
                            lastAddrTag <= tag;
                            line       <= `PLACE_PIXEL(bytePos,  s_frag_tdata);
                            lineStrobe <= `PLACE_STROBE(bytePos, s_frag_tstrb);
                            wasLast <= s_frag_tlast;
                            if (s_frag_tlast)
                            begin
                                lastSkid <= 0;
                                lineStrobeSkid <= 0;
                                s_frag_tready <= 0;
                            end
                        end
                        else
                        begin
                            // This is the slow case where a memory request is still pending. The pixel must be written 
                            // into the skid buffer and the stream must be interrupted.
                            s_frag_tready <= 0;
                            lastAddrTagSkid <= tag;
                            bytePosSkid <= bytePos;
                            pixelSkid <= s_frag_tdata;
                            lineStrobeSkid <= s_frag_tstrb;
                            lastSkid <= s_frag_tlast;
                        end
                    end
                end
            end
            else
            begin
                // This is the skid state. It will wait till the memory request can be executed and as soon, that is the 
                // case the skid state will be left.
                if (!memRequest)
                begin
                    // Create memory request
                    memRequestAddr <= { lastAddrTag, { DATA_WIDTH_LG { 1'b0 } } };
                    memRequestData <= line;
                    memRequestStrb <= lineStrobe;
                    memRequest <= optDataInLine;
                    
                    if (wasLast)
                    begin
                        // If it was the last pixel, reset the address tag
                        lastAddrTag <= ~0;
                        wasLast <= 0;
                        optDataInLine <= 0;
                    end
                    else
                    begin
                        // Load data from the skid buffer
                        lastAddrTag <= lastAddrTagSkid;
                        wasLast <= lastSkid;
                        optDataInLine <= 1;
                    end
                    
                    line       <= `PLACE_PIXEL(bytePosSkid,  pixelSkid);
                    lineStrobe <= `PLACE_STROBE(bytePosSkid, lineStrobeSkid);
                    if (lastSkid)
                    begin
                        // If it was the last pixel, go immediately into the skid state to trigger a 
                        // write request.
                        lastSkid <= 0;
                        lineStrobeSkid <= 0;
                        s_frag_tready <= 0;
                    end
                    else
                    begin
                        s_frag_tready <= 1;
                    end
                end
            end
        end

        // Memory request handling
        if (!resetn)
        begin
            m_mem_axi_awid <= 0;
            m_mem_axi_awlen <= 0; // Use always one beat. If the performance too slow, then we could increase the DATA_WIDTH and use an external bus converter
            m_mem_axi_awsize <= DATA_WIDTH_LG[0 +: 3];
            m_mem_axi_awburst <= 1;
            m_mem_axi_awlock <= 0;
            m_mem_axi_awcache <= 4'b0011;
            m_mem_axi_awprot <= 0;
            m_mem_axi_awvalid <= 0;

            m_mem_axi_wlast <= 1; // Always the last beat, since there is only one beat.
            m_mem_axi_wvalid <= 0;

            m_mem_axi_bready <= 1;

            memRequest <= 0;
        end
        else
        begin
            if (!m_mem_axi_awvalid && !m_mem_axi_wvalid)
            begin
                // If no pending memory request is available, then check if a new request has to be issued
                if (memRequest)
                begin
                    m_mem_axi_awaddr <= memRequestAddr;
                    m_mem_axi_awvalid <= |memRequestStrb;
                    m_mem_axi_awid <= m_mem_axi_awid + 1;

                    m_mem_axi_wdata <= memRequestData;
                    m_mem_axi_wstrb <= memRequestStrb;
                    m_mem_axi_wvalid <= |memRequestStrb; // Trigger memory request only when there is something to write
        
                    memRequest <= 0;
                end
            end
            else
            begin
                // Reset pending request when the xrready signal was asserted
                if (m_mem_axi_awready)
                begin
                    m_mem_axi_awvalid <= 0;
                end

                if (m_mem_axi_wready)
                begin
                    m_mem_axi_wvalid <= 0;
                end
            end
        end
    end

    `undef PLACE_PIXEL
    `undef PLACE_STROBE
endmodule
