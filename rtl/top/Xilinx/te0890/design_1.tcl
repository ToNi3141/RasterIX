
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
# DisplayController8BitILI9341, HyperRamWrapper, RasterIXSystem, Serial2AXIS

# Please add the sources of those modules before sourcing this Tcl script.

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7s25ftgb196-1
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
DisplayController8BitILI9341\
HyperRamWrapper\
RasterIXSystem\
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
  set clk_100m_pin [ create_bd_port -dir I -type clk -freq_hz 100000000 clk_100m_pin ]
  set disp_cs [ create_bd_port -dir O disp_cs ]
  set disp_data [ create_bd_port -dir O -from 7 -to 0 disp_data ]
  set disp_dc [ create_bd_port -dir O disp_dc ]
  set disp_rd [ create_bd_port -dir O disp_rd ]
  set disp_rst [ create_bd_port -dir O disp_rst ]
  set disp_wr [ create_bd_port -dir O disp_wr ]
  set hr_ck [ create_bd_port -dir O hr_ck ]
  set hr_cs_l [ create_bd_port -dir O hr_cs_l ]
  set hr_dq [ create_bd_port -dir IO -from 7 -to 0 hr_dq ]
  set hr_rst_l [ create_bd_port -dir O hr_rst_l ]
  set hr_rwds [ create_bd_port -dir IO hr_rwds ]
  set reset [ create_bd_port -dir I -type rst reset ]
  set_property -dict [ list \
   CONFIG.POLARITY {ACTIVE_HIGH} \
 ] $reset
  set serial_cts [ create_bd_port -dir O serial_cts ]
  set serial_miso [ create_bd_port -dir O serial_miso ]
  set serial_mosi [ create_bd_port -dir I serial_mosi ]
  set serial_ncs [ create_bd_port -dir I serial_ncs ]
  set serial_sck [ create_bd_port -dir I serial_sck ]

  # Create instance: DisplayController8Bi_0, and set properties
  set block_name DisplayController8BitILI9341
  set block_cell_name DisplayController8Bi_0
  if { [catch {set DisplayController8Bi_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $DisplayController8Bi_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [list \
    CONFIG.CLOCK_DIV {2} \
    CONFIG.LANDSCAPE_CONFIG {1} \
  ] $DisplayController8Bi_0


  # Create instance: HyperRamWrapper_0, and set properties
  set block_name HyperRamWrapper
  set block_cell_name HyperRamWrapper_0
  if { [catch {set HyperRamWrapper_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $HyperRamWrapper_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property CONFIG.G_ERRATA_ISSI_D_FIX {1} $HyperRamWrapper_0


  # Create instance: RasterIXSystem_0, and set properties
  set block_name RasterIXSystem
  set block_cell_name RasterIXSystem_0
  if { [catch {set RasterIXSystem_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2095 -severity "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $RasterIXSystem_0 eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2096 -severity "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
    set_property -dict [list \
    CONFIG.DATA_WIDTH {16} \
    CONFIG.FBR_BLOCKING {0} \
    CONFIG.FBR_FIFO_DEPTH_LG {5} \
    CONFIG.RIX_ENABLE_DEPTH_BUFFER {1} \
    CONFIG.RIX_ENABLE_FOG {1} \
    CONFIG.RIX_ENABLE_MEMORY_COALESCING {1} \
    CONFIG.RIX_ENABLE_MIPMAPPING {1} \
    CONFIG.RIX_ENABLE_STENCIL_BUFFER {1} \
    CONFIG.RIX_ENABLE_TEXTURE_FILTERING {1} \
    CONFIG.RIX_MAX_TEXTURE_SIZE {128} \
    CONFIG.RIX_RASTERIZER_FIXPOINT_PRECISION {18} \
    CONFIG.RIX_SUB_PIXEL_CALC_PRECISION {6} \
    CONFIG.RIX_TEXTURE_PAGE_SIZE {2048} \
    CONFIG.RIX_TMU_COUNT {1} \
    CONFIG.RIX_VARIANT {ef} \
  ] $RasterIXSystem_0


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
  
  # Create instance: clk_wiz_0, and set properties
  set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
  set_property -dict [list \
    CONFIG.CLKOUT1_JITTER {160.557} \
    CONFIG.CLKOUT1_PHASE_ERROR {132.063} \
    CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {75.000} \
    CONFIG.CLKOUT1_USED {true} \
    CONFIG.CLKOUT2_JITTER {160.557} \
    CONFIG.CLKOUT2_PHASE_ERROR {132.063} \
    CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {75.000} \
    CONFIG.CLKOUT2_REQUESTED_PHASE {90.000} \
    CONFIG.CLKOUT2_USED {true} \
    CONFIG.CLKOUT3_JITTER {132.221} \
    CONFIG.CLKOUT3_PHASE_ERROR {132.063} \
    CONFIG.CLKOUT3_REQUESTED_OUT_FREQ {200.000} \
    CONFIG.CLKOUT3_USED {true} \
    CONFIG.JITTER_SEL {No_Jitter} \
    CONFIG.MMCM_CLKFBOUT_MULT_F {6.000} \
    CONFIG.MMCM_CLKIN1_PERIOD {10.0} \
    CONFIG.MMCM_CLKIN2_PERIOD {10.0} \
    CONFIG.MMCM_CLKOUT0_DIVIDE_F {8.000} \
    CONFIG.MMCM_CLKOUT0_DUTY_CYCLE {0.5} \
    CONFIG.MMCM_CLKOUT1_DIVIDE {8} \
    CONFIG.MMCM_CLKOUT1_DUTY_CYCLE {0.5} \
    CONFIG.MMCM_CLKOUT1_PHASE {90.000} \
    CONFIG.MMCM_CLKOUT2_DIVIDE {3} \
    CONFIG.MMCM_CLKOUT2_DUTY_CYCLE {0.5} \
    CONFIG.MMCM_DIVCLK_DIVIDE {1} \
    CONFIG.NUM_OUT_CLKS {3} \
    CONFIG.USE_MIN_POWER {true} \
    CONFIG.USE_POWER_DOWN {false} \
    CONFIG.USE_RESET {false} \
  ] $clk_wiz_0


  # Create instance: proc_sys_reset_0, and set properties
  set proc_sys_reset_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net RasterIXSystem_0_m_axi [get_bd_intf_pins HyperRamWrapper_0/s_axi] [get_bd_intf_pins RasterIXSystem_0/m_axi]
  connect_bd_intf_net -intf_net RasterIXSystem_0_m_cmd_resp_axis [get_bd_intf_pins RasterIXSystem_0/m_cmd_resp_axis] [get_bd_intf_pins Serial2AXIS_0/s_axis]
  connect_bd_intf_net -intf_net RasterIXSystem_0_m_disp_axis [get_bd_intf_pins DisplayController8Bi_0/s_axis] [get_bd_intf_pins RasterIXSystem_0/m_disp_axis]
  connect_bd_intf_net -intf_net Serial2AXIS_0_m_axis [get_bd_intf_pins RasterIXSystem_0/s_cmd_axis] [get_bd_intf_pins Serial2AXIS_0/m_axis]

  # Create port connections
  connect_bd_net -net DisplayController8Bi_0_cs [get_bd_ports disp_cs] [get_bd_pins DisplayController8Bi_0/cs]
  connect_bd_net -net DisplayController8Bi_0_data [get_bd_ports disp_data] [get_bd_pins DisplayController8Bi_0/data]
  connect_bd_net -net DisplayController8Bi_0_dc [get_bd_ports disp_dc] [get_bd_pins DisplayController8Bi_0/dc]
  connect_bd_net -net DisplayController8Bi_0_rd [get_bd_ports disp_rd] [get_bd_pins DisplayController8Bi_0/rd]
  connect_bd_net -net DisplayController8Bi_0_rst [get_bd_ports disp_rst] [get_bd_pins DisplayController8Bi_0/rst]
  connect_bd_net -net DisplayController8Bi_0_wr [get_bd_ports disp_wr] [get_bd_pins DisplayController8Bi_0/wr]
  connect_bd_net -net HyperRamWrapper_0_hr_ck_o [get_bd_ports hr_ck] [get_bd_pins HyperRamWrapper_0/hr_ck_o]
  connect_bd_net -net HyperRamWrapper_0_hr_csn_o [get_bd_ports hr_cs_l] [get_bd_pins HyperRamWrapper_0/hr_csn_o]
  connect_bd_net -net HyperRamWrapper_0_hr_resetn_o [get_bd_ports hr_rst_l] [get_bd_pins HyperRamWrapper_0/hr_resetn_o]
  connect_bd_net -net Net [get_bd_ports hr_rwds] [get_bd_pins HyperRamWrapper_0/hr_rwds_io]
  connect_bd_net -net Net1 [get_bd_ports hr_dq] [get_bd_pins HyperRamWrapper_0/hr_dq_io]
  connect_bd_net -net Serial2AXIS_0_serial_cts [get_bd_ports serial_cts] [get_bd_pins Serial2AXIS_0/serial_cts]
  connect_bd_net -net Serial2AXIS_0_serial_miso [get_bd_ports serial_miso] [get_bd_pins Serial2AXIS_0/serial_miso]
  connect_bd_net -net clk_100m_pin_1 [get_bd_ports clk_100m_pin] [get_bd_pins clk_wiz_0/clk_in1]
  connect_bd_net -net clk_wiz_0_clk_out1 [get_bd_pins DisplayController8Bi_0/aclk] [get_bd_pins HyperRamWrapper_0/aclk] [get_bd_pins RasterIXSystem_0/aclk] [get_bd_pins Serial2AXIS_0/aclk] [get_bd_pins clk_wiz_0/clk_out1] [get_bd_pins proc_sys_reset_0/slowest_sync_clk]
  connect_bd_net -net clk_wiz_0_clk_out2 [get_bd_pins HyperRamWrapper_0/aclk90degree] [get_bd_pins clk_wiz_0/clk_out2]
  connect_bd_net -net clk_wiz_0_clk_out3 [get_bd_pins HyperRamWrapper_0/delay_refclk_i] [get_bd_pins clk_wiz_0/clk_out3]
  connect_bd_net -net clk_wiz_0_locked [get_bd_pins clk_wiz_0/locked] [get_bd_pins proc_sys_reset_0/dcm_locked]
  connect_bd_net -net proc_sys_reset_0_mb_reset [get_bd_pins DisplayController8Bi_0/resetn] [get_bd_pins HyperRamWrapper_0/resetn] [get_bd_pins RasterIXSystem_0/resetn] [get_bd_pins Serial2AXIS_0/resetn] [get_bd_pins proc_sys_reset_0/peripheral_aresetn]
  connect_bd_net -net reset_1 [get_bd_ports reset] [get_bd_pins proc_sys_reset_0/ext_reset_in]
  connect_bd_net -net serial_mosi_1 [get_bd_ports serial_mosi] [get_bd_pins Serial2AXIS_0/serial_mosi]
  connect_bd_net -net serial_ncs_1 [get_bd_ports serial_ncs] [get_bd_pins Serial2AXIS_0/serial_ncs]
  connect_bd_net -net serial_sck_1 [get_bd_ports serial_sck] [get_bd_pins Serial2AXIS_0/serial_sck]

  # Create address segments
  assign_bd_address -offset 0x00000000 -range 0x000100000000 -target_address_space [get_bd_addr_spaces RasterIXSystem_0/m_axi] [get_bd_addr_segs HyperRamWrapper_0/s_axi/reg0] -force


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


