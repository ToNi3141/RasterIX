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

// Texture reader controller which manages the texture reader context and handles read requests.
// Pipelined: yes
// Depth: 1
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

    // Interface to the TextureReaderContext
    output reg  [ 1 : 0]                    m_trc_texel_pos,
    output reg                              m_trc_cmd, // 1 = store and sample, 0 = store only
    output reg                              m_trc_valid,
    input  wire                             m_trc_ready,
    output reg  [BYTE_ADDR_WIDTH - 1 : 0]   m_trc_addr
);
    localparam INVALID_TEXEL_ADDR = { TEX_ADDR_WIDTH { 1'b1 } };
    localparam CMD_STORE_AND_SAMPLE = 1;
    localparam CMD_STORE_ONLY       = 0;

    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel00;
    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel01;
    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel10;
    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel11;

    reg                          rSkidValid;
    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel00Skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel01Skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel10Skid;
    reg [TEX_ADDR_WIDTH - 1 : 0] rTexel11Skid;

    wire [TEX_ADDR_WIDTH - 1 : 0] rTexel00Next = (rSkidValid) ? rTexel00Skid : s_tr_texel_00;
    wire [TEX_ADDR_WIDTH - 1 : 0] rTexel01Next = (rSkidValid) ? rTexel01Skid : s_tr_texel_01;
    wire [TEX_ADDR_WIDTH - 1 : 0] rTexel10Next = (rSkidValid) ? rTexel10Skid : s_tr_texel_10;
    wire [TEX_ADDR_WIDTH - 1 : 0] rTexel11Next = (rSkidValid) ? rTexel11Skid : s_tr_texel_11;

    wire [3 : 0] texelMatch = {
        ((rTexel11Next == rTexel11) || nearest),
        ((rTexel10Next == rTexel10) || nearest),
        ((rTexel01Next == rTexel01) || nearest),
        (rTexel00Next == rTexel00)
    };

    always @(posedge aclk)
    begin
        if (!resetn || invalidate) 
        begin
            rTexel00 <= INVALID_TEXEL_ADDR;
            rTexel01 <= INVALID_TEXEL_ADDR;
            rTexel10 <= INVALID_TEXEL_ADDR;
            rTexel11 <= INVALID_TEXEL_ADDR;

            if (!resetn)
            begin
                s_tr_ready <= 1;
                rSkidValid <= 0;
                m_trc_valid <= 0;
            end
        end 
        else 
        begin
            if ((s_tr_valid || rSkidValid) && (!m_trc_valid || m_trc_ready))
            begin
                m_trc_valid <= 1;

                case (texelMatch)
                    // All texels or at least one does not match
                    // One texel loading is always for free
                    4'b1111, 
                    4'b1110,
                    4'b1101,
                    4'b1011,
                    4'b0111:
                    begin
                        m_trc_cmd <= CMD_STORE_AND_SAMPLE;
                        s_tr_ready <= 1;
                        rSkidValid <= 0;
                    end
                    // More than one does not match. Now we need a stall
                    default:
                    begin
                        m_trc_cmd <= CMD_STORE_ONLY;
                        s_tr_ready <= 0;
                        rSkidValid <= 1;

                        if (!rSkidValid)
                        begin
                            rTexel00Skid <= s_tr_texel_00;
                            rTexel01Skid <= s_tr_texel_01;
                            rTexel10Skid <= s_tr_texel_10;
                            rTexel11Skid <= s_tr_texel_11;
                        end
                    end
                endcase

                // Check which texels do not match and set the corresponding address
                // to load them.
                if (!texelMatch[0])
                begin
                    m_trc_addr <= { rTexel00Next, 1'b0 };
                    m_trc_texel_pos <= 2'b00;
                    rTexel00 <= rTexel00Next;
                end
                else if (!texelMatch[1])
                begin
                    m_trc_addr <= { rTexel01Next, 1'b0 };
                    m_trc_texel_pos <= 2'b01;
                    rTexel01 <= rTexel01Next;
                end
                else if (!texelMatch[2])
                begin
                    m_trc_addr <= { rTexel10Next, 1'b0 };
                    m_trc_texel_pos <= 2'b10;
                    rTexel10 <= rTexel10Next;
                end
                else if (!texelMatch[3])
                begin
                    m_trc_addr <= { rTexel11Next, 1'b0 };
                    m_trc_texel_pos <= 2'b11;
                    rTexel11 <= rTexel11Next;
                end
            end
            else if (m_trc_valid && !m_trc_ready && s_tr_valid && s_tr_ready)
            begin
                rTexel00Skid <= s_tr_texel_00;
                rTexel01Skid <= s_tr_texel_01;
                rTexel10Skid <= s_tr_texel_10;
                rTexel11Skid <= s_tr_texel_11;
                rSkidValid <= 1;
                s_tr_ready <= 0;
            end
            else if (!s_tr_valid && !rSkidValid && (!m_trc_valid || m_trc_ready))
            begin
                m_trc_valid <= 0;
                s_tr_ready <= 1;
                rSkidValid <= 0;
            end
        end
    end
endmodule 