// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

// Receives a stream from a framebuffer and writes it into the memory
module AxisFramebufferWriter #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = 4,
    // Width of ID signal
    parameter ID_WIDTH = 8
) (
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire                         tstart,
    input  wire [ADDR_WIDTH - 1 : 0]    taddr,
    input  wire [ADDR_WIDTH - 1 : 0]    tbytes,
    output reg                          tdone,

    // When this is true, the m_xlast signal matches the address channel transfers (axi behavior).
    // If this is false, m_xlast is only asserted at the end of the stream (axis behavior).
    input  wire                         enableAxiLastSignal,

    // Read port
    input  wire                         s_xvalid,
    output reg                          s_xready,
    input  wire                         s_xlast,
    input  wire [DATA_WIDTH - 1 : 0]    s_xdata,
    input  wire [STRB_WIDTH - 1 : 0]    s_xstrb,

    // Write port
    output reg  [DATA_WIDTH - 1 : 0]    m_xdata,
    output reg  [STRB_WIDTH - 1 : 0]    m_xstrb,
    output reg                          m_xlast,
    output reg                          m_xvalid,
    input  wire                         m_xready,

    // Address port
    output wire [ID_WIDTH - 1 : 0]      m_axid,
    output wire [ADDR_WIDTH - 1 : 0]    m_axaddr,
    output wire [ 7 : 0]                m_axlen,
    output wire [ 2 : 0]                m_axsize,
    output wire [ 1 : 0]                m_axburst,
    output wire                         m_axlock,
    output wire [ 3 : 0]                m_axcache,
    output wire [ 2 : 0]                m_axprot, 
    output wire                         m_axvalid,
    input  wire                         m_axready

);
    localparam AWLEN = 15;
    localparam AWSIZE = $clog2(DATA_WIDTH / 8);

    wire axiLastSignal = ((counterNext >> AWSIZE) & AWLEN) == 0;
    wire axisLastSignal = (counterNext == dataSizeInBytes);

    reg  [ADDR_WIDTH - 1 : 0]   dataSizeInBytes;
    reg  [ADDR_WIDTH - 1 : 0]   counter;
    wire [ADDR_WIDTH - 1 : 0]   counterNext = counter + (DATA_WIDTH / 8);
    wire                        lastSignal = (axiLastSignal && enableAxiLastSignal) || (axisLastSignal && !enableAxiLastSignal);
    wire                        transferEnd = counterNext > dataSizeInBytes;

    reg                         startAddressGeneration;
    wire                        addressGenerationDone;

    reg  [DATA_WIDTH - 1 : 0]   wdataSkid;
    reg  [STRB_WIDTH - 1 : 0]   wstrbSkid;
    reg                         wlastSkid;
    reg                         wvalidSkid;
    reg                         skidValid;

    LinearAddressGenerator #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .AxLEN_BEATS_PER_TRANSFER(AWLEN),
        .AxSIZE_BYTES_PER_BEAT(AWSIZE)
    ) addressGenerator (
        .aclk(aclk),
        .resetn(resetn),

        .start(startAddressGeneration),
        .done(addressGenerationDone),

        .startAddr(taddr),
        .dataSizeInBytes(tbytes),

        .axid(m_axid),
        .axaddr(m_axaddr),
        .axlen(m_axlen),
        .axsize(m_axsize),
        .axburst(m_axburst),
        .axlock(m_axlock),
        .axcache(m_axcache),
        .axprot(m_axprot),
        .axvalid(m_axvalid),
        .axready(m_axready)
    );

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            tdone <= 1;
            startAddressGeneration <= 0;
            s_xready <= 0;
            m_xvalid <= 0;
            skidValid <= 0;
        end
        else
        begin
            if (tstart && addressGenerationDone)
            begin
                tdone <= 0;
                counter <= 0;
                s_xready <= 1;
                dataSizeInBytes <= tbytes;
                startAddressGeneration <= 1;
            end

            if (!tdone)
            begin
                if (skidValid && m_xready)
                begin
                    m_xdata <= wdataSkid; 
                    m_xvalid <= wvalidSkid; 
                    m_xstrb <= wstrbSkid; 
                    m_xlast <= wlastSkid;
                    skidValid <= 0;
                    s_xready <= !transferEnd; 
                end
                else if (!m_xvalid || m_xready)
                begin
                    m_xdata <= s_xdata;
                    m_xvalid <= s_xvalid;
                    m_xstrb <= s_xstrb;
                    m_xlast <= lastSignal;
                    s_xready <= !transferEnd;
                    if (s_xvalid)
                        counter <= counterNext;
                end
                else if (!skidValid && !m_xready && m_xvalid && s_xvalid)
                begin
                    wdataSkid <= s_xdata;
                    wvalidSkid <= s_xvalid;
                    wstrbSkid <= s_xstrb;
                    wlastSkid <= lastSignal;
                    s_xready <= 0;
                    skidValid <= 1;
                    counter <= counterNext;
                end

                if (transferEnd && m_xready && m_xvalid && !skidValid)
                begin
                    tdone <= 1;
                    m_xvalid <= 0;
                    s_xready <= 0;
                end
            end

            if (startAddressGeneration && !addressGenerationDone)
            begin
                startAddressGeneration <= 0;
            end
        end
    end
endmodule