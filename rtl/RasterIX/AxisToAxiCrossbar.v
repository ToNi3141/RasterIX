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

// Crossbar to write an AXI Stream to an AXI memory. Several AXI Streams
// can be connected to this crossbar, and the crossbar will select one
// of the streams to write to the memory.
// It uses a dedicated AXI Stream address channel to transfer the address and
// an read/write flag to transfer data to, or from, the memory.
module AxisToAxiCrossbar #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = 4,
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Number of ports
    parameter NPRT = 2
) (
    input  wire                                 aclk,
    input  wire                                 resetn,

    // Address port
    input  wire [NPRT - 1 : 0]                  s_avalid,
    input  wire [NPRT - 1 : 0]                  s_arnw, // address read not or write, 0 = read, 1 = write
    input  wire [(NPRT * ADDR_WIDTH) - 1 : 0]   s_aaddr,
    input  wire [(NPRT * ADDR_WIDTH) - 1 : 0]   s_abeats,
    output reg  [NPRT - 1 : 0]                  s_aready,

    // Write port
    input  wire [NPRT - 1 : 0]                  s_wvalid,
    output wire [NPRT - 1 : 0]                  s_wready,
    input  wire [NPRT - 1 : 0]                  s_wlast,
    input  wire [(NPRT * DATA_WIDTH) - 1 : 0]   s_wdata,
    input  wire [(NPRT * STRB_WIDTH) - 1 : 0]   s_wstrb,

    // Read port
    output wire [(NPRT * DATA_WIDTH) - 1 : 0]   m_rdata,
    output wire [NPRT - 1 : 0]                  m_rlast,
    output wire [NPRT - 1 : 0]                  m_rvalid,
    input  wire [NPRT - 1 : 0]                  m_rready,

    // Memory port
    output wire [ID_WIDTH - 1 : 0]              m_mem_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]            m_mem_axi_awaddr,
    output wire [ 7 : 0]                        m_mem_axi_awlen, // How many beats are in this transaction
    output wire [ 2 : 0]                        m_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addrStart by 1, 2 by 4 and so on
    output wire [ 1 : 0]                        m_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrappig
    output wire                                 m_mem_axi_awlock,
    output wire [ 3 : 0]                        m_mem_axi_awcache,
    output wire [ 2 : 0]                        m_mem_axi_awprot, 
    output wire                                 m_mem_axi_awvalid,
    input  wire                                 m_mem_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]            m_mem_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]            m_mem_axi_wstrb,
    output wire                                 m_mem_axi_wlast,
    output wire                                 m_mem_axi_wvalid,
    input  wire                                 m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]              m_mem_axi_bid,
    input  wire [ 1 : 0]                        m_mem_axi_bresp,
    input  wire                                 m_mem_axi_bvalid,
    output wire                                 m_mem_axi_bready,

    output wire [ID_WIDTH - 1 : 0]              m_mem_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]            m_mem_axi_araddr,
    output wire [ 7 : 0]                        m_mem_axi_arlen,
    output wire [ 2 : 0]                        m_mem_axi_arsize,
    output wire [ 1 : 0]                        m_mem_axi_arburst,
    output wire                                 m_mem_axi_arlock,
    output wire [ 3 : 0]                        m_mem_axi_arcache,
    output wire [ 2 : 0]                        m_mem_axi_arprot,
    output wire                                 m_mem_axi_arvalid,
    input  wire                                 m_mem_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]              m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]            m_mem_axi_rdata,
    input  wire [ 1 : 0]                        m_mem_axi_rresp,
    input  wire                                 m_mem_axi_rlast,
    input  wire                                 m_mem_axi_rvalid,
    output wire                                 m_mem_axi_rready
);
    genvar i;
    `define MAX(a, b) ((a) > (b) ? (a) : (b))
    
    reg  [$clog2(NPRT) - 1 : 0] portSelect;
    reg                         writeToMemory;
    reg                         transferActive;

    wire [ID_WIDTH - 1 : 0]     axid;
    wire [ADDR_WIDTH - 1 : 0]   axaddr;
    wire [ 7 : 0]               axlen;
    wire [ 2 : 0]               axsize;
    wire [ 1 : 0]               axburst;
    wire                        axlock;
    wire [ 3 : 0]               axcache;
    wire [ 2 : 0]               axprot; 
    wire                        axvalid;
    wire                        axready;

    wire [DATA_WIDTH - 1 : 0]   wdata;
    wire [STRB_WIDTH - 1 : 0]   wstrb;
    wire                        wlast;
    wire                        wvalid;
    wire                        wready;

    wire [DATA_WIDTH - 1 : 0]   rdata;
    wire [STRB_WIDTH - 1 : 0]   rstrb;
    wire                        rlast;
    wire                        rvalid;
    wire                        rready;

    reg                         avalid;
    wire                        aready;
    reg  [ADDR_WIDTH - 1 : 0]   aaddr;
    reg  [ADDR_WIDTH - 1 : 0]   abeats;

    wire                        s_wlastSignal;
    wire                        m_rlastSignal;
    wire                        mem_wlastSignal;
    reg                         s_wlastReg;

    AxisToAxiAdapter #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .STRB_WIDTH(STRB_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) axisToAxiAdapter (
        .aclk(aclk),
        .resetn(resetn),

        .s_avalid(avalid),
        .s_aaddr(aaddr),
        .s_abeats(abeats),
        .s_aready(aready),

        .enableAxiLastSignal(writeToMemory),

        .s_xdata(rdata),
        .s_xstrb(rstrb),
        .s_xlast(rlast),
        .s_xvalid(rvalid),
        .s_xready(rready),
    
        .m_xdata(wdata),
        .m_xstrb(wstrb),
        .m_xlast(wlast),
        .m_xvalid(wvalid),
        .m_xready(wready),

        .m_axid(axid),
        .m_axaddr(axaddr),
        .m_axlen(axlen),
        .m_axsize(axsize),
        .m_axburst(axburst),
        .m_axlock(axlock),
        .m_axcache(axcache),
        .m_axprot(axprot),
        .m_axvalid(axvalid),
        .m_axready(axready)
    );

    assign m_mem_axi_arid = axid; 
    assign m_mem_axi_araddr = axaddr; 
    assign m_mem_axi_arlen = axlen; 
    assign m_mem_axi_arsize = axsize; 
    assign m_mem_axi_arburst = axburst; 
    assign m_mem_axi_arlock = axlock; 
    assign m_mem_axi_arcache = axcache; 
    assign m_mem_axi_arprot = axprot; 
    assign m_mem_axi_arvalid = axvalid && !writeToMemory; 

    assign m_mem_axi_awid = axid; 
    assign m_mem_axi_awaddr = axaddr; 
    assign m_mem_axi_awlen = axlen; 
    assign m_mem_axi_awsize = axsize; 
    assign m_mem_axi_awburst = axburst; 
    assign m_mem_axi_awlock = axlock; 
    assign m_mem_axi_awcache = axcache; 
    assign m_mem_axi_awprot = axprot; 
    assign m_mem_axi_awvalid = axvalid && writeToMemory; 

    assign axready = (m_mem_axi_awready && writeToMemory) || (m_mem_axi_arready && !writeToMemory);

    assign m_mem_axi_wdata = wdata;
    assign m_mem_axi_wstrb = wstrb;
    assign m_mem_axi_wlast = wlast;
    assign m_mem_axi_wvalid = wvalid && transferActive && writeToMemory;
    assign m_rdata = { NPRT { wdata } };
    assign m_rlast = { NPRT { wlast } };
    generate
        for (i = 0; i < NPRT; i = i + 1)
        begin
            assign m_rvalid[i] = wvalid && transferActive && !writeToMemory && (portSelect == i);
        end
    endgenerate
    assign wready = ((m_mem_axi_wready && writeToMemory) || (m_rready[portSelect] && !writeToMemory)) && transferActive;

    assign rdata = (!writeToMemory) ? m_mem_axi_rdata : s_wdata[(portSelect * DATA_WIDTH) +: DATA_WIDTH];
    assign rstrb = (!writeToMemory) ? 0 : s_wstrb[(portSelect * STRB_WIDTH) +: STRB_WIDTH];
    assign rlast = (!writeToMemory) ? m_mem_axi_rlast : s_wlast[portSelect];
    assign rvalid = ((!writeToMemory) ? m_mem_axi_rvalid : s_wvalid[portSelect]) && transferActive;
    assign m_mem_axi_rready = rready && !writeToMemory && transferActive;
    generate
        for (i = 0; i < NPRT; i = i + 1)
        begin
            assign s_wready[i] = rready && transferActive && writeToMemory && (portSelect == i);
        end
    endgenerate

    assign m_mem_axi_bready = 1;

    assign s_wlastSignal = (s_wlast[portSelect] && s_wvalid[portSelect] && s_wready[portSelect]);
    assign m_rlastSignal = (m_rlast[portSelect] && m_rvalid[portSelect] && m_rready[portSelect]);
    assign mem_wlastSignal = (m_mem_axi_wlast && m_mem_axi_wvalid && m_mem_axi_wready);

    always @(posedge aclk)
    begin
        if (!resetn)
        begin : Reset
            portSelect <= 0;
            transferActive <= 0;
            s_aready <= { NPRT { 1'b0 } };
            avalid <= 0;
        end
        else
        begin
            if (aready)
            begin
                avalid <= 0;
            end
            if (|s_aready)
            begin
                s_aready <= { NPRT { 1'b0 } };
            end

            if (!transferActive)
            begin
                if (s_avalid[portSelect] && !avalid)
                begin
                    transferActive <= 1;
                    writeToMemory <= s_arnw[portSelect];
                    avalid <= s_avalid[portSelect];
                    aaddr <= s_aaddr[(portSelect * ADDR_WIDTH) +: ADDR_WIDTH];
                    abeats <= s_abeats[(portSelect * ADDR_WIDTH) +: ADDR_WIDTH];
                    s_aready[portSelect] <= 1;

                    s_wlastReg <= 0;
                end
                else
                begin if (NPRT > 1)
                    if ((portSelect + 1) < NPRT)
                    begin
                        portSelect <= portSelect + 1;
                    end
                    else
                    begin
                        portSelect <= 0;
                    end
                end
            end
            else
            begin
                // When writing to memory, and the last signal of the stream is set,
                // then we must check the last signal of the memory to make sure, that
                // all data is processed. Then we can disable the transfer and reset the muxing
                if (writeToMemory)
                begin
                    if (s_wlastSignal && !s_wlastReg)
                    begin
                        s_wlastReg <= 1;
                    end
                    if (s_wlastReg && mem_wlastSignal)
                    begin
                        transferActive <= 0;
                    end
                end

                // Opposite for the read port, we only must check if the last signal of the stream is set.
                // The memory transfer will already be finished when the last signal of the stream is set.
                if (!writeToMemory)
                begin
                    if (m_rlastSignal)
                    begin
                        transferActive <= 0;
                    end
                end
            end
        end
    end

endmodule