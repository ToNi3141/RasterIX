
################################################################
# This is a generated script based on design: design_1
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2022.2
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   catch {common::send_gid_msg -ssname BD::TCL -id 2041 -severity "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source design_1_script.tcl


# The design that will be created by this Tcl script contains the following 
# module references:
# AsyncSramController, AsyncSramPhy, AxisFramebufferReader, DisplayController8BitILI9341, RasterIX, Serial2AXIS

# Please add the sources of those modules before sourcing this Tcl script.

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7a35tcpg236-1
   set_property BOARD_PART digilentinc.com:cmod_a7-35t:part0:1.2 [current_project]
}


# CHANGE DESIGN NAME HERE
variable design_name
set design_name design_1

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_gid_msg -ssname BD::TCL -id 2001 -severity "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_gid_msg -ssname BD::TCL -id 2002 -severity "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_gid_msg -ssname BD::TCL -id 2003 -severity "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_gid_msg -ssname BD::TCL -id 2004 -severity "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_gid_msg -ssname BD::TCL -id 2005 -severity "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_gid_msg -ssname BD::TCL -id 2006 -severity "ERROR" $errMsg}
   return $nRet
}

set bCheckIPsPassed 1
##################################################################
# CHECK IPs
##################################################################
set bCheckIPs 1
if { $bCheckIPs == 1 } {
   set list_check_ips "\ 
xilinx.com:ip:axi_crossbar:2.1\
xilinx.com:ip:axis_dwidth_converter:1.1\
xilinx.com:ip:clk_wiz:6.0\
xilinx.com:ip:proc_sys_reset:5.0\
"

   set list_ips_missing ""
   common::send_gid_msg -ssname BD::TCL -id 2011 -severity "INFO" "Checking if the following IPs exist in the project's IP catalog: $list_check_ips ."

   foreach ip_vlnv $list_check_ips {
      set ip_obj [get_ipdefs -all $ip_vlnv]
      if { $ip_obj eq "" } {
         lappend list_ips_missing $ip_vlnv
      }
   }

   if { $list_ips_missing ne "" } {
      catch {common::send_gid_msg -ssname BD::TCL -id 2012 -severity "ERROR" "The following IPs are not found in the IP Catalog:\n  $list_ips_missing\n\nResolution: Please add the repository containing the IP(s) to the project." }
      set bCheckIPsPassed 0
   }

}

##################################################################
# CHECK Modules
##################################################################
set bCheckModules 1
if { $bCheckModules == 1 } {
   set list_check_mods "\ 
AsyncSramController\
AsyncSramPhy\
AxisFramebufferReader\
DisplayController8BitILI9341\
RasterIX\
Serial2AXIS\
"

   set list_mods_missing ""
   common::send_gid_msg -ssname BD::TCL -id 2020 -severity "INFO" "Checking if the following modules exist in the project's sources: $list_check_mods ."

   foreach mod_vlnv $list_check_mods {
      if { [can_resolve_reference $mod_vlnv] == 0 } {
         lappend list_mods_missing $mod_vlnv
      }
   }

   if { $list_mods_missing ne "" } {
      catch {common::send_gid_msg -ssname BD::TCL -id 2021 -severity "ERROR" "The following module(s) are not found in the project: $list_mods_missing" }
      common::send_gid_msg -ssname BD::TCL -id 2022 -severity "INFO" "Please add source files for the missing module(s) above."
      set bCheckIPsPassed 0
   }
}

if { $bCheckIPsPassed != 1 } {
  common::send_gid_msg -ssname BD::TCL -id 2023 -severity "WARNING" "Will not continue with creation of design due to the error(s) above."
  return 3
}

##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports

  # Create ports
  set MemAdr [ create_bd_port -dir O -from 18 -to 0 MemAdr ]
  set MemDB [ create_bd_port -dir IO -from 7 -to 0 MemDB ]
  set RamCEn [ create_bd_port -dir O RamCEn ]
  set RamOEn [ create_bd_port -dir O RamOEn ]
  set RamWEn [ create_bd_port -dir O RamWEn ]
  set dispCs [ create_bd_port -dir O dispCs ]
  set dispData [ create_bd_port -dir O -from 7 -to 0 dispData ]
  set dispDc [ create_bd_port -dir O dispDc ]
  set dispRd [ create_bd_port -dir O dispRd ]
  set dispRst [ create_bd_port -dir O dispRst ]
  set dispWr [ create_bd_port -dir O dispWr ]
  set reset [ create_bd_port -dir I -type rst reset ]
  set_property -dict [ list \
   CONFIG.POLARITY {ACTIVE_HIGH} \
 ] $reset
  set serCs [ create_bd_port -dir I serCs ]
  set serCts [ create_bd_port -dir O serCts ]
  set serMiso [ create_bd_port -dir O serMiso ]
  set serMosi [ create_bd_port -dir I serMosi ]
  set serSck [ create_bd_port -dir I serSck ]
  set sys_clock [ create_bd_port -dir I -type clk -freq_hz 12000000 sys_clock ]
  set_property -dict [ list \
   CONFIG.PHASE {0.000} \
 ] $sys_clock

  # Create instance: AsyncSramController_0, and set properties
  set block_name AsyncSramController
  set block_cell_name AsyncSramController_0
  if { [catch {set AsyncSramController_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $AsyncSramController_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property CONFIG.ID_WIDTH {9} $AsyncSramController_0


  # Create instance: AsyncSramPhy_0, and set properties
  set block_name AsyncSramPhy
  set block_cell_name AsyncSramPhy_0
  if { [catch {set AsyncSramPhy_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $AsyncSramPhy_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: AxisFramebufferReader_0, and set properties
  set block_name AxisFramebufferReader
  set block_cell_name AxisFramebufferReader_0
  if { [catch {set AxisFramebufferReader_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $AxisFramebufferReader_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: DisplayController8Bi_1, and set properties
  set block_name DisplayController8BitILI9341
  set block_cell_name DisplayController8Bi_1
  if { [catch {set DisplayController8Bi_1 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $DisplayController8Bi_1 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [list \
    CONFIG.CLOCK_DIV {2} \
    CONFIG.LANDSCAPE_CONFIG {1} \
  ] $DisplayController8Bi_1


  # Create instance: RasterIX_0, and set properties
  set block_name RasterIX
  set block_cell_name RasterIX_0
  if { [catch {set RasterIX_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $RasterIX_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [list \
    CONFIG.ENABLE_DEPTH_BUFFER {1} \
    CONFIG.ENABLE_FOG {1} \
    CONFIG.ENABLE_MIPMAPPING {1} \
    CONFIG.ENABLE_STENCIL_BUFFER {1} \
    CONFIG.ENABLE_TEXTURE_FILTERING {1} \
    CONFIG.FRAMEBUFFER_SUB_PIXEL_WIDTH {5} \
    CONFIG.MAX_TEXTURE_SIZE {128} \
    CONFIG.RASTERIZER_FIXPOINT_PRECISION {25} \
    CONFIG.SUB_PIXEL_CALC_PRECISION {6} \
    CONFIG.TEXTURE_PAGE_SIZE {2048} \
    CONFIG.TMU_COUNT {1} \
    CONFIG.VARIANT {ef} \
  ] $RasterIX_0


  # Create instance: Serial2AXIS_0, and set properties
  set block_name Serial2AXIS
  set block_cell_name Serial2AXIS_0
  if { [catch {set Serial2AXIS_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $Serial2AXIS_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [list \
    CONFIG.FIFO_SIZE {32768} \
    CONFIG.FIFO_TRESHOLD {2048} \
  ] $Serial2AXIS_0


  # Create instance: axi_crossbar_0, and set properties
  set axi_crossbar_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_crossbar:2.1 axi_crossbar_0 ]
  set_property -dict [list \
    CONFIG.NUM_MI {1} \
    CONFIG.NUM_SI {2} \
  ] $axi_crossbar_0


  # Create instance: axis_dwidth_converter_0, and set properties
  set axis_dwidth_converter_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_dwidth_converter:1.1 axis_dwidth_converter_0 ]

  # Create instance: clk_wiz_0, and set properties
  set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
  set_property -dict [list \
    CONFIG.AXI_DRP {false} \
    CONFIG.CLKIN1_JITTER_PS {833.33} \
    CONFIG.CLKOUT1_DRIVES {BUFG} \
    CONFIG.CLKOUT1_JITTER {479.872} \
    CONFIG.CLKOUT1_PHASE_ERROR {668.310} \
    CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {100} \
    CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {100.000} \
    CONFIG.CLKOUT3_REQUESTED_OUT_FREQ {100.000} \
    CONFIG.CLKOUT4_REQUESTED_OUT_FREQ {100.000} \
    CONFIG.CLKOUT5_REQUESTED_OUT_FREQ {100.000} \
    CONFIG.CLKOUT6_REQUESTED_OUT_FREQ {100.000} \
    CONFIG.CLKOUT7_REQUESTED_OUT_FREQ {100.000} \
    CONFIG.CLK_IN1_BOARD_INTERFACE {sys_clock} \
    CONFIG.CLK_OUT1_PORT {clk_out1} \
    CONFIG.FEEDBACK_SOURCE {FDBK_AUTO} \
    CONFIG.JITTER_SEL {Min_O_Jitter} \
    CONFIG.MMCM_BANDWIDTH {HIGH} \
    CONFIG.MMCM_CLKFBOUT_MULT_F {62.500} \
    CONFIG.MMCM_CLKIN1_PERIOD {83.333} \
    CONFIG.MMCM_CLKIN2_PERIOD {10.0} \
    CONFIG.MMCM_CLKOUT0_DIVIDE_F {7.500} \
    CONFIG.MMCM_COMPENSATION {ZHOLD} \
    CONFIG.PHASE_DUTY_CONFIG {false} \
    CONFIG.RESET_BOARD_INTERFACE {Custom} \
    CONFIG.USE_BOARD_FLOW {true} \
    CONFIG.USE_DYN_PHASE_SHIFT {false} \
    CONFIG.USE_DYN_RECONFIG {false} \
    CONFIG.USE_FREQ_SYNTH {true} \
    CONFIG.USE_RESET {false} \
  ] $clk_wiz_0


  # Create instance: proc_sys_reset_0, and set properties
  set proc_sys_reset_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset_0 ]
  set_property -dict [list \
    CONFIG.RESET_BOARD_INTERFACE {reset} \
    CONFIG.USE_BOARD_FLOW {true} \
  ] $proc_sys_reset_0


  # Create interface connections
  connect_bd_intf_net -intf_net AxisFramebufferReader_0_m_disp_axis [get_bd_intf_pins AxisFramebufferReader_0/m_disp_axis] [get_bd_intf_pins axis_dwidth_converter_0/S_AXIS]
  connect_bd_intf_net -intf_net AxisFramebufferReader_0_m_mem_axi [get_bd_intf_pins AxisFramebufferReader_0/m_mem_axi] [get_bd_intf_pins axi_crossbar_0/S01_AXI]
  connect_bd_intf_net -intf_net RasterIX_0_m_axi [get_bd_intf_pins RasterIX_0/m_axi] [get_bd_intf_pins axi_crossbar_0/S00_AXI]
  connect_bd_intf_net -intf_net Serial2AXIS_0_m_axis [get_bd_intf_pins RasterIX_0/s_cmd_axis] [get_bd_intf_pins Serial2AXIS_0/m_axis]
  connect_bd_intf_net -intf_net axi_crossbar_0_M00_AXI [get_bd_intf_pins AsyncSramController_0/s_axi] [get_bd_intf_pins axi_crossbar_0/M00_AXI]
  connect_bd_intf_net -intf_net axis_dwidth_converter_0_M_AXIS [get_bd_intf_pins DisplayController8Bi_1/s_axis] [get_bd_intf_pins axis_dwidth_converter_0/M_AXIS]

  # Create port connections
  connect_bd_net -net AsyncSramController_0_mem_addr [get_bd_pins AsyncSramController_0/mem_addr] [get_bd_pins AsyncSramPhy_0/s_mem_addr]
  connect_bd_net -net AsyncSramController_0_mem_data_o [get_bd_pins AsyncSramController_0/mem_data_o] [get_bd_pins AsyncSramPhy_0/s_mem_data_i]
  connect_bd_net -net AsyncSramController_0_mem_data_t [get_bd_pins AsyncSramController_0/mem_data_t] [get_bd_pins AsyncSramPhy_0/s_mem_data_t]
  connect_bd_net -net AsyncSramController_0_mem_nce [get_bd_pins AsyncSramController_0/mem_nce] [get_bd_pins AsyncSramPhy_0/s_mem_nce]
  connect_bd_net -net AsyncSramController_0_mem_noe [get_bd_pins AsyncSramController_0/mem_noe] [get_bd_pins AsyncSramPhy_0/s_mem_noe]
  connect_bd_net -net AsyncSramController_0_mem_nwe [get_bd_pins AsyncSramController_0/mem_nwe] [get_bd_pins AsyncSramPhy_0/s_mem_nwe]
  connect_bd_net -net AsyncSramPhy_0_m_mem_addr [get_bd_ports MemAdr] [get_bd_pins AsyncSramPhy_0/m_mem_addr]
  connect_bd_net -net AsyncSramPhy_0_m_mem_nce [get_bd_ports RamCEn] [get_bd_pins AsyncSramPhy_0/m_mem_nce]
  connect_bd_net -net AsyncSramPhy_0_m_mem_noe [get_bd_ports RamOEn] [get_bd_pins AsyncSramPhy_0/m_mem_noe]
  connect_bd_net -net AsyncSramPhy_0_m_mem_nwe [get_bd_ports RamWEn] [get_bd_pins AsyncSramPhy_0/m_mem_nwe]
  connect_bd_net -net AsyncSramPhy_0_s_mem_data_o [get_bd_pins AsyncSramController_0/mem_data_i] [get_bd_pins AsyncSramPhy_0/s_mem_data_o]
  connect_bd_net -net AxisFramebufferReader_0_fb_swapped [get_bd_pins AxisFramebufferReader_0/fb_swapped] [get_bd_pins RasterIX_0/fb_swapped]
  connect_bd_net -net DisplayController8Bi_1_cs [get_bd_ports dispCs] [get_bd_pins DisplayController8Bi_1/cs]
  connect_bd_net -net DisplayController8Bi_1_data [get_bd_ports dispData] [get_bd_pins DisplayController8Bi_1/data]
  connect_bd_net -net DisplayController8Bi_1_dc [get_bd_ports dispDc] [get_bd_pins DisplayController8Bi_1/dc]
  connect_bd_net -net DisplayController8Bi_1_rd [get_bd_ports dispRd] [get_bd_pins DisplayController8Bi_1/rd]
  connect_bd_net -net DisplayController8Bi_1_rst [get_bd_ports dispRst] [get_bd_pins DisplayController8Bi_1/rst]
  connect_bd_net -net DisplayController8Bi_1_wr [get_bd_ports dispWr] [get_bd_pins DisplayController8Bi_1/wr]
  connect_bd_net -net Net [get_bd_ports MemDB] [get_bd_pins AsyncSramPhy_0/m_mem_data]
  connect_bd_net -net RasterIX_0_fb_addr [get_bd_pins AxisFramebufferReader_0/fb_addr] [get_bd_pins RasterIX_0/fb_addr]
  connect_bd_net -net RasterIX_0_fb_size [get_bd_pins AxisFramebufferReader_0/fb_size] [get_bd_pins RasterIX_0/fb_size]
  connect_bd_net -net RasterIX_0_swap_fb [get_bd_pins AxisFramebufferReader_0/swap_fb] [get_bd_pins RasterIX_0/swap_fb]
  connect_bd_net -net Serial2AXIS_0_serial_cts [get_bd_ports serCts] [get_bd_pins Serial2AXIS_0/serial_cts]
  connect_bd_net -net Serial2AXIS_0_serial_miso [get_bd_ports serMiso] [get_bd_pins Serial2AXIS_0/serial_miso]
  connect_bd_net -net clk_wiz_0_clk_out1 [get_bd_pins AsyncSramController_0/aclk] [get_bd_pins AxisFramebufferReader_0/aclk] [get_bd_pins DisplayController8Bi_1/aclk] [get_bd_pins RasterIX_0/aclk] [get_bd_pins Serial2AXIS_0/aclk] [get_bd_pins axi_crossbar_0/aclk] [get_bd_pins axis_dwidth_converter_0/aclk] [get_bd_pins clk_wiz_0/clk_out1] [get_bd_pins proc_sys_reset_0/slowest_sync_clk]
  connect_bd_net -net clk_wiz_0_locked [get_bd_pins clk_wiz_0/locked] [get_bd_pins proc_sys_reset_0/dcm_locked]
  connect_bd_net -net proc_sys_reset_0_peripheral_aresetn [get_bd_pins AsyncSramController_0/resetn] [get_bd_pins AxisFramebufferReader_0/resetn] [get_bd_pins DisplayController8Bi_1/resetn] [get_bd_pins RasterIX_0/resetn] [get_bd_pins Serial2AXIS_0/resetn] [get_bd_pins axi_crossbar_0/aresetn] [get_bd_pins axis_dwidth_converter_0/aresetn] [get_bd_pins proc_sys_reset_0/peripheral_aresetn]
  connect_bd_net -net reset_1 [get_bd_ports reset] [get_bd_pins proc_sys_reset_0/ext_reset_in]
  connect_bd_net -net serCs_1 [get_bd_ports serCs] [get_bd_pins Serial2AXIS_0/serial_ncs]
  connect_bd_net -net serMosi_1 [get_bd_ports serMosi] [get_bd_pins Serial2AXIS_0/serial_mosi]
  connect_bd_net -net serSck_1 [get_bd_ports serSck] [get_bd_pins Serial2AXIS_0/serial_sck]
  connect_bd_net -net sys_clock_1 [get_bd_ports sys_clock] [get_bd_pins clk_wiz_0/clk_in1]

  # Create address segments
  assign_bd_address -offset 0x00000000 -range 0x00080000 -target_address_space [get_bd_addr_spaces AxisFramebufferReader_0/m_mem_axi] [get_bd_addr_segs AsyncSramController_0/s_axi/reg0] -force
  assign_bd_address -offset 0x00000000 -range 0x00080000 -target_address_space [get_bd_addr_spaces RasterIX_0/m_axi] [get_bd_addr_segs AsyncSramController_0/s_axi/reg0] -force


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


