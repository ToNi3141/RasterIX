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


// Verilog wrapper around the VHDL 2008 hyperram entity with integrated PHY
// and AXI4-to-Avalon bridge.
// Use this as the IP top file in Vivado block designs to avoid
// the "VHDL 2008 not allowed as top file" limitation.
// Includes:
//   - AXI4 slave interface (connects to block design interconnect)
//   - AxiToAvalonBridge (AXI4 → Avalon MM)
//   - hyperram controller (Avalon MM → HyperRAM signals)
//   - HyperRamPhy (IOBUF tri-state buffers)

module HyperRamWrapper #(
    parameter G_ERRATA_ISSI_D_FIX = 1,
    parameter ADDR_WIDTH = 32,
    parameter ID_WIDTH   = 8
) (
    input  wire                      aclk,
    input  wire                      aclk90degree,
    input  wire                      delay_refclk_i,
    input  wire                      resetn,

    // -------------------------------------------------------------------
    // AXI4 Slave Interface
    // -------------------------------------------------------------------
    // Write Address Channel
    input  wire [ID_WIDTH - 1 : 0]   s_axi_awid,
    input  wire [ADDR_WIDTH - 1 : 0] s_axi_awaddr,
    input  wire [ 7 : 0]             s_axi_awlen,
    input  wire [ 2 : 0]             s_axi_awsize,
    input  wire [ 1 : 0]             s_axi_awburst,
    input  wire                      s_axi_awvalid,
    output wire                      s_axi_awready,

    // Write Data Channel
    input  wire [15 : 0]             s_axi_wdata,
    input  wire [ 1 : 0]             s_axi_wstrb,
    input  wire                      s_axi_wlast,
    input  wire                      s_axi_wvalid,
    output wire                      s_axi_wready,

    // Write Response Channel
    output wire [ID_WIDTH - 1 : 0]   s_axi_bid,
    output wire [ 1 : 0]             s_axi_bresp,
    output wire                      s_axi_bvalid,
    input  wire                      s_axi_bready,

    // Read Address Channel
    input  wire [ID_WIDTH - 1 : 0]   s_axi_arid,
    input  wire [ADDR_WIDTH - 1 : 0] s_axi_araddr,
    input  wire [ 7 : 0]             s_axi_arlen,
    input  wire [ 2 : 0]             s_axi_arsize,
    input  wire [ 1 : 0]             s_axi_arburst,
    input  wire                      s_axi_arvalid,
    output wire                      s_axi_arready,

    // Read Data Channel
    output wire [ID_WIDTH - 1 : 0]   s_axi_rid,
    output wire [15 : 0]             s_axi_rdata,
    output wire [ 1 : 0]             s_axi_rresp,
    output wire                      s_axi_rlast,
    output wire                      s_axi_rvalid,
    input  wire                      s_axi_rready,

    // Statistics
    output wire [31:0]               count_long_o,
    output wire [31:0]               count_short_o,

    // HyperRAM physical pins
    output wire                      hr_resetn_o,
    output wire                      hr_csn_o,
    output wire                      hr_ck_o,
    inout  wire                      hr_rwds_io,
    inout  wire [ 7:0]               hr_dq_io
);

    // -------------------------------------------------------------------
    // Internal Avalon MM wires (bridge → hyperram controller)
    // -------------------------------------------------------------------
    wire        avm_write;
    wire        avm_read;
    wire [ADDR_WIDTH - 1 : 0] avm_address;
    wire [15:0] avm_writedata;
    wire [ 1:0] avm_byteenable;
    wire [ 7:0] avm_burstcount;
    wire [15:0] avm_readdata;
    wire        avm_readdatavalid;
    wire        avm_waitrequest;

    // -------------------------------------------------------------------
    // AXI4 → Avalon MM Bridge
    // -------------------------------------------------------------------
    AxiToAvalonBridge #(
        .ADDR_WIDTH (ADDR_WIDTH),
        .ID_WIDTH   (ID_WIDTH)
    ) u_axi2avl (
        .aclk               (aclk),
        .resetn              (resetn),
        // AXI write address
        .s_axi_awid          (s_axi_awid),
        .s_axi_awaddr        (s_axi_awaddr),
        .s_axi_awlen         (s_axi_awlen),
        .s_axi_awsize        (s_axi_awsize),
        .s_axi_awburst       (s_axi_awburst),
        .s_axi_awvalid       (s_axi_awvalid),
        .s_axi_awready       (s_axi_awready),
        // AXI write data
        .s_axi_wdata         (s_axi_wdata),
        .s_axi_wstrb         (s_axi_wstrb),
        .s_axi_wlast         (s_axi_wlast),
        .s_axi_wvalid        (s_axi_wvalid),
        .s_axi_wready        (s_axi_wready),
        // AXI write response
        .s_axi_bid           (s_axi_bid),
        .s_axi_bresp         (s_axi_bresp),
        .s_axi_bvalid        (s_axi_bvalid),
        .s_axi_bready        (s_axi_bready),
        // AXI read address
        .s_axi_arid          (s_axi_arid),
        .s_axi_araddr        (s_axi_araddr),
        .s_axi_arlen         (s_axi_arlen),
        .s_axi_arsize        (s_axi_arsize),
        .s_axi_arburst       (s_axi_arburst),
        .s_axi_arvalid       (s_axi_arvalid),
        .s_axi_arready       (s_axi_arready),
        // AXI read data
        .s_axi_rid           (s_axi_rid),
        .s_axi_rdata         (s_axi_rdata),
        .s_axi_rresp         (s_axi_rresp),
        .s_axi_rlast         (s_axi_rlast),
        .s_axi_rvalid        (s_axi_rvalid),
        .s_axi_rready        (s_axi_rready),
        // Avalon MM master
        .avm_write           (avm_write),
        .avm_read            (avm_read),
        .avm_address         (avm_address),
        .avm_writedata       (avm_writedata),
        .avm_byteenable      (avm_byteenable),
        .avm_burstcount      (avm_burstcount),
        .avm_readdata        (avm_readdata),
        .avm_readdatavalid   (avm_readdatavalid),
        .avm_waitrequest     (avm_waitrequest)
    );

    // -------------------------------------------------------------------
    // Internal wires between hyperram controller and PHY
    // -------------------------------------------------------------------
    wire        hr_rwds_out;
    wire        hr_rwds_in;
    wire        hr_rwds_oe_n;
    wire [ 7:0] hr_dq_out;
    wire [ 7:0] hr_dq_in;
    wire [ 7:0] hr_dq_oe_n;
    wire        hr_resetn;
    wire        hr_csn;
    wire        hr_ck;

    // -------------------------------------------------------------------
    // HyperRAM Controller (Avalon MM slave)
    // -------------------------------------------------------------------
    hyperram #(
        .G_ERRATA_ISSI_D_FIX (G_ERRATA_ISSI_D_FIX != 0)
    ) u_hyperram (
        .clk_i               (aclk),
        .clk_del_i           (aclk90degree),
        .delay_refclk_i      (delay_refclk_i),
        .rst_i               (~resetn),
        .avm_write_i         (avm_write),
        .avm_read_i          (avm_read),
        .avm_address_i       (avm_address),
        .avm_writedata_i     (avm_writedata),
        .avm_byteenable_i    (avm_byteenable),
        .avm_burstcount_i    (avm_burstcount),
        .avm_readdata_o      (avm_readdata),
        .avm_readdatavalid_o (avm_readdatavalid),
        .avm_waitrequest_o   (avm_waitrequest),
        .count_long_o        (count_long_o),
        .count_short_o       (count_short_o),
        .hr_resetn_o         (hr_resetn),
        .hr_csn_o            (hr_csn),
        .hr_ck_o             (hr_ck),
        .hr_rwds_in_i        (hr_rwds_in),
        .hr_rwds_out_o       (hr_rwds_out),
        .hr_rwds_oe_n_o      (hr_rwds_oe_n),
        .hr_dq_in_i          (hr_dq_in),
        .hr_dq_out_o         (hr_dq_out),
        .hr_dq_oe_n_o        (hr_dq_oe_n)
    );

    // -------------------------------------------------------------------
    // HyperRAM PHY (IOBUF tri-state buffers)
    // -------------------------------------------------------------------
    HyperRamPhy #(
        .MEM_WIDTH (8)
    ) u_phy (
        .s_phy_data_i   (hr_dq_out),
        .s_phy_data_o   (hr_dq_in),
        .s_phy_data_noe (hr_dq_oe_n),
        .s_phy_rwds_i   (hr_rwds_out),
        .s_phy_rwds_o   (hr_rwds_in),
        .s_phy_rwds_noe (hr_rwds_oe_n),
        .s_phy_csn      (hr_csn),
        .s_phy_ck       (hr_ck),
        .s_phy_resetn   (hr_resetn),
        .m_hr_data      (hr_dq_io),
        .m_hr_rwds      (hr_rwds_io),
        .m_hr_csn       (hr_csn_o),
        .m_hr_ck        (hr_ck_o),
        .m_hr_resetn    (hr_resetn_o)
    );

endmodule
