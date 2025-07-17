PROJ = Rasterizer

all: \
	dmaStreamEngine  \
	simulationILI9486 \
	simulationILI9341 \
	attributeInterpolator \
	functionInterpolator \
	textureSampler \
	colorInterpolator \
	texEnv \
	stencilOp \
	memoryReadRequestGenerator \
	framebufferSerializer \
	streamConcatFifo \
	framebufferWriter \
	framebufferWriterStrobeGen \
	framebufferWriterClear \
	framebufferReader \
	lodCalculator \
	attributeInterpolationX \
	attributePerspectiveCorrectionX \
	triangleStreamF2XConverter \
	pagedMemoryReader \
	internalFramebufferCommandHandler
 
clean:
	rm -rf obj_dir

dmaStreamEngine:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/DmaStreamEngine.v --top-module DmaStreamEngine cpp/sim_DmaStreamEngine.cpp -I../rtl/RasterIX/
	-make -C obj_dir -f VDmaStreamEngine.mk
	./obj_dir/VDmaStreamEngine

simulationILI9486:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Display/DisplayController8BitILI9486.v --top-module DisplayController8BitILI9486 cpp/sim_DisplayController8BitILI9486.cpp
	-make -C obj_dir -f VDisplayController8BitILI9486.mk
	./obj_dir/VDisplayController8BitILI9486

simulationILI9341:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Display/DisplayController8BitILI9341.v --top-module DisplayController8BitILI9341 cpp/sim_DisplayController8BitILI9341.cpp
	-make -C obj_dir -f VDisplayController8BitILI9341.mk
	./obj_dir/VDisplayController8BitILI9341

attributeInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/AttributeInterpolator.v --top-module AttributeInterpolator cpp/sim_AttributeInterpolator.cpp -I../rtl/Float/rtl/float/ -I../rtl/RasterIX/
	-make -C obj_dir -f VAttributeInterpolator.mk
	./obj_dir/VAttributeInterpolator

functionInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/FunctionInterpolator.v --top-module FunctionInterpolator cpp/sim_FunctionInterpolator.cpp -I../rtl/RasterIX/
	-make -C obj_dir -f VFunctionInterpolator.mk
	./obj_dir/VFunctionInterpolator

textureSampler:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe v/TextureSamplerTestModule.v --top-module TextureSamplerTestModule cpp/sim_TextureSampler.cpp -I../rtl/RasterIX/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VTextureSamplerTestModule.mk
	./obj_dir/VTextureSamplerTestModule

colorInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/ColorInterpolator.v --top-module ColorInterpolator cpp/sim_ColorInterpolator.cpp -I../rtl/RasterIX/
	-make -C obj_dir -f VColorInterpolator.mk
	./obj_dir/VColorInterpolator

texEnv:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/TexEnv.v --top-module TexEnv cpp/sim_TexEnv.cpp -I../rtl/RasterIX/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VTexEnv.mk
	./obj_dir/VTexEnv

stencilOp:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/StencilOp.v --top-module StencilOp cpp/sim_StencilOp.cpp -I../rtl/RasterIX/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VStencilOp.mk
	./obj_dir/VStencilOp

memoryReadRequestGenerator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/MemoryReadRequestGenerator.v --top-module MemoryReadRequestGenerator cpp/sim_MemoryReadRequestGenerator.cpp -I../rtl/RasterIX/
	-make -C obj_dir -f VMemoryReadRequestGenerator.mk
	./obj_dir/VMemoryReadRequestGenerator

framebufferSerializer:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/FramebufferSerializer.v --top-module FramebufferSerializer cpp/sim_FramebufferSerializer.cpp -I../rtl/RasterIX/
	-make -C obj_dir -f VFramebufferSerializer.mk
	./obj_dir/VFramebufferSerializer

streamConcatFifo:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/StreamConcatFifo.v --top-module StreamConcatFifo cpp/sim_StreamConcatFifo.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/
	-make -C obj_dir -f VStreamConcatFifo.mk
	./obj_dir/VStreamConcatFifo

framebufferWriter:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/FramebufferWriter.v --top-module FramebufferWriter cpp/sim_FramebufferWriter.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/
	-make -C obj_dir -f VFramebufferWriter.mk
	./obj_dir/VFramebufferWriter

framebufferWriterStrobeGen:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/FramebufferWriterStrobeGen.v --top-module FramebufferWriterStrobeGen cpp/sim_FramebufferWriterStrobeGen.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/
	-make -C obj_dir -f VFramebufferWriterStrobeGen.mk
	./obj_dir/VFramebufferWriterStrobeGen

framebufferWriterClear:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/FramebufferWriterClear.v --top-module FramebufferWriterClear cpp/sim_FramebufferWriterClear.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/
	-make -C obj_dir -f VFramebufferWriterClear.mk
	./obj_dir/VFramebufferWriterClear

framebufferReader:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/FramebufferReader.v --top-module FramebufferReader cpp/sim_FramebufferReader.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ -I../rtl/3rdParty/verilog-axis
	-make -C obj_dir -f VFramebufferReader.mk
	./obj_dir/VFramebufferReader

lodCalculator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/LodCalculator.v --top-module LodCalculator cpp/sim_LodCalculator.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ 
	-make -C obj_dir -f VLodCalculator.mk
	./obj_dir/VLodCalculator

attributeInterpolationX:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/AttributeInterpolationX.v --top-module AttributeInterpolationX cpp/sim_AttributeInterpolationX.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VAttributeInterpolationX.mk
	./obj_dir/VAttributeInterpolationX

attributePerspectiveCorrectionX:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/AttributePerspectiveCorrectionX.v --top-module AttributePerspectiveCorrectionX cpp/sim_AttributePerspectiveCorrectionX.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VAttributePerspectiveCorrectionX.mk
	./obj_dir/VAttributePerspectiveCorrectionX

triangleStreamF2XConverter:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/TriangleStreamF2XConverter.v --top-module TriangleStreamF2XConverter cpp/sim_TriangleStreamF2XConverter.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VTriangleStreamF2XConverter.mk
	./obj_dir/VTriangleStreamF2XConverter

pagedMemoryReader:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/PagedMemoryReader.v --top-module PagedMemoryReader cpp/sim_PagedMemoryReader.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VPagedMemoryReader.mk
	./obj_dir/VPagedMemoryReader

internalFramebufferCommandHandler:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/InternalFramebufferCommandHandler.v --top-module InternalFramebufferCommandHandler cpp/sim_InternalFramebufferCommandHandler.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VInternalFramebufferCommandHandler.mk
	./obj_dir/VInternalFramebufferCommandHandler

axisToAxiAdapter:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/RasterIX/AxisToAxiAdapter.v --top-module AxisToAxiAdapter cpp/sim_AxisToAxiAdapter.cpp -I../rtl/RasterIX/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VAxisToAxiAdapter.mk
	./obj_dir/VAxisToAxiAdapter

.SECONDARY:
.PHONY: all clean
