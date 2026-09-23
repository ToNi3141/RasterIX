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

module TextureReaderController #(
    parameter TEX_ADDR_WIDTH = 17,
    parameter TEXEL_WIDTH = 16,
    parameter ID_WIDTH = 4,
    localparam BYTE_ADDR_WIDTH = TEX_ADDR_WIDTH + 1,
    localparam ARSIZE = $clog2(TEXEL_WIDTH / 8)
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // Control
    input  wire                             invalidate,
    input  wire                             nearest,

    // Texture Read
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_texel_00,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_texel_01,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_texel_10,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    s_tr_texel_11,
    input  wire                             s_tr_valid,
    output reg                              s_tr_ready,

    // Interface to the texture texel cache
    output reg  [ 1 : 0]                    m_tr_texel_pos,
    output reg                              m_tr_cmd, // 1 = store and sample, 0 = store only
    output reg                              m_tr_valid,
    input  wire                             m_tr_ready,
    output reg  [BYTE_ADDR_WIDTH - 1 : 0]   m_tr_addr,

    // AXI read address metadata
    output wire [ID_WIDTH - 1 : 0]          m_arid,
    output wire [ 7 : 0]                    m_arlen,
    output wire [ 2 : 0]                    m_arsize,
    output wire [ 1 : 0]                    m_arburst,
    output wire                             m_arlock,
    output wire [ 3 : 0]                    m_arcache,
    output wire [ 2 : 0]                    m_arprot
);
    localparam INVALID_TEXEL_ADDR = { TEX_ADDR_WIDTH { 1'b1 } };
    localparam CMD_STORE_AND_SAMPLE = 1;
    localparam CMD_STORE_ONLY       = 0;

    assign m_arid = 0;
    assign m_arlen = 0;
    assign m_arsize = ARSIZE[0 +: 3];
    assign m_arburst = 2'b01;
    assign m_arlock = 0;
    assign m_arcache = 4'b0011;
    assign m_arprot = 3'b000;

    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel00;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel01;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel10;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel11;

    reg                          r_skid_valid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel00_skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel01_skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel10_skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] r_texel11_skid;

    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel00_next = (r_skid_valid) ? r_texel00_skid : s_tr_texel_00;
    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel01_next = (r_skid_valid) ? r_texel01_skid : s_tr_texel_01;
    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel10_next = (r_skid_valid) ? r_texel10_skid : s_tr_texel_10;
    wire [TEX_ADDR_WIDTH - 1 : 0] r_texel11_next = (r_skid_valid) ? r_texel11_skid : s_tr_texel_11;

    wire [3 : 0] texel_match = { 
        ((r_texel11_next == r_texel11) || nearest), 
        ((r_texel10_next == r_texel10) || nearest), 
        ((r_texel01_next == r_texel01) || nearest), 
        (r_texel00_next == r_texel00) 
    };

    always @(posedge aclk)
    begin
        if (!resetn || invalidate) 
        begin
            r_texel00 <= INVALID_TEXEL_ADDR;
            r_texel01 <= INVALID_TEXEL_ADDR;
            r_texel10 <= INVALID_TEXEL_ADDR;
            r_texel11 <= INVALID_TEXEL_ADDR;

            if (!resetn)
            begin
                s_tr_ready <= 1;
                r_skid_valid <= 0;
                m_tr_valid <= 0;
            end
        end 
        else 
        begin
            if ((s_tr_valid || r_skid_valid) && (!m_tr_valid || m_tr_ready))
            begin
                m_tr_valid <= 1;

                case (texel_match)
                    // All texels or at least one does not match
                    // One texel loading is always for free
                    4'b1111, 
                    4'b1110,
                    4'b1101,
                    4'b1011,
                    4'b0111:
                    begin
                        m_tr_cmd <= CMD_STORE_AND_SAMPLE;
                        s_tr_ready <= 1;
                        r_skid_valid <= 0;
                    end
                    // More than one does not match. Now we need a stall
                    default:
                    begin
                        m_tr_cmd <= CMD_STORE_ONLY;
                        s_tr_ready <= 0;
                        r_skid_valid <= 1;

                        if (!r_skid_valid)
                        begin
                            r_texel00_skid <= s_tr_texel_00;
                            r_texel01_skid <= s_tr_texel_01;
                            r_texel10_skid <= s_tr_texel_10;
                            r_texel11_skid <= s_tr_texel_11;
                        end
                    end
                endcase

                // Check which texels do not match and set the corresponding address
                // to load them.
                if (!texel_match[0])
                begin
                    m_tr_addr <= { r_texel00_next, 1'b0 };
                    m_tr_texel_pos <= 2'b00;
                    r_texel00 <= r_texel00_next;
                end
                else if (!texel_match[1])
                begin
                    m_tr_addr <= { r_texel01_next, 1'b0 };
                    m_tr_texel_pos <= 2'b01;
                    r_texel01 <= r_texel01_next;
                end
                else if (!texel_match[2])
                begin
                    m_tr_addr <= { r_texel10_next, 1'b0 };
                    m_tr_texel_pos <= 2'b10;
                    r_texel10 <= r_texel10_next;
                end
                else if (!texel_match[3])
                begin
                    m_tr_addr <= { r_texel11_next, 1'b0 };
                    m_tr_texel_pos <= 2'b11;
                    r_texel11 <= r_texel11_next;
                end
            end
            else if (m_tr_valid && !m_tr_ready && s_tr_valid && s_tr_ready)
            begin
                r_texel00_skid <= s_tr_texel_00;
                r_texel01_skid <= s_tr_texel_01;
                r_texel10_skid <= s_tr_texel_10;
                r_texel11_skid <= s_tr_texel_11;
                r_skid_valid <= 1;
                s_tr_ready <= 0;
            end
            else if (!s_tr_valid && !r_skid_valid && (!m_tr_valid || m_tr_ready))
            begin
                m_tr_valid <= 0;
                s_tr_ready <= 1;
                r_skid_valid <= 0;
            end
        end
    end
endmodule 