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

// This core is used to store streams into memory, load streams from memory and initialize memory.
// Padding calculation: padding = STREAM_WIDTH - 32.
// The engine uses for the commands st0_axis.
//
// NOP
// Beat 1:
// +---------+------+------+------------------------------------+
// | padding | 2'h0 | 2'h0 | 28'h don't care                    |
// +---------+------+------+------------------------------------+
// No operation. Can be used to resynchronize to a stream.
//
// Standard operation
// Beat 1:
// +---------+----------+---------+-----------------------------+
// | padding | 2'hx out | 2'hx in | 28'h stream length in bytes |
// +---------+----------+---------+-----------------------------+
// Beat 2:
// +---------+--------------------------------------------------+
// | padding | 32'h addr (only evaluated when mem_axi is used)  |
// +---------+--------------------------------------------------+
// Beat 3 .. n:
// +------------------------------------------------------------+
// | STREAM_WIDTH'h payload                                     |
// +------------------------------------------------------------+
//
// out and in values:
//
// Channel int = 2'h0
// Selects the internal channel. When used as in channel, it will store the last beat
// of the payload stream into a internal register.
// When used as out, it will load the the value from the internal register and outputs
// it on the selected channel. It can be used in combination with an other channel to 
// simulate a memset.
//
// Channel st0 = 2'h1
// Selects the st0_axis port. When used as in, then the s_st0_axis is used. When used as
// out then the m_st0_axis is used.
//
// Channel st1 = 2'h2
// Selects the st1_axis port. When used as in, then the s_st1_axis is used. When used as
// out then the m_st1_axis is used.
//
// Channel mem = 2'h3
// Selects the mem_axi port. When used as in, then it will read from the memory. When used
// as out, then it will write to the memory.
// Note: Used as in and out is a illegal configuration.

module FrameStreamingCore #(
    // Width of the axi interfaces
    parameter STREAM_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = (STREAM_WIDTH / 8),
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Auto load command. If this is a NOP (0), no autoload is active and the s_st0_axis is used for commands
    // For more specific documentation, refere the documentation of the s_st0_axis.
    parameter AUTO_CH_IN = 0, // beat 1
    parameter AUTO_CH_OUT = 0, // beat 1

    // How many bytes are loaded
    parameter AUTO_DATA_SIZE = 0, // beat 1

    // The adress used for the auto load
    parameter AUTO_ADDRESS = 32'h0 // beat 2
) (
    input  wire                         aclk,
    input  wire                         resetn,

    // Stream interface (Channel 0)
    output wire                         m_st0_axis_tvalid,
    input  wire                         m_st0_axis_tready,
    output wire                         m_st0_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]  m_st0_axis_tdata,

    input  wire                         s_st0_axis_tvalid,
    output wire                         s_st0_axis_tready,
    input  wire                         s_st0_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_st0_axis_tdata,

    // Stream interface (Channel 1)
    output wire                         m_st1_axis_tvalid,
    input  wire                         m_st1_axis_tready,
    output wire                         m_st1_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]  m_st1_axis_tdata,

    input  wire                         s_st1_axis_tvalid,
    output wire                         s_st1_axis_tready,
    input  wire                         s_st1_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_st1_axis_tdata,

    // Memory interface (Channel 2)
    output wire [ID_WIDTH - 1 : 0]      m_mem_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]    m_mem_axi_awaddr,
    output wire [ 7 : 0]                m_mem_axi_awlen, // How many beats are in this transaction
    output wire [ 2 : 0]                m_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addr by 1, 2 by 4 and so on
    output wire [ 1 : 0]                m_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrappig
    output wire                         m_mem_axi_awlock,
    output wire [ 3 : 0]                m_mem_axi_awcache,
    output wire [ 2 : 0]                m_mem_axi_awprot, 
    output wire                         m_mem_axi_awvalid,
    input  wire                         m_mem_axi_awready,

    output wire [STREAM_WIDTH - 1 : 0]  m_mem_axi_wdata,
    output reg  [STRB_WIDTH - 1 : 0]    m_mem_axi_wstrb,
    output wire                         m_mem_axi_wlast,
    output wire                         m_mem_axi_wvalid,
    input  wire                         m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]      m_mem_axi_bid,
    input  wire [ 1 : 0]                m_mem_axi_bresp,
    input  wire                         m_mem_axi_bvalid,
    output reg                          m_mem_axi_bready,

    output wire [ID_WIDTH - 1 : 0]      m_mem_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]    m_mem_axi_araddr,
    output wire [ 7 : 0]                m_mem_axi_arlen,
    output wire [ 2 : 0]                m_mem_axi_arsize,
    output wire [ 1 : 0]                m_mem_axi_arburst,
    output wire                         m_mem_axi_arlock,
    output wire [ 3 : 0]                m_mem_axi_arcache,
    output wire [ 2 : 0]                m_mem_axi_arprot,
    output wire                         m_mem_axi_arvalid,
    input  wire                         m_mem_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]      m_mem_axi_rid,
    input  wire [STREAM_WIDTH - 1 : 0]  m_mem_axi_rdata,
    input  wire [ 1 : 0]                m_mem_axi_rresp,
    input  wire                         m_mem_axi_rlast,
    input  wire                         m_mem_axi_rvalid,
    output wire                         m_mem_axi_rready
);
    localparam BYTES_PER_BEAT = STREAM_WIDTH / 8;
    localparam BYTES_PER_BEAT_LG2 = $clog2(BYTES_PER_BEAT);

    // Memory transfers have to be 128 bytes aligned.
    // 128 because: The zynq has 64 bit axi3 ports, which means one beat contains 8 bytes.
    // Max beats of an axi3 port are 16.
    // 128 / 8 = 16. So we will be axi3 compliant.
    localparam BEATS_PER_TRANSFER = 128 / BYTES_PER_BEAT;

    localparam OP_IMM_POS = 0;
    localparam OP_IMM_SIZE = 28;
    localparam MUX_CHANNEL_INT = 0;
    localparam MUX_CHANNEL_ST0 = 1;
    localparam MUX_CHANNEL_ST1 = 2;
    localparam MUX_CHANNEL_MEM = 3;
    localparam MUX_SELECT_IN_POS = 28;
    localparam MUX_SELECT_OUT_POS = 30;
    localparam MUX_SELECT_SIZE = 2;

    localparam IDLE = 0;
    localparam COMMAND = 1;
    localparam ADDR = 2;
    localparam LOAD = 3;
    localparam STREAM = 4;
    localparam STREAM_PAUSED = 5;

    localparam AUTO_LOAD = AUTO_CH_IN != 0 && AUTO_CH_OUT != 0;

    wire                         m_int_axis_tvalid;
    wire                         m_int_axis_tready;
    wire                         m_int_axis_tlast;
    wire [STREAM_WIDTH - 1 : 0]  m_int_axis_tdata;

    wire                         s_int_axis_tvalid;
    wire                         s_int_axis_tready;
    wire                         s_int_axis_tlast;
    wire [STREAM_WIDTH - 1 : 0]  s_int_axis_tdata;

    FrameStreamingCoreBusTerminate #(
        .STREAM_WIDTH(STREAM_WIDTH)
    ) intBusTerminate (
        .aclk(aclk),
        .resetn(resetn),

        .m_axis_tvalid(s_int_axis_tvalid),
        .m_axis_tready(s_int_axis_tready),
        .m_axis_tlast(s_int_axis_tlast),
        .m_axis_tdata(s_int_axis_tdata),
        
        .s_axis_tvalid(m_int_axis_tvalid),
        .s_axis_tready(m_int_axis_tready),
        .s_axis_tlast(m_int_axis_tlast),
        .s_axis_tdata(m_int_axis_tdata) 
    );

    reg  [ 5 : 0]               state;
    reg  [ 1 : 0]               muxIn;
    reg  [ 1 : 0]               muxOut;
    reg  [ 1 : 0]               tmpMuxIn;
    reg  [ 1 : 0]               tmpMuxOut;
    reg  [ADDR_WIDTH - 1 : 0]   counter;
    reg  [ADDR_WIDTH - 1 : 0]   dataSizeInBeats;
    
    reg  [ADDR_WIDTH - 1 : 0]   addrStart;
    reg                         enableWriteChannel;
    reg                         enableAddressChannel;
    reg                         startAddressGeneration;
    wire                        addressGenerationDone;
    reg                         readAddressGeneration;
    reg                         writeAddressGeneration;

    reg                         axisDestValid;
    reg                         axisDestReady;
    reg                         axisDestLast;
    reg                         axiDestLast;
    reg  [STREAM_WIDTH - 1 : 0] axisDestData;

    reg                         axisSourceValid;
    reg                         axisSourceReady;
    reg                         axisSourceLast;
    reg  [STREAM_WIDTH - 1 : 0] axisSourceData;

    reg                         axisSourceLastNext;
    reg                         axiSourceLastNext;
    reg  [STREAM_WIDTH - 1 : 0] axisSourceDataNext;

    wire [ID_WIDTH - 1 : 0]     mem_axi_axid;
    wire [ADDR_WIDTH - 1 : 0]   mem_axi_axaddr;
    wire [ 7 : 0]               mem_axi_axlen;
    wire [ 2 : 0]               mem_axi_axsize;
    wire [ 1 : 0]               mem_axi_axburst;
    wire                        mem_axi_axlock;
    wire [ 3 : 0]               mem_axi_axcache;
    wire [ 2 : 0]               mem_axi_axprot;
    wire                        mem_axi_axvalid;
    wire                        mem_axi_axready;

    LinearAddressGenerator #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .AxLEN_BEATS_PER_TRANSFER(BEATS_PER_TRANSFER - 1),
        .AxSIZE_BYTES_PER_BEAT(BYTES_PER_BEAT_LG2),
        .AxBURST(1) // Incremental
    ) readAddressGenerator (
        .aclk(aclk),
        .resetn(resetn),

        .start(startAddressGeneration),
        .done(addressGenerationDone),

        .startAddr(addrStart),
        .dataSizeInBeats(dataSizeInBeats),

        .axid(mem_axi_axid),
        .axaddr(mem_axi_axaddr),
        .axlen(mem_axi_axlen),
        .axsize(mem_axi_axsize),
        .axburst(mem_axi_axburst),
        .axlock(mem_axi_axlock),
        .axcache(mem_axi_axcache),
        .axprot(mem_axi_axprot), 
        .axvalid(mem_axi_axvalid),
        .axready(mem_axi_axready)
    );

    assign m_mem_axi_arid = mem_axi_axid;
    assign m_mem_axi_araddr = mem_axi_axaddr;
    assign m_mem_axi_arlen = mem_axi_axlen;
    assign m_mem_axi_arsize = mem_axi_axsize;
    assign m_mem_axi_arburst = mem_axi_axburst;
    assign m_mem_axi_arlock = mem_axi_axlock;
    assign m_mem_axi_arcache = mem_axi_axcache;
    assign m_mem_axi_arprot = mem_axi_axprot;
    assign m_mem_axi_arvalid = mem_axi_axvalid && readAddressGeneration;

    assign m_mem_axi_awid = mem_axi_axid;
    assign m_mem_axi_awaddr = mem_axi_axaddr;
    assign m_mem_axi_awlen = mem_axi_axlen;
    assign m_mem_axi_awsize = mem_axi_axsize;
    assign m_mem_axi_awburst = mem_axi_axburst;
    assign m_mem_axi_awlock = mem_axi_axlock;
    assign m_mem_axi_awcache = mem_axi_axcache;
    assign m_mem_axi_awprot = mem_axi_axprot;
    assign m_mem_axi_awvalid = mem_axi_axvalid && writeAddressGeneration;

    assign mem_axi_axready = (m_mem_axi_awready && writeAddressGeneration) 
                                || (m_mem_axi_arready && readAddressGeneration);

    initial 
    begin
        m_mem_axi_wstrb = { STRB_WIDTH { 1'b1 } };
        m_mem_axi_bready = 1;
    end

    // Master Channel Muxes output ports
    assign m_st0_axis_tvalid    = (muxOut == MUX_CHANNEL_ST0) ? axisDestValid : 0;
    assign m_st1_axis_tvalid    = (muxOut == MUX_CHANNEL_ST1) ? axisDestValid : 0;
    assign m_mem_axi_wvalid     = (muxOut == MUX_CHANNEL_MEM) ? axisDestValid : 0;
    assign m_int_axis_tvalid    = (muxOut == MUX_CHANNEL_INT) ? axisDestValid : 0;

    assign m_st0_axis_tlast = (muxOut == MUX_CHANNEL_ST0) ? axisDestLast : 0;
    assign m_st1_axis_tlast = (muxOut == MUX_CHANNEL_ST1) ? axisDestLast : 0;
    assign m_mem_axi_wlast  = (muxOut == MUX_CHANNEL_MEM) ? axiDestLast : 0;
    assign m_int_axis_tlast = (muxOut == MUX_CHANNEL_INT) ? axisDestLast : 0;
                            
    assign m_st0_axis_tdata = axisDestData;
    assign m_st1_axis_tdata = axisDestData;
    assign m_mem_axi_wdata  = axisDestData;
    assign m_int_axis_tdata = axisDestData;

    // Slave Channel Muxes input ports
    assign s_st0_axis_tready    = (muxIn == MUX_CHANNEL_ST0) ? axisSourceReady : 0;
    assign s_st1_axis_tready    = (muxIn == MUX_CHANNEL_ST1) ? axisSourceReady : 0;
    assign m_mem_axi_rready     = (muxIn == MUX_CHANNEL_MEM) ? axisSourceReady : 0;
    assign s_int_axis_tready    = (muxIn == MUX_CHANNEL_INT) ? axisSourceReady : 0;

    always @(posedge aclk)
    begin
        // Master Channel Muxes input ports
        axisDestReady   = (muxOut == MUX_CHANNEL_ST0) ? m_st0_axis_tready
                        : (muxOut == MUX_CHANNEL_ST1) ? m_st1_axis_tready
                        : (muxOut == MUX_CHANNEL_MEM) ? m_mem_axi_wready
                        : m_int_axis_tready;

        // Slave Channel Muxes output ports
        axisSourceValid = (muxIn == MUX_CHANNEL_ST0) ? s_st0_axis_tvalid 
                        : (muxIn == MUX_CHANNEL_ST1) ? s_st1_axis_tvalid
                        : (muxIn == MUX_CHANNEL_MEM) ? m_mem_axi_rvalid
                        : s_int_axis_tvalid;

        axisSourceLast  = (muxIn == MUX_CHANNEL_ST0) ? s_st0_axis_tlast 
                        : (muxIn == MUX_CHANNEL_ST1) ? s_st1_axis_tlast
                        : (muxIn == MUX_CHANNEL_MEM) ? m_mem_axi_rlast
                        : s_int_axis_tlast;

        axisSourceData  = (muxIn == MUX_CHANNEL_ST0) ? s_st0_axis_tdata
                        : (muxIn == MUX_CHANNEL_ST1) ? s_st1_axis_tdata
                        : (muxIn == MUX_CHANNEL_MEM) ? m_mem_axi_rdata
                        : s_int_axis_tdata;

        if (resetn == 0)
        begin
            axisDestValid <= 0;
            axisDestLast <= 0;
            axiDestLast <= 0;
            axisSourceReady <= 0;

            addrStart <= 0;

            enableAddressChannel <= 0;
            startAddressGeneration <= 0;
            readAddressGeneration <= 0;
            writeAddressGeneration <= 0;

            state <= IDLE;
            muxIn <= MUX_CHANNEL_ST0;
            muxOut <= MUX_CHANNEL_INT;
        end
        else 
        begin
            case (state)
            IDLE:
            begin
                axisDestLast <= 0;
                axiDestLast <= 0;
                axisDestValid <= 0;

                axisSourceReady <= 1;

                if (AUTO_LOAD)
                begin
                    muxIn <= MUX_CHANNEL_INT;
                    muxOut <= MUX_CHANNEL_INT;
                end
                else
                begin
                    muxIn <= MUX_CHANNEL_ST0;
                    muxOut <= MUX_CHANNEL_INT;
                end

                state <= COMMAND;
            end
            COMMAND:
            begin : CommandParsing
                reg  [OP_IMM_SIZE - 1 : 0]  counterConverted;
                if (axisSourceValid || AUTO_LOAD)
                begin
                    if (AUTO_LOAD)
                    begin
                        counterConverted = AUTO_DATA_SIZE >> BYTES_PER_BEAT_LG2;
                        tmpMuxIn <= AUTO_CH_IN;
                        tmpMuxOut <= AUTO_CH_OUT;
                    end 
                    else
                    begin
                        counterConverted = axisSourceData[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_PER_BEAT_LG2;
                        tmpMuxIn <= axisSourceData[MUX_SELECT_IN_POS +: MUX_SELECT_SIZE];
                        tmpMuxOut <= axisSourceData[MUX_SELECT_OUT_POS +: MUX_SELECT_SIZE];
                    end

                    if (ADDR_WIDTH < OP_IMM_SIZE)
                    begin
                        /* verilator lint_off SELRANGE */
                        counter <= counterConverted[0 +: ADDR_WIDTH];
                    end
                    else
                    begin : SetCounter
                        localparam REPLICA = (ADDR_WIDTH < OP_IMM_SIZE) ? 0 : (ADDR_WIDTH - OP_IMM_SIZE);
                        counter <= { { REPLICA { 1'b0 } }, counterConverted };
                    end
                    
                    if (axisSourceData[MUX_SELECT_IN_POS +: MUX_SELECT_SIZE] == 0 && axisSourceData[MUX_SELECT_OUT_POS +: MUX_SELECT_SIZE] == 0)
                    begin
                        // NOP
                        state <= COMMAND;
                    end
                    else
                    begin
                        state <= ADDR;
                    end
                end
            end
            ADDR:
            begin
                if (axisSourceValid || AUTO_LOAD)
                begin
                    if (counter > 0)
                    begin : Addr
                        reg  [ADDR_WIDTH - 1 : 0] addrTmp;
                        if (AUTO_LOAD)
                        begin
                            addrTmp = AUTO_ADDRESS[0 +: ADDR_WIDTH];
                        end
                        else
                        begin
                            addrTmp = axisSourceData[0 +: ADDR_WIDTH];
                        end
                        writeAddressGeneration <= (tmpMuxOut == MUX_CHANNEL_MEM);
                        readAddressGeneration <= (tmpMuxIn == MUX_CHANNEL_MEM);
                        enableAddressChannel <= (tmpMuxIn == MUX_CHANNEL_MEM) || (tmpMuxOut == MUX_CHANNEL_MEM);
                        startAddressGeneration <= (tmpMuxIn == MUX_CHANNEL_MEM) || (tmpMuxOut == MUX_CHANNEL_MEM);

                        addrStart <= addrTmp;
                        dataSizeInBeats <= counter;

                        muxIn <= tmpMuxIn;
                        muxOut <= tmpMuxOut;
                        state <= STREAM;
                    end
                    else 
                    begin
                        axisSourceReady <= 0;
                        state <= IDLE;
                    end
                end
            end
            STREAM:
            begin
                // Write the next beat when the destination is ready
                // if the destination is not ready, preload one entry till the output is valid
                if ((axisDestReady || !axisDestValid) && (counter != 0))
                begin
                    // Copy data from the source to the destination
                    axisDestValid <= axisSourceValid;
                    axisDestData <= axisSourceData;
                    axisDestLast <= counter == 1;
                    axiDestLast <= counter[0 +: $clog2(BEATS_PER_TRANSFER)] == 1; // streams to the axi interface must be partitioned in several sub transfers. This register will signal that.

                    // Enable the data source as long as we have to receive data.
                    // If the counter is 1 (which means last beat) but we didn't got valid data, keep the source active.
                    if ((counter != 1) || !axisSourceValid)
                    begin
                        axisSourceReady <= 1;
                    end
                    else
                    begin
                        axisSourceReady <= 0;
                    end

                    // Valid data signals what we have fetched one beat and decrement the counter
                    if (axisSourceValid)
                    begin
                        counter <= counter - 1;
                    end
                end
                else
                begin
                    // Wait till new data are received.
                    // If our destination was not ready, we have to save the data and to disable our data source
                    if ((counter > 0) && !axisDestReady && axisSourceValid)
                    begin
                        axisSourceReady <= 0;

                        axisSourceLastNext <= counter == 1;
                        axiSourceLastNext <= counter[0 +: $clog2(BEATS_PER_TRANSFER)] == 1;
                        axisSourceDataNext <= axisSourceData;

                        counter <= counter - 1;

                        // Pause stream till our destination is ready
                        state <= STREAM_PAUSED;
                    end
                end

                // If the counter is zero and we have transfered the last beat, then we can 
                // go back to idle
                if ((counter == 0) && axisDestReady)
                begin
                    axisDestValid <= 0;
                    axisSourceReady <= 0;
                    axisDestLast <= 0;

                    // Synchronize with the address channel
                    if (!enableAddressChannel)
                    begin
                        state <= IDLE;
                    end
                end
            end
            STREAM_PAUSED:
            begin
                if (axisDestReady)
                begin
                    // Check if the counter is greater than zero, if so, we still have to fetch data
                    if (counter > 0)
                    begin
                        axisSourceReady <= 1;
                    end

                    // Output the next data
                    axisDestData <= axisSourceDataNext;
                    axisDestLast <= axisSourceLastNext;
                    axiDestLast <= axiSourceLastNext;
                    
                    // Reenable the stream
                    state <= STREAM;
                end
            end
            endcase 
        end

        if (startAddressGeneration && !addressGenerationDone)
        begin
            startAddressGeneration <= 0;
        end
        if (!startAddressGeneration && enableAddressChannel && addressGenerationDone)
        begin
            readAddressGeneration <= 0;
            writeAddressGeneration <= 0;
            enableAddressChannel <= 0;
        end
    end
endmodule


module FrameStreamingCoreBusTerminate #(
    // Width of the axi interfaces
    parameter STREAM_WIDTH = 32
) (
    input  wire                         aclk,
    input  wire                         resetn,

    // Stream interface
    output wire                         m_axis_tvalid,
    input  wire                         m_axis_tready,
    output wire                         m_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]  m_axis_tdata,

    input  wire                         s_axis_tvalid,
    output wire                         s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);
    reg [STREAM_WIDTH - 1 : 0] intVal;
    always @(posedge aclk)
    begin
        if (s_axis_tvalid)
        begin
            intVal <= s_axis_tdata;
        end
    end
    assign s_axis_tready = 1;
    assign m_axis_tvalid = 1;
    assign m_axis_tlast = 0;
    assign m_axis_tdata = intVal;
endmodule