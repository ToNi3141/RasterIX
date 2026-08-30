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

// Texture buffer which stores a whole texture. When reading a texel, the texture buffer
// reads a texel quad with the neighbored texels. Additionally it returns the sub pixel 
// coordinates which later can be used for texture filtering
// Pipelined: yes
// Depth: 2 cycle
module TextureBuffer #(

`define TMAX(a,b) ((a) > (b) ? (a) : (b))

    // Width of the write port
    parameter STREAM_WIDTH = 32,

    parameter MAX_TEXTURE_SIZE = 256,

    parameter ENABLE_LOD = 1,

    parameter TEXEL_WIDTH = 16,

    localparam MEM_WIDTH = `TMAX(32, STREAM_WIDTH),

    localparam MEM_WIDTH_HALF = MEM_WIDTH / 2,

    localparam SIZE_IN_BYTES_LG = $clog2(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE) + 1,
    localparam ADDR_WIDTH = SIZE_IN_BYTES_LG - $clog2(MEM_WIDTH / TEXEL_WIDTH),
    localparam ADDR_WIDTH_DIFF = SIZE_IN_BYTES_LG - ADDR_WIDTH,

    localparam TEX_ADDR_WIDTH = 17
`undef TMAX
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // Texture Read
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr00,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr01,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr10,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr11,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput00,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput01,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput10,
    output wire [TEXEL_WIDTH - 1 : 0]       texelOutput11,

    // Texture Write
    input  wire                             s_axis_tvalid,
    input  wire                             s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]      s_axis_tdata
);
    initial 
    begin
        if (STREAM_WIDTH < TEXEL_WIDTH)
        begin
            $error("STREAM_WIDTH must be at least TEXEL_WIDTH bits");
            $finish;
        end

        if (!((MAX_TEXTURE_SIZE == 256) 
            || (MAX_TEXTURE_SIZE == 128)
            || (MAX_TEXTURE_SIZE == 64)
            || (MAX_TEXTURE_SIZE == 32)))
        begin
            $error("MAX_TEXTURE_SIZE allowed values: 256, 128, 64, 32. Actual: %d", MAX_TEXTURE_SIZE);
            $finish;
        end
    end

    reg  [ADDR_WIDTH - 1 : 0]           memWriteAddr = 0;
    wire                                memWriteEven;
    wire                                memWriteOdd;

    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding00;
    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding01;
    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding10;
    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding11;

    wire [ADDR_WIDTH - 1 : 0]           memReadAddrEven0;
    wire [ADDR_WIDTH - 1 : 0]           memReadAddrOdd0;
    wire [ADDR_WIDTH - 1 : 0]           memReadAddrEven1;
    wire [ADDR_WIDTH - 1 : 0]           memReadAddrOdd1;

    wire [MEM_WIDTH_HALF - 1 : 0]       memReadDataEven0;
    wire [MEM_WIDTH_HALF - 1 : 0]       memReadDataOdd0;
    wire [MEM_WIDTH_HALF - 1 : 0]       memReadDataEven1;
    wire [MEM_WIDTH_HALF - 1 : 0]       memReadDataOdd1;

    wire [MEM_WIDTH_HALF - 1 : 0]       tdataEvenS;
    wire [MEM_WIDTH_HALF - 1 : 0]       tdataOddS;

    wire [TEXEL_WIDTH - 1 : 0]          texelSelect00;
    wire [TEXEL_WIDTH - 1 : 0]          texelSelect01;
    wire [TEXEL_WIDTH - 1 : 0]          texelSelect10;
    wire [TEXEL_WIDTH - 1 : 0]          texelSelect11;

    MipmapOptimizedRam #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .MEM_WIDTH(MEM_WIDTH_HALF),
        .WRITE_STROBE_WIDTH(TEXEL_WIDTH),
        .MEMORY_PRIMITIVE("block"),
        .ENABLE_LOD_OPTIMIZATION(ENABLE_LOD)
    ) texCacheEvenS (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataEvenS),
        .write(s_axis_tvalid & memWriteEven),
        .writeAddr((s_axis_tvalid) ? memWriteAddr : memReadAddrEven1),
        .writeMask({ (MEM_WIDTH_HALF / TEXEL_WIDTH) { 1'b1 } }),
        .writeDataOut(memReadDataEven1),

        .readData(memReadDataEven0),
        .readAddr(memReadAddrEven0)
    );

    MipmapOptimizedRam #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .MEM_WIDTH(MEM_WIDTH_HALF),
        .WRITE_STROBE_WIDTH(TEXEL_WIDTH),
        .MEMORY_PRIMITIVE("block"),
        .ENABLE_LOD_OPTIMIZATION(ENABLE_LOD)
    ) texCacheOddS (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataOddS),
        .write(s_axis_tvalid & memWriteOdd),
        .writeAddr((s_axis_tvalid) ? memWriteAddr : memReadAddrOdd1),
        .writeMask({ (MEM_WIDTH_HALF / TEXEL_WIDTH) { 1'b1 } }),
        .writeDataOut(memReadDataOdd1),

        .readData(memReadDataOdd0),
        .readAddr(memReadAddrOdd0)
    );
    
    //////////////////////////////////////////////
    //  Build RAM addresses
    //////////////////////////////////////////////

    // Muxing of the RAM access to query the texels from the even and odd RAMs.
    // The odd RAM only contains the texels of the odd s coordinates. The even only the texels of an even s
    assign memReadAddrEven0 = (texelAddr00[0]) ? texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd0  = (texelAddr00[0]) ? texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrEven1 = (texelAddr10[0]) ? texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd1  = (texelAddr10[0]) ? texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH];

    always @(posedge aclk)
    begin
        // Save decoding information to select the right word from the memory read vector
        texelAddrForDecoding00 <= texelAddr00;
        texelAddrForDecoding01 <= texelAddr01;
        texelAddrForDecoding10 <= texelAddr10;
        texelAddrForDecoding11 <= texelAddr11;
    end

    //////////////////////////////////////////////
    // Demux RAM address and expand pixels
    //////////////////////////////////////////////
    // Demux the RAM access and access the texels in the read vector
    generate
        if (MEM_WIDTH <= 32)
        begin
            assign texelSelect00 = (texelAddrForDecoding00[0])  ? memReadDataOdd0
                                                                : memReadDataEven0;

            assign texelSelect01 = (texelAddrForDecoding01[0])  ? memReadDataOdd0
                                                                : memReadDataEven0;

            assign texelSelect10 = (texelAddrForDecoding10[0])  ? memReadDataOdd1
                                                                : memReadDataEven1;

            assign texelSelect11 = (texelAddrForDecoding11[0])  ? memReadDataOdd1
                                                                : memReadDataEven1;
        end
        else 
        begin
            // Bit zero is used to check, if we have to select the RAM with the even or uneven pixel addresses (see also the multiplexing of the memReadAddr*)
            // Since bit zero is already used from the ADDR_WIDTH_DIFF to select the even or uneven ram, we can use the rest of the
            // bits to select the pixel from the vector. Therefore we start at position 1 and select one bit less from ADDR_WIDTH_DIFF to keep the selection in bound.
            assign texelSelect00 = (texelAddrForDecoding00[0])  ? memReadDataOdd0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH]
                                                                : memReadDataEven0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH];

            assign texelSelect01 = (texelAddrForDecoding01[0])  ? memReadDataOdd0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH]
                                                                : memReadDataEven0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH];

            assign texelSelect10 = (texelAddrForDecoding10[0])  ? memReadDataOdd1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH]
                                                                : memReadDataEven1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH];

            assign texelSelect11 = (texelAddrForDecoding11[0])  ? memReadDataOdd1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH]
                                                                : memReadDataEven1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * TEXEL_WIDTH +: TEXEL_WIDTH];
        end
    endgenerate

    assign texelOutput00 = texelSelect00;
    assign texelOutput01 = texelSelect01;
    assign texelOutput10 = texelSelect10;
    assign texelOutput11 = texelSelect11;

    //////////////////////////////////////////////
    // AXIS Interface
    //////////////////////////////////////////////
    generate 
    if (STREAM_WIDTH == TEXEL_WIDTH)
    begin
        reg memWriteEvenNotOdd;
        always @(posedge aclk)
        begin
            if (!resetn)
            begin
                memWriteAddr <= 0;
                memWriteEvenNotOdd <= 1;
            end
            else
            begin
                if (s_axis_tvalid)
                begin
                    if (s_axis_tlast)
                    begin
                        memWriteAddr <= 0;
                        memWriteEvenNotOdd <= 1;
                    end
                    else
                    begin
                        if (memWriteEvenNotOdd)
                        begin
                            memWriteEvenNotOdd <= 0;
                        end
                        else
                        begin
                            memWriteAddr <= memWriteAddr + 1;
                            memWriteEvenNotOdd <= 1;
                        end
                    end
                end
            end
        end

        assign memWriteEven = memWriteEvenNotOdd;
        assign memWriteOdd = !memWriteEvenNotOdd;
        assign tdataEvenS = s_axis_tdata;
        assign tdataOddS = s_axis_tdata;
    end
    else
    begin
        always @(posedge aclk)
        begin
            if (!resetn)
            begin
                memWriteAddr <= 0;
            end
            else
            begin
                if (s_axis_tvalid)
                begin
                    if (s_axis_tlast)
                    begin
                        memWriteAddr <= 0;
                    end
                    else
                    begin
                        memWriteAddr <= memWriteAddr + 1;
                    end
                end
            end
        end

        assign memWriteEven = 1;
        assign memWriteOdd = 1;

        // Stride the incoming data. All even pixel on the X coordinate have to go to the even RAM
        // All uneven pixel on the X coordinate have to go in the odd RAM.
        genvar i;

        // Stride for the even RAM
        for (i = 0; i < MEM_WIDTH_HALF / TEXEL_WIDTH; i = i + 1)
        begin
            localparam ii = i * (TEXEL_WIDTH * 2);
            localparam jj = i * TEXEL_WIDTH;
            assign tdataEvenS[jj +: TEXEL_WIDTH] = s_axis_tdata[ii +: TEXEL_WIDTH];
        end

        // Stride for the uneven RAM
        for (i = 0; i < MEM_WIDTH_HALF / TEXEL_WIDTH; i = i + 1)
        begin
            localparam ii = (i * (TEXEL_WIDTH * 2)) + TEXEL_WIDTH;
            localparam jj = i * TEXEL_WIDTH;
            assign tdataOddS[jj +: TEXEL_WIDTH] = s_axis_tdata[ii +: TEXEL_WIDTH];
        end
    end
    endgenerate
endmodule 