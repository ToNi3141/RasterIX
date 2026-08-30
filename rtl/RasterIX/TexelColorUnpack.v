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

`include "PixelUtil.vh"

module TexelColorUnpack #(
    parameter TEXEL_WIDTH = 16,

    localparam PIXEL_WIDTH = 32,
    localparam NUMBER_OF_SUB_PIXELS = 4,
    localparam SUB_PIXEL_WIDTH = PIXEL_WIDTH / NUMBER_OF_SUB_PIXELS,
    localparam SUB_PIXEL_WIDTH_INT = TEXEL_WIDTH / NUMBER_OF_SUB_PIXELS
)
(
    input  wire [ 3 : 0] confPixelFormat,
    input  wire [TEXEL_WIDTH - 1 : 0] texelInput,
    output wire [PIXEL_WIDTH - 1 : 0] texelOutput
);
`include "RegisterAndDescriptorDefines.vh"

    initial
    begin
        if (TEXEL_WIDTH != 16)
        begin
            $error("TEXEL_WIDTH must be 16");
            $finish;
        end
        if (PIXEL_WIDTH != 32)
        begin
            $error("PIXEL_WIDTH must be 32. Otherwise the conversions from the internal format to the external will not work.");
            $finish;
        end
        if (COLOR_A_POS != 0)
        begin
            $error("The COLOR_A_POS is expected to be at position 0. Otherwise the conversions from the internal format to the external will not work.");
            $finish;
        end
        if (RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE != 4)
        begin
            $error("RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE must be 4. If not, adapt confPixelFormat.");
            $finish;
        end
    end

    function [PIXEL_WIDTH - 1 : 0] RGBA5551TO8888;
        input [TEXEL_WIDTH - 1 : 0] pixels;
        begin
            RGBA5551TO8888[0 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { SUB_PIXEL_WIDTH { pixels[0] } };
            RGBA5551TO8888[1 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { pixels[1  +: 5], pixels[2  +: 3] };
            RGBA5551TO8888[2 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { pixels[6  +: 5], pixels[7  +: 3] };
            RGBA5551TO8888[3 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { pixels[11 +: 5], pixels[12 +: 3] };
        end
    endfunction

    function [PIXEL_WIDTH - 1 : 0] RGB565TO8888;
        input [TEXEL_WIDTH - 1 : 0] pixels;
        begin
            RGB565TO8888[0 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { SUB_PIXEL_WIDTH { 1'b1 } };
            RGB565TO8888[1 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { pixels[0  +: 5], pixels[2  +: 3] };
            RGB565TO8888[2 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { pixels[5  +: 6], pixels[9  +: 2] };
            RGB565TO8888[3 * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { pixels[11 +: 5], pixels[13 +: 3] };
        end
    endfunction

    `Expand(RGBA4444TO8888, SUB_PIXEL_WIDTH_INT, SUB_PIXEL_WIDTH, NUMBER_OF_SUB_PIXELS)

    assign texelOutput = (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGB565)  
                         ? RGB565TO8888(texelInput)
                         : (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGBA5551)  
                           ? RGBA5551TO8888(texelInput)
                           : RGBA4444TO8888(texelInput);

endmodule
