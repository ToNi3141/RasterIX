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

// Serial and FT245x interface to AXIS
module MainInterface 
#(
    parameter FIFO_SIZE = 4096,
    parameter FIFO_TRESHOLD = FIFO_SIZE / 4
)
(
    input  wire             aclk,
    input  wire             resetn,

    // Serial interface
    input  wire             serial_reset,

    input  wire             serial_mosi,
    output wire             serial_miso,
    input  wire             serial_sck,
    input  wire             serial_ncs,
    output wire             serial_cts,

    // FT245 interface
    output wire [ 1 : 0]    set_vadj,
    output wire             vadj_en,
    
    input  wire             fmc_reset,

    inout  wire             fmc_wkup,
    input  wire             fmc_clk,
    input  wire             fmc_txen,
    input  wire             fmc_rxfn,
    output wire             fmc_rdn,
    output wire             fmc_oen,
    inout  wire [ 1 : 0]    fmc_be,
    output wire             fmc_siwun,
    output wire             fmc_wrn,
    inout  wire [15 : 0]    fmc_data,

    // AXIS interface
    output wire             m_axis_tvalid,
    input  wire             m_axis_tready,
    output wire             m_axis_tlast,
    output wire [31 : 0]    m_axis_tdata,

    input  wire             s_axis_tvalid,
    output wire             s_axis_tready,
    input  wire             s_axis_tlast,
    input  wire [31 : 0]    s_axis_tdata,

    // System reset
    output wire             concatinated_reset
);
    assign concatinated_reset = !(serial_reset && fmc_reset);

    wire            fmc_axis_tvalid;
    wire            fmc_axis_tready;
    wire            fmc_axis_tlast;
    wire [15 : 0]   fmc_axis_tdata;

    wire            fmc32_axis_tvalid;
    wire            fmc32_axis_tready;
    wire            fmc32_axis_tlast;
    wire [31 : 0]   fmc32_axis_tdata;

    wire            serial_axis_tvalid;
    wire            serial_axis_tready;
    wire            serial_axis_tlast;
    wire [31 : 0]   serial_axis_tdata;

    wire            mux_axis_tvalid;
    wire            mux_axis_tready;
    wire            mux_axis_tlast;
    wire [31 : 0]   mux_axis_tdata;

    assign s_axis_tready = 1'b1;

    FT245X2AXIS ft245x2axisInst (
        .aclk(aclk),
        .aresetn(resetn),
        .set_vadj(set_vadj),
        .vadj_en(vadj_en),
        .fmc_wkup(fmc_wkup),
        .fmc_clk(fmc_clk),
        .fmc_txen(fmc_txen),
        .fmc_rxfn(fmc_rxfn),
        .fmc_rdn(fmc_rdn),
        .fmc_oen(fmc_oen),
        .fmc_be(fmc_be),
        .fmc_siwun(fmc_siwun),
        .fmc_wrn(fmc_wrn),
        .fmc_data(fmc_data),
        .m_axis_tvalid(fmc_axis_tvalid),
        .m_axis_tready(fmc_axis_tready),
        .m_axis_tlast(fmc_axis_tlast),
        .m_axis_tdata(fmc_axis_tdata)
    );

    Serial2AXIS #(
        .FIFO_SIZE(FIFO_SIZE),
        .FIFO_TRESHOLD(FIFO_TRESHOLD)
    ) serial2axisInst (
        .aclk(aclk),
        .resetn(resetn), 
        .serial_mosi(serial_mosi),
        .serial_miso(serial_miso),
        .serial_sck(serial_sck),
        .serial_ncs(serial_ncs),
        .serial_cts(serial_cts),
        .m_axis_tvalid(serial_axis_tvalid),
        .m_axis_tready(serial_axis_tready),
        .m_axis_tlast(serial_axis_tlast),
        .m_axis_tdata(serial_axis_tdata),
        .s_axis_tvalid(0),
        .s_axis_tready(),
        .s_axis_tlast(0),
        .s_axis_tdata(0)
    );

    axis_adapter #(
        .S_DATA_WIDTH(16),
        .M_DATA_WIDTH(32),
        .S_KEEP_ENABLE(1),
        .M_KEEP_ENABLE(1),
        .ID_ENABLE(0),
        .DEST_ENABLE(0),
        .USER_ENABLE(0)
    ) axisAdapterInst (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tkeep(~0),
        .s_axis_tdata(fmc_axis_tdata),
        .s_axis_tvalid(fmc_axis_tvalid),
        .s_axis_tready(fmc_axis_tready),
        .s_axis_tlast(fmc_axis_tlast),

        .m_axis_tdata(fmc32_axis_tdata),
        .m_axis_tvalid(fmc32_axis_tvalid),
        .m_axis_tready(fmc32_axis_tready),
        .m_axis_tlast(fmc32_axis_tlast)
    );

    axis_mux #(
        .S_COUNT(2),
        .DATA_WIDTH(32),
        .KEEP_ENABLE(0),
        .ID_ENABLE(0),
        .DEST_ENABLE(0),
        .USER_ENABLE(0)
    ) axisMuxInst (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata({ fmc32_axis_tdata, serial_axis_tdata }),
        .s_axis_tvalid({ fmc32_axis_tvalid, serial_axis_tvalid }),
        .s_axis_tready({ fmc32_axis_tready, serial_axis_tready }),
        .s_axis_tlast({ fmc32_axis_tlast, serial_axis_tlast }),

        .m_axis_tdata(m_axis_tdata),
        .m_axis_tvalid(m_axis_tvalid),
        .m_axis_tready(m_axis_tready),
        .m_axis_tlast(m_axis_tlast),

        .enable(fmc32_axis_tvalid || serial_axis_tvalid),
        .select({ serial_axis_tvalid ? 0 : 1 })
    );
endmodule

