#create_clock -period 10.000 -name clk_100m -waveform {0.000 5.000} [get_ports clk_100m_pin]
#set_input_jitter clk_100m 0.200

# Pinout file for Black Mesa Labs' Spartan7 M2 Board
set_property CFGBVS VCCO [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
# set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets clk_100m_pin_IBUF] 
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets design_1_i/clk_wiz_0/inst/clk_in1_design_1_clk_wiz_0_0]

# XC7S25 has limited BUFGs per half — allow MMCM output to cross CMT boundary
set_property CLOCK_DEDICATED_ROUTE ANY_CMT_COLUMN [get_nets design_1_i/clk_wiz_0/inst/clk_out1_design_1_clk_wiz_0_0]

# Config PROM. Note spi_sck handled by STARTUPE2 hard IP
# set_property PACKAGE_PIN B11 [get_ports {spi_mosi}]
# set_property PACKAGE_PIN B12 [get_ports {spi_miso}]
# set_property PACKAGE_PIN C11 [get_ports {spi_cs_l}]
# set_property PACKAGE_PIN A8  [get_ports {spi_sck}]

# S7 Mini Board
# set_property PACKAGE_PIN A10 [get_ports {rst_l}]
set_property PACKAGE_PIN L5  [get_ports {clk_100m_pin}]
# set_property PACKAGE_PIN A13 [get_ports {ftdi[0]}]
# set_property PACKAGE_PIN A12 [get_ports {ftdi[1]}]
# set_property PACKAGE_PIN A5  [get_ports {ftdi[2]}]
# set_property PACKAGE_PIN B5  [get_ports {ftdi[3]}]
# set_property PACKAGE_PIN D3  [get_ports {j1_l}]
# set_property PACKAGE_PIN A4  [get_ports {j2_l}]
set_property PACKAGE_PIN D14 [get_ports {led1}]
set_property PACKAGE_PIN C14 [get_ports {led2}]

# S7 Mini 32 DIP I/O and 64 50mil I/O (shared)
set_property PACKAGE_PIN A2 [get_ports {disp_cs}]
# set_property PACKAGE_PIN B3 [get_ports {port_a[4]}]
set_property PACKAGE_PIN C4 [get_ports {disp_dc}]
# set_property PACKAGE_PIN C5 [get_ports {port_a[5]}]
set_property PACKAGE_PIN D4 [get_ports {disp_wr}]
# set_property PACKAGE_PIN E4 [get_ports {port_a[6]}]
set_property PACKAGE_PIN A3 [get_ports {disp_rd}]
# set_property PACKAGE_PIN C3 [get_ports {port_a[7]}]

set_property PACKAGE_PIN B2 [get_ports {disp_rst}]
# set_property PACKAGE_PIN B1 [get_ports {port_b[4]}]
# set_property PACKAGE_PIN C1 [get_ports {port_b[1]}]
# set_property PACKAGE_PIN D1 [get_ports {port_b[5]}]
# set_property PACKAGE_PIN D2 [get_ports {port_b[2]}]
# set_property PACKAGE_PIN E2 [get_ports {port_b[6]}]
# set_property PACKAGE_PIN F1 [get_ports {port_b[3]}]
# set_property PACKAGE_PIN G1 [get_ports {port_b[7]}]

set_property PACKAGE_PIN F3 [get_ports {disp_data[0]}]
# set_property PACKAGE_PIN F2 [get_ports {port_c[4]}]
set_property PACKAGE_PIN F4 [get_ports {disp_data[1]}]
# set_property PACKAGE_PIN G4 [get_ports {port_c[5]}]
set_property PACKAGE_PIN H3 [get_ports {disp_data[2]}]
# set_property PACKAGE_PIN H4 [get_ports {port_c[6]}]
set_property PACKAGE_PIN J3 [get_ports {disp_data[3]}]
# set_property PACKAGE_PIN J4 [get_ports {port_c[7]}]

set_property PACKAGE_PIN K3 [get_ports {disp_data[4]}]
# set_property PACKAGE_PIN K4 [get_ports {port_d[4]}]
set_property PACKAGE_PIN L2 [get_ports {disp_data[5]}]
# set_property PACKAGE_PIN L3 [get_ports {port_d[5]}]
set_property PACKAGE_PIN M2 [get_ports {disp_data[6]}]
# set_property PACKAGE_PIN M3 [get_ports {port_d[6]}]
set_property PACKAGE_PIN M4 [get_ports {disp_data[7]}]
# set_property PACKAGE_PIN M5 [get_ports {port_d[7]}]

# set_property PACKAGE_PIN E11 [get_ports {port_h[0]}]
# set_property PACKAGE_PIN C12 [get_ports {port_h[4]}]
# set_property PACKAGE_PIN C10 [get_ports {port_h[1]}]
# set_property PACKAGE_PIN D10 [get_ports {port_h[5]}]
# set_property PACKAGE_PIN D12 [get_ports {port_h[2]}]
# set_property PACKAGE_PIN D13 [get_ports {port_h[6]}]
# set_property PACKAGE_PIN E13 [get_ports {port_h[3]}]
# set_property PACKAGE_PIN F13 [get_ports {port_h[7]}]

# set_property PACKAGE_PIN F14 [get_ports {port_g[0]}]
# set_property PACKAGE_PIN G14 [get_ports {port_g[4]}]
# set_property PACKAGE_PIN E12 [get_ports {port_g[1]}]
# set_property PACKAGE_PIN F12 [get_ports {port_g[5]}]
# set_property PACKAGE_PIN F11 [get_ports {port_g[2]}]
# set_property PACKAGE_PIN G11 [get_ports {port_g[6]}]
# set_property PACKAGE_PIN H12 [get_ports {port_g[3]}]
# set_property PACKAGE_PIN H11 [get_ports {port_g[7]}]

# set_property PACKAGE_PIN H13 [get_ports {port_f[0]}]
# set_property PACKAGE_PIN H14 [get_ports {port_f[4]}]
# set_property PACKAGE_PIN J13 [get_ports {port_f[1]}]
# set_property PACKAGE_PIN J14 [get_ports {port_f[5]}]
set_property PACKAGE_PIN L12 [get_ports {serial_cts}]
# set_property PACKAGE_PIN L13 [get_ports {port_f[6]}]
set_property PACKAGE_PIN L14 [get_ports {serial_ncs}]
# set_property PACKAGE_PIN M13 [get_ports {port_f[7]}]

set_property PACKAGE_PIN J12 [get_ports {serial_sck}]
# set_property PACKAGE_PIN J11 [get_ports {port_e[4]}]
set_property PACKAGE_PIN M14 [get_ports {serial_miso}]
# set_property PACKAGE_PIN N14 [get_ports {port_e[5]}]
set_property PACKAGE_PIN K12 [get_ports {serial_mosi}]
# set_property PACKAGE_PIN K11 [get_ports {port_e[6]}]
set_property PACKAGE_PIN M12 [get_ports {reset}]
# set_property PACKAGE_PIN M11 [get_ports {port_e[7]}]



set_property PACKAGE_PIN P2   [get_ports {hr_cs_l}]
set_property PACKAGE_PIN P3   [get_ports {hr_rst_l}]
set_property PACKAGE_PIN N1   [get_ports {hr_ck}]
set_property PACKAGE_PIN P4   [get_ports {hr_rwds}]
set_property PACKAGE_PIN N4   [get_ports {hr_dq[2]}]
set_property PACKAGE_PIN P12  [get_ports {hr_dq[1]}]
set_property PACKAGE_PIN P11  [get_ports {hr_dq[0]}]
set_property PACKAGE_PIN P10  [get_ports {hr_dq[3]}]
set_property PACKAGE_PIN P5   [get_ports {hr_dq[4]}]
set_property PACKAGE_PIN P13  [get_ports {hr_dq[7]}]
set_property PACKAGE_PIN N11  [get_ports {hr_dq[6]}]
set_property PACKAGE_PIN N10  [get_ports {hr_dq[5]}]

set_property IOSTANDARD LVCMOS33 [get_ports {hr_*}]
set_property IOSTANDARD LVCMOS33 [get_ports {serial_*}]
set_property IOSTANDARD LVCMOS33 [get_ports {disp_*}]
set_property IOSTANDARD LVCMOS33 [get_ports {reset}]

# set_property IOSTANDARD LVCMOS33 [get_ports rst_l]
set_property IOSTANDARD LVCMOS33 [get_ports clk_100m_pin]
# set_property IOSTANDARD LVCMOS33 [get_ports j1_l]
# set_property IOSTANDARD LVCMOS33 [get_ports j2_l]
set_property IOSTANDARD LVCMOS33 [get_ports led1]
set_property IOSTANDARD LVCMOS33 [get_ports led2]
# set_property IOSTANDARD LVCMOS33 [get_ports {ftdi[*]}]
# set_property IOSTANDARD LVCMOS33 [get_ports {spi_*}]
# set_property IOSTANDARD LVCMOS33 [get_ports {port_*}]

# set_property PULLUP     TRUE     [get_ports rst_l]
set_property PULLUP     TRUE     [get_ports clk_100m_pin]
# set_property PULLUP     TRUE     [get_ports j1_l]
# set_property PULLUP     TRUE     [get_ports j2_l]
# set_property PULLUP     TRUE     [get_ports {ftdi[*]}]
# set_property PULLUP     TRUE     [get_ports {spi_*]}]
set_property PULLUP     TRUE     [get_ports {hr_cs_l}]
set_property PULLDOWN   TRUE     [get_ports {hr_rst_l}]
set_property PULLDOWN   TRUE     [get_ports {reset}]

set_property DRIVE      16       [get_ports led1 ]
set_property DRIVE      16       [get_ports led2 ]
set_property SLEW       SLOW     [get_ports led1 ]
set_property SLEW       SLOW     [get_ports led2 ]



# SPI clock
create_clock -add -name clk_port_spi -period 20 -waveform {0 10} [get_ports { serial_sck }];
# SPI
set_input_delay -max -clock clk_port_spi 3.5 [get_ports reset]
set_input_delay -max -clock clk_port_spi 3.5 [get_ports serial_mosi]
set_input_delay -max -clock clk_port_spi 3.5 [get_ports serial_ncs]
set_output_delay -max -clock clk_port_spi 3.5 [get_ports serial_miso]
set_input_delay -min -clock clk_port_spi 3.0 [get_ports reset]
set_input_delay -min -clock clk_port_spi 3.0 [get_ports serial_mosi]
set_input_delay -min -clock clk_port_spi 3.0 [get_ports serial_ncs]
set_output_delay -min -clock clk_port_spi 3.0 [get_ports serial_miso]

# Virtual 100MHz clock
create_generated_clock -name ctrl_clk  [get_pins design_1_i/clk_wiz_0/inst/mmcm_adv_inst/CLKOUT0]; 
set_input_delay -max -clock ctrl_clk 3.5 [get_ports reset]
set_output_delay -max -clock ctrl_clk 3.5 [get_ports serial_cts]
set_input_delay -min -clock ctrl_clk 3.0 [get_ports reset]
set_output_delay -min -clock ctrl_clk 3.0 [get_ports serial_cts]
# Display
set_output_delay -max -clock ctrl_clk 3.5 [get_ports disp_data[*]]
set_output_delay -max -clock ctrl_clk 3.5 [get_ports disp_rd]
set_output_delay -max -clock ctrl_clk 3.5 [get_ports disp_wr]
set_output_delay -max -clock ctrl_clk 3.5 [get_ports disp_dc]
set_output_delay -max -clock ctrl_clk 3.5 [get_ports disp_cs]
set_output_delay -max -clock ctrl_clk 3.5 [get_ports disp_rst]
set_output_delay -min -clock ctrl_clk 3.0 [get_ports disp_data[*]]
set_output_delay -min -clock ctrl_clk 3.0 [get_ports disp_rd]
set_output_delay -min -clock ctrl_clk 3.0 [get_ports disp_wr]
set_output_delay -min -clock ctrl_clk 3.0 [get_ports disp_dc]
set_output_delay -min -clock ctrl_clk 3.0 [get_ports disp_cs]
set_output_delay -min -clock ctrl_clk 3.0 [get_ports disp_rst]

set_false_path -from [get_clocks clk_port_spi] -to [get_clocks -of_objects [get_pins design_1_i/clk_wiz_0/inst/mmcm_adv_inst/CLKOUT0]]
set_false_path -from [get_clocks -of_objects [get_pins design_1_i/clk_wiz_0/inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks clk_port_spi]

########### HyperRAM timing #################
# Rename autogenerated clocks
create_generated_clock -name delay_refclk [get_pins design_1_i/clk_wiz_0/inst/mmcm_adv_inst/CLKOUT2]
create_generated_clock -name hr_clk_del   [get_pins design_1_i/clk_wiz_0/inst/mmcm_adv_inst/CLKOUT1]

# HyperRAM output clock relative to delayed clock
create_generated_clock -name hr_ck_fpga         [get_ports hr_ck] \
   -source [get_pins design_1_i/clk_wiz_0/inst/mmcm_adv_inst/CLKOUT1] -multiply_by 1

# HyperRAM RWDS as a clock for the read path (hr_dq -> IDDR -> CDC)
create_clock -period 13.300 -name hr_rwds_fpga -waveform {3.325 9.975} [get_ports hr_rwds]

# Asynchronous clocks
set_false_path -from [get_ports hr_rwds] -to [get_clocks hr_ck_fpga]

# Clock Domain Crossing
set_max_delay 3.325 -datapath_only -from [get_cells design_1_i/HyperRamWrapper_0/inst/u_hyperram/hyperram_ctrl_inst/hb_read_o_reg]
set_max_delay 3.325 -datapath_only -from [get_cells design_1_i/HyperRamWrapper_0/inst/u_hyperram/hyperram_rx_inst/iddr_dq_gen[*].iddr_dq_inst]

# Prevent insertion of extra BUFG
set_property CLOCK_BUFFER_TYPE NONE [get_nets -of [get_pins design_1_i/HyperRamWrapper_0/inst/u_hyperram/hyperram_rx_inst/delay_rwds_inst/DATAOUT]]
# Receive FIFO: There is a CDC in the LUTRAM.
# There is approx 1.1 ns Clock->Data delay for the LUTRAM itself, plus 0.5 ns routing delay to the capture flip-flop.
set_max_delay 3.325 -datapath_only -from [get_clocks hr_rwds_fpga] -to [get_clocks ctrl_clk]

################################################################################
# HyperRAM timing (correct for IS66WVH8M8DBLL-100B1LI)

set tCKHP    6.65 ; # Clock Half Period
set HR_tIS   1.0 ; # input setup time
set HR_tIH   1.0 ; # input hold time
set tDSSmax  0.8 ; # RWDS to data valid, max
set tDSHmin -0.8 ; # RWDS to data invalid, min

################################################################################
# FPGA to HyperRAM (address and write data)

set_property IOB TRUE [get_cells design_1_i/HyperRamWrapper_0/inst/u_hyperram/hyperram_tx_inst/hr_rwds_oe_n_reg ]
set_property IOB TRUE [get_cells design_1_i/HyperRamWrapper_0/inst/u_hyperram/hyperram_tx_inst/hr_dq_oe_n_reg[*] ]
set_property IOB TRUE [get_cells design_1_i/HyperRamWrapper_0/inst/u_hyperram/hyperram_ctrl_inst/hb_csn_o_reg ]
set_property IOB TRUE [get_cells design_1_i/HyperRamWrapper_0/inst/u_hyperram/hyperram_ctrl_inst/hb_rstn_o_reg ]

# setup
set_output_delay -max  $HR_tIS -clock hr_ck_fpga [get_ports {hr_rst_l hr_cs_l hr_rwds hr_dq[*]}]
set_output_delay -max  $HR_tIS -clock hr_ck_fpga [get_ports {hr_rst_l hr_cs_l hr_rwds hr_dq[*]}] -clock_fall -add_delay

# hold
set_output_delay -min -$HR_tIH -clock hr_ck_fpga [get_ports {hr_rst_l hr_cs_l hr_rwds hr_dq[*]}]
set_output_delay -min -$HR_tIH -clock hr_ck_fpga [get_ports {hr_rst_l hr_cs_l hr_rwds hr_dq[*]}] -clock_fall -add_delay

################################################################################
# HyperRAM to FPGA (read data, clocked in by RWDS)
# edge aligned, so pretend that data is launched by previous edge

# setup
set_input_delay -max [expr $tCKHP + $tDSSmax] -clock hr_rwds_fpga [get_ports hr_dq[*]]
set_input_delay -max [expr $tCKHP + $tDSSmax] -clock hr_rwds_fpga [get_ports hr_dq[*]] -clock_fall -add_delay

# hold
set_input_delay -min [expr $tCKHP + $tDSHmin] -clock hr_rwds_fpga [get_ports hr_dq[*]]
set_input_delay -min [expr $tCKHP + $tDSHmin] -clock hr_rwds_fpga [get_ports hr_dq[*]] -clock_fall -add_delay


# Flash config
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
# set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]
# set_property CONFIG_MODE SPIx4 [current_design]

# HR Banks mA Ranges     HP Banks mA Ranges
# LVCMOS33 4,8,12,16
# LVCMOS25 4,8,12,16     
# LVCMOS18 4,8,12,16,24  2,4,6,8,12,16
# LVCMOS15 4,8,12,16     2,4,6,8,12,16
# LVCMOS12 4,8,12        2,4,6,8