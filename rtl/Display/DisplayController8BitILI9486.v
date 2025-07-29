// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

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

`timescale 1ns / 1ps

// This module initializes and feeds an ultra cheap ILI9486 display with a resolution of 480x320x16. 
// It uses the 8 bit 8080 interface of the display. As addition, you can configure the landscape or 
// portait mode of the display. It has an 16 bit axi stream slave interface which is used as data 
// source for the display.
module DisplayController8BitILI9486 #(
    parameter CLOCK_DIV = 0, // Divides clk to slowdown the wr cycles. 0 means wr is "clocked" with clk / 2, 1 equals clk / 4, 2 equals clk / 5 ...
    parameter SKIP_INIT = 0, // Skips the initialization sequence and directly starts serializting from the axi stream interface
    
    // Converts an RGB(A) stream to the display format
    // If STREAM_COLORMODE_RGBA == 1, then the expetect format is: | 4 bit R | 4 bit G | 4 bit B | 4 bit A |
    // If STREAM_COLORMODE_RGBA == 0, then the expetect format is: | 5 bit R | 6 bit G | 5 bit B |
    parameter STREAM_COLORMODE_RGBA = 0,

    parameter LANDSCAPE_CONFIG = 0
) (
    input  wire         resetn,
    input  wire         aclk,

    // Data out
    output reg  [7 : 0] data,
    output wire         rd, 
    output reg          wr,
    output reg          cs,
    output reg          dc,
    output wire         rst,

    // AXI Stream slave interface
    input  wire         s_axis_tvalid,
    output wire         s_axis_tready,
    input  wire         s_axis_tlast,
    input  wire [15 : 0] s_axis_tdata
);
    localparam INIT_MEM_SIZE = 102;

    localparam COLOR_R_POS = 12;
    localparam COLOR_G_POS = 8;
    localparam COLOR_B_POS = 4;
    localparam COLOR_A_POS = 0;
    localparam COLOR_SUB_PIXEL_WIDTH = 4;
    localparam DISP_SIZE = 480 * 320; // Display size in pixel

    assign rst = resetn;
    assign rd = 1;
    reg [8 : 0] initMem [0 : INIT_MEM_SIZE - 1]; // dc[8], data[7:0]
    initial 
    begin
        // ILI9468 initialization sequence
        initMem[  0] = {1'b0, 8'hF9};  
        initMem[  1] = {1'b1, 8'h00}; 
        initMem[  2] = {1'b1, 8'h08};
        initMem[  3] = {1'b0, 8'hC0};  
        initMem[  4] = {1'b1, 8'h19}; 
        initMem[  5] = {1'b1, 8'h1A};  
        initMem[  6] = {1'b0, 8'hC1};  
        initMem[  7] = {1'b1, 8'h45}; 
        initMem[  8] = {1'b1, 8'h00}; 
        initMem[  9] = {1'b0, 8'hC2};  
        initMem[ 10] = {1'b1, 8'h33}; 
        initMem[ 11] = {1'b0, 8'hC5};  
        initMem[ 12] = {1'b1, 8'h00}; 
        initMem[ 13] = {1'b1, 8'h28}; 
        initMem[ 14] = {1'b0, 8'hB1};  
        initMem[ 15] = {1'b1, 8'h90}; 
        initMem[ 16] = {1'b1, 8'h11}; 
        initMem[ 17] = {1'b0, 8'hB4};  
        initMem[ 18] = {1'b1, 8'h01}; 
        initMem[ 19] = {1'b0, 8'hB6};  
        initMem[ 20] = {1'b1, 8'h00}; 
        initMem[ 21] = {1'b1, 8'h42};
        initMem[ 22] = {1'b1, 8'h3B};
        initMem[ 23] = {1'b0, 8'hB7};  
        initMem[ 24] = {1'b1, 8'h07}; 
        initMem[ 25] = {1'b0, 8'hE0}; 
        initMem[ 26] = {1'b1, 8'h1F}; 
        initMem[ 27] = {1'b1, 8'h25}; 
        initMem[ 28] = {1'b1, 8'h22}; 
        initMem[ 29] = {1'b1, 8'h0B}; 
        initMem[ 30] = {1'b1, 8'h06}; 
        initMem[ 31] = {1'b1, 8'h0A}; 
        initMem[ 32] = {1'b1, 8'h4E}; 
        initMem[ 33] = {1'b1, 8'hC6}; 
        initMem[ 34] = {1'b1, 8'h39}; 
        initMem[ 35] = {1'b1, 8'h00}; 
        initMem[ 36] = {1'b1, 8'h00}; 
        initMem[ 37] = {1'b1, 8'h00}; 
        initMem[ 38] = {1'b1, 8'h00}; 
        initMem[ 39] = {1'b1, 8'h00}; 
        initMem[ 40] = {1'b1, 8'h00}; 
        initMem[ 41] = {1'b0, 8'hE1}; 
        initMem[ 42] = {1'b1, 8'h1F}; 
        initMem[ 43] = {1'b1, 8'h3F}; 
        initMem[ 44] = {1'b1, 8'h3F}; 
        initMem[ 45] = {1'b1, 8'h0F}; 
        initMem[ 46] = {1'b1, 8'h1F}; 
        initMem[ 47] = {1'b1, 8'h0F}; 
        initMem[ 48] = {1'b1, 8'h46}; 
        initMem[ 49] = {1'b1, 8'h49}; 
        initMem[ 50] = {1'b1, 8'h31}; 
        initMem[ 51] = {1'b1, 8'h05}; 
        initMem[ 52] = {1'b1, 8'h09}; 
        initMem[ 53] = {1'b1, 8'h03}; 
        initMem[ 54] = {1'b1, 8'h1C}; 
        initMem[ 55] = {1'b1, 8'h1A}; 
        initMem[ 56] = {1'b1, 8'h00};  
        initMem[ 57] = {1'b0, 8'hF1};   
        initMem[ 58] = {1'b1, 8'h36};   
        initMem[ 59] = {1'b1, 8'h04};
        initMem[ 60] = {1'b1, 8'h00};
        initMem[ 61] = {1'b1, 8'h3C};
        initMem[ 62] = {1'b1, 8'h0F};
        initMem[ 63] = {1'b1, 8'h0F};
        initMem[ 64] = {1'b1, 8'hA4};
        initMem[ 65] = {1'b1, 8'h02};
        initMem[ 66] = {1'b0, 8'hF2};   
        initMem[ 67] = {1'b1, 8'h18};   
        initMem[ 68] = {1'b1, 8'hA3};
        initMem[ 69] = {1'b1, 8'h12};
        initMem[ 70] = {1'b1, 8'h02};
        initMem[ 71] = {1'b1, 8'h32};
        initMem[ 72] = {1'b1, 8'h12};
        initMem[ 73] = {1'b1, 8'hFF};
        initMem[ 74] = {1'b1, 8'h32};
        initMem[ 75] = {1'b1, 8'h00};
        initMem[ 76] = {1'b0, 8'hF4};   
        initMem[ 77] = {1'b1, 8'h40};   
        initMem[ 78] = {1'b1, 8'h00};
        initMem[ 79] = {1'b1, 8'h08};
        initMem[ 80] = {1'b1, 8'h91};
        initMem[ 81] = {1'b1, 8'h04};     
        initMem[ 82] = {1'b0, 8'hF8};   
        initMem[ 83] = {1'b1, 8'h21};   
        initMem[ 84] = {1'b1, 8'h04};
        if (LANDSCAPE_CONFIG)
        begin
            initMem[ 85] = {1'b0, 8'h36};  
            initMem[ 86] = {1'b1, 8'hB8}; // Vertical
        end 
        else
        begin
            initMem[ 85] = {1'b0, 8'h36};  
            initMem[ 86] = {1'b1, 8'hC8}; // Horizontal
        end
        initMem[ 87] = {1'b0, 8'h3A};   
        initMem[ 88] = {1'b1, 8'h55}; 
        initMem[ 89] = {1'b0, 8'h11}; //Exit Sleep 			
        initMem[ 90] = {1'b0, 8'h29}; //Display on 
        if (LANDSCAPE_CONFIG)
        begin
            initMem[ 91] = {1'b0, 8'h2a};
            initMem[ 92] = {1'b1, 8'h00}; // 0
            initMem[ 93] = {1'b1, 8'h00};
            initMem[ 94] = {1'b1, 8'h01}; // 479
            initMem[ 95] = {1'b1, 8'hDF};
            initMem[ 96] = {1'b0, 8'h2b};
            initMem[ 97] = {1'b1, 8'h00}; // 0
            initMem[ 98] = {1'b1, 8'h00};
            initMem[ 99] = {1'b1, 8'h01}; // 319
            initMem[100] = {1'b1, 8'h3F};
        end
        else
        begin
            initMem[ 91] = {1'b0, 8'h2a};
            initMem[ 92] = {1'b1, 8'h00}; // 0
            initMem[ 93] = {1'b1, 8'h00};
            initMem[ 94] = {1'b1, 8'h01}; // 319
            initMem[ 95] = {1'b1, 8'h3F};
            initMem[ 96] = {1'b0, 8'h2b};
            initMem[ 97] = {1'b1, 8'h00}; // 0
            initMem[ 98] = {1'b1, 8'h00};
            initMem[ 99] = {1'b1, 8'h01}; // 479
            initMem[100] = {1'b1, 8'hDF};
        end
        initMem[101] = {1'b0, 8'h2c};   
    end
    reg [$clog2(INIT_MEM_SIZE) - 1 : 0] counterMemInit;
    reg [$clog2(DISP_SIZE) - 1 : 0]     counter;
    reg [5 : 0]     state;

    reg [15 : 0]    pixel;
    reg             pixelValid;

    reg [5 : 0]     clockDiv;

    assign s_axis_tready = !pixelValid;

    localparam INIT = 0;
    localparam STREAM0 = 1;
    localparam STREAM1 = 2;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            data <= 0;
            wr <= 1;
            cs <= 1;
            dc <= 1;

            counterMemInit <= 0;
            counter <= 0;
            
            if (SKIP_INIT)
            begin
                pixelValid <= 0;
                state <= STREAM0;
            end
            else 
            begin
                pixelValid <= 1;
                state <= INIT;
            end
        end
        else 
        begin
            cs <= 0;
            if (!pixelValid)
            begin
                if (s_axis_tvalid)
                begin
                    if (STREAM_COLORMODE_RGBA)
                    begin
                        pixel <= {s_axis_tdata[COLOR_R_POS +: COLOR_SUB_PIXEL_WIDTH], 1'b0, 
                                  s_axis_tdata[COLOR_G_POS +: COLOR_SUB_PIXEL_WIDTH], 2'b00, 
                                  s_axis_tdata[COLOR_B_POS +: COLOR_SUB_PIXEL_WIDTH], 1'b0};
                    end
                    else
                    begin
                        pixel <= s_axis_tdata;
                    end
                      
                    pixelValid <= counter < DISP_SIZE;

                    if (s_axis_tlast)
                    begin
                        counter <= 0;
                    end
                    else
                    begin
                        counter <= counter + 1;
                    end
                end
            end

            if ((clockDiv == CLOCK_DIV) || (CLOCK_DIV == 0))
            begin
                clockDiv <= 0;
                case (state)
                INIT:
                begin
                    if (wr)
                    begin
                        dc <= initMem[counterMemInit][8];
                        data <= initMem[counterMemInit][0 +: 8];
                        wr <= 0;
                        counterMemInit <= counterMemInit + 1;
                    end
                    else 
                    begin
                        wr <= 1;
                        if (counterMemInit == INIT_MEM_SIZE) 
                        begin
                            pixelValid <= 0;
                            state <= STREAM0;
                        end
                    end
                end
                STREAM0:
                begin
                    if (wr)
                    begin
                        if (pixelValid)
                        begin
                            dc <= 1;
                            wr <= 0;
                            data <= pixel[8 +: 8];
                        end
                    end
                    else 
                    begin
                        wr <= 1;
                        state <= STREAM1;
                    end
                end
                STREAM1:
                begin
                    if (wr)
                    begin
                        wr <= 0;
                        data <= pixel[0 +: 8];
                    end
                    else 
                    begin
                        wr <= 1;
                        pixelValid <= 0;
                        state <= STREAM0;
                    end
                end
                endcase  
            end
            else 
            begin
                clockDiv <= clockDiv + 1;    
            end
        end
    end
endmodule
