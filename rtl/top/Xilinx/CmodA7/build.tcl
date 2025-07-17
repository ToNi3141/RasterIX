set SYNTH_OUT ./synth
set REPORT_PATH ./reports
set BOARD_FILE .srcs/sources_1/bd/design_1/design_1.bd
file delete -force $SYNTH_OUT
file mkdir $SYNTH_OUT

# Change to synth directory as build directory
cd $SYNTH_OUT

file mkdir $REPORT_PATH

# Setup project properties
set_part xc7a35tcpg236-1
set_property TARGET_LANGUAGE Verilog [current_project]
set_property BOARD_PART digilentinc.com:cmod_a7-35t:part0:1.2 [current_project]
set_property DEFAULT_LIB work [current_project]
set_property source_mgmt_mode All [current_project]

# Load verilog source
read_verilog ./../../../../RasterIX/AttributeF2XConverter.v
read_verilog ./../../../../RasterIX/AttributeInterpolationX.v
read_verilog ./../../../../RasterIX/AttributeInterpolator.v
read_verilog ./../../../../RasterIX/AttributeInterpolatorX.v
read_verilog ./../../../../RasterIX/AttributePerspectiveCorrectionX.v
read_verilog ./../../../../RasterIX/ColorBlender.v
read_verilog ./../../../../RasterIX/ColorInterpolator.v
read_verilog ./../../../../RasterIX/ColorMixerSigned.v
read_verilog ./../../../../RasterIX/ColorMixer.v
read_verilog ./../../../../RasterIX/CommandParser.v
read_verilog ./../../../../RasterIX/AxisFramebufferReader.v
read_verilog ./../../../../RasterIX/AxisToAxiAdapter.v
read_verilog ./../../../../RasterIX/DmaStreamEngine.v
read_verilog ./../../../../RasterIX/DualPortRam.v
read_verilog ./../../../../RasterIX/Fog.v
read_verilog ./../../../../RasterIX/FramebufferReader.v
read_verilog ./../../../../RasterIX/FramebufferSerializer.v
read_verilog ./../../../../RasterIX/InternalFramebuffer.v
read_verilog ./../../../../RasterIX/InternalFramebufferCommandHandler.v
read_verilog ./../../../../RasterIX/InternalFramebufferRam.v
read_verilog ./../../../../RasterIX/InternalFramebufferReader.v
read_verilog ./../../../../RasterIX/InternalFramebufferScissorFunc.vh
read_verilog ./../../../../RasterIX/InternalFramebufferWriter.v
read_verilog ./../../../../RasterIX/FramebufferWriterClear.v
read_verilog ./../../../../RasterIX/FramebufferWriterStrobeGen.v
read_verilog ./../../../../RasterIX/FramebufferWriter.v
read_verilog ./../../../../RasterIX/FunctionInterpolator.v
read_verilog ./../../../../RasterIX/LinearAddressGenerator.v
read_verilog ./../../../../RasterIX/LodCalculator.v
read_verilog ./../../../../RasterIX/MemoryReadRequestGenerator.v
read_verilog ./../../../../RasterIX/MipmapOptimizedRam.v
read_verilog ./../../../../RasterIX/PagedMemoryReader.v
read_verilog ./../../../../RasterIX/PerFragmentPipeline.v
read_verilog ./../../../../RasterIX/PixelPipeline.v
read_verilog ./../../../../RasterIX/PixelUtil.vh
read_verilog ./../../../../RasterIX/RasterIXCoreEF.v
read_verilog ./../../../../RasterIX/RasterIXCoreIF.v
read_verilog ./../../../../RasterIX/RasterIXRenderCore.v
read_verilog ./../../../../RasterIX/RasterizerCommands.vh
read_verilog ./../../../../RasterIX/Rasterizer.v
read_verilog ./../../../../RasterIX/RegisterAndDescriptorDefines.vh
read_verilog ./../../../../RasterIX/RegisterBank.v
read_verilog ./../../../../RasterIX/RasterIX_EF.v
read_verilog ./../../../../RasterIX/RasterIX_IF.v
read_verilog ./../../../../RasterIX/RasterIX.v
read_verilog ./../../../../RasterIX/StencilOp.v
read_verilog ./../../../../RasterIX/StreamConcatFifo.v
read_verilog ./../../../../RasterIX/StreamFramebuffer.v
read_verilog ./../../../../RasterIX/TestFunc.v
read_verilog ./../../../../RasterIX/TexEnv.v
read_verilog ./../../../../RasterIX/TextureBuffer.v
read_verilog ./../../../../RasterIX/TextureFilter.v
read_verilog ./../../../../RasterIX/TextureMappingUnit.v
read_verilog ./../../../../RasterIX/TextureSampler.v
read_verilog ./../../../../RasterIX/TriangleStreamF2XConverter.v
read_verilog ./../../../../RasterIX/TrueDualPortRam.v
read_verilog ./../../../../Float/rtl/float/XRecip.v
read_verilog ./../../../../Float/rtl/float/ComputeRecip.v
read_verilog ./../../../../Float/rtl/float/FindExponent.v
read_verilog ./../../../../Float/rtl/float/FloatAdd.v
read_verilog ./../../../../Float/rtl/float/FloatRecip.v
read_verilog ./../../../../Float/rtl/float/FloatFastRecip.v
read_verilog ./../../../../Float/rtl/float/FloatMul.v
read_verilog ./../../../../Float/rtl/float/FloatSub.v
read_verilog ./../../../../Float/rtl/float/FloatToInt.v
read_verilog ./../../../../Float/rtl/float/IntToFloat.v
read_verilog ./../../../../Float/rtl/float/ValueDelay.v
read_verilog ./../../../../Float/rtl/float/ValueTrack.v
read_verilog ./../../../../3rdParty/verilog-axis/axis_adapter.v
read_verilog ./../../../../3rdParty/verilog-axis/axis_broadcast.v
read_verilog ./../../../../3rdParty/verilog-axi/arbiter.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar_addr.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar_rd.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar_wr.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_adapter_rd.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_adapter.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_adapter_wr.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_ram.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_register_rd.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_register_wr.v
read_verilog ./../../../../3rdParty/verilog-axi/priority_encoder.v
read_verilog ./../../../../3rdParty/sfifo.v
read_verilog ./../../../../3rdParty/skidbuffer.v
read_verilog ./../../../../Display/DisplayController8BitILI9341.v
read_verilog ./../../../../Util/Serial2AXIS.v
read_verilog ./../../../../Util/SpiSlave.v
read_verilog ./../AsyncSramController.v
read_verilog ./../AsyncSramPhy.v
read_xdc ./../Cmod-A7-Master.xdc

source ../design_1.tcl

# Open board file
read_bd $BOARD_FILE
# Create wrapper for the board file and set the board file as top
make_wrapper -import -files [get_files $BOARD_FILE] -top

set_property synth_checkpoint_mode None [get_files $BOARD_FILE]
generate_target all [get_files $BOARD_FILE]


# Run synthesis and implementation
synth_design -top design_1_wrapper
opt_design
place_design
phys_opt_design
route_design
phys_opt_design

# Create reports
report_timing_summary -file $REPORT_PATH/post_route_timing_summary.rpt
report_timing -sort_by group -max_paths 100 -path_type summary -file $REPORT_PATH/post_route_timing.rpt
report_clock_utilization -file $REPORT_PATH/clock_util.rpt
report_utilization -file $REPORT_PATH/post_route_util.rpt
report_power -file $REPORT_PATH/post_route_power.rpt
report_drc -file $REPORT_PATH/post_imp_drc.rpt

# Write bit file 
write_bitstream -bin_file rasterix.bit
