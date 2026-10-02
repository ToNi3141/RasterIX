- [Arty Z7-20 Build](#arty-z7-20-build)
- [Library Build](#library-build)
  - [Preparation SD Card](#preparation-sd-card)
  - [Build Petalinux](#build-petalinux)
  - [Build Kernel Driver](#build-kernel-driver)
  - [Build SDK](#build-sdk)
  - [Build Examples](#build-examples)
    - [glX](#glx)
  - [Reload FPGA content without rebuilding the File System](#reload-fpga-content-without-rebuilding-the-file-system)


# Arty Z7-20 Build
The build target is an Arty Z7-20 board with an `XC7Z020` SoC. It expects petalinux running on the board. It will by default output a 1024x600px video signal on the HDMI OUT. Just connect there a monitor which can handle this resolution.

<img src="https://github.com/user-attachments/assets/2481e940-0bcb-4b07-8bb8-f84af454f469" width="70%"> 


There are two variants available:

`rixif`:  
  - 1 TMU (max res: 256x256)
  - Mip mapping
  - Framebuffer size: 128kB + 128kB + 32 kB
  - 25 bit fix point

`rixef`:
  - 2 TMU (max res: 256x256)
  - Mip mapping
  - 25 bit fix point

To build the binaries use the following commands.
```sh
cd rtl/top/Xilinx/ArtyZ7-20
/Xilinx/Vivado/2022.2/bin/vivado -mode batch -source build.tcl -tclargs rixif
```
You will find `rasterix.bin` and `rasterix.bit` in the synth directory. You will also find there the `design_1_wrapper.xsa` file which is used for petalinux.

# Library Build
The library build expects a petalinux SDK. Before starting to build, create a petalinux distribution and use the SDK for this build.

## Preparation SD Card
Use the following layout:
```
> fdisk -l /dev/sdc
Disk /dev/sdc: 29.72 GiB, 31914983424 bytes, 62333952 sectors
Disk model: USB  SD Reader  
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0xafa8e6ad

Device     Boot   Start      End  Sectors  Size Id Type
/dev/sdc1          8192  1056767  1048576  512M  b W95 FAT32
/dev/sdc2       1056768 62332927 61276160 29.2G 83 Linux

```
Use for `sdc1` the following label: `BOOT`

Use for `sdc2` the following label: `rootfs`

## Build Petalinux
The following steps will give you a hint how to build and install petalinux. As precondition you must have build the hardware and have installed the petalinux tools.
```sh
# Set environment variables (if not already done)
source /opt/pkg/petalinux/settings.sh

# Create project
petalinux-create --type project --template zynq --name artyZ7_os_rix
cd artyZ7_os_rix

# Configure petalinux
petalinux-config --get-hw-description '/home/<username>/RasterIX/rtl/top/Xilinx/ArtyZ7-20/synth'
# Now set the following configuration:
#   Image Packaging Configuration --> Root filesystem type (EXT4 (SD/eMMC/SATA/USB))

# Copy the pre configured device tree overlay
cp /home/<username>/RasterIX/lib/driver/dmaproxy/kernel/system-user.dtsi project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi

# Build
petalinux-build -c fsbl-firmware -x cleansstate # important for rebuilding
petalinux-build
petalinux-package --boot --fsbl ./images/linux/zynq_fsbl.elf --fpga ./images/linux/system.bit --u-boot --force

# Copy the files to your SD card
cp ./images/linux/BOOT.BIN ./images/linux/boot.scr ./images/linux/image.ub /media/BOOT
sudo tar -xzf ./images/linux/rootfs.tar.gz -C /media/rootfs
```
Now you can plug the SD card into your ArtyZ7. It should now boot. If not, please refer the official documentation.

## Build Kernel Driver
The Zynq build requires a kernel driver to transfer data via DMA to the renderer. You can find the sources for the kernel driver in `lib/driver/dmaproxy/kernel`. Use petalinux to create a kernel driver and use the sources in this directory to build the kernel driver. This driver is a modification of Xilinx's dma-proxy driver. This directory also contains the device tree overlay which contains memory allocations for the graphics memory and entries for the dma proxy.
```sh
# Create a symbolic link of the dmaproxy driver into the petalinux modules
ln -s '/home/<username>/RasterIX/lib/driver/dmaproxy/kernel/dma-proxy' '/home/<username>/ZynqRasterIX/artyZ7_os_rix/project-spec/meta-user/recipes-modules/'

# Build the kernel module
petalinux-build -c dma-proxy 

# Copy it to your target
scp ./build/tmp/sysroots-components/zynq_generic/dma-proxy/lib/modules/5.15.36-xilinx-v2022.2/extra/dma-proxy.ko petalinux@192.168.2.120:/home/petalinux/
```
Load the driver on the target with `sudo insmod dma-proxy.ko`.

## Build SDK
The SDK is used to build examples.
```sh
# Build the petalinux SDK
petalinux-build --sdk

# Install the SDK (for instance to /opt/petalinux/2022.2/)
sh images/linux/sdk.sh
```

## Build Examples
Open a terminal. Use the following commands:
```sh
cd <rasterix_directory>
export SYSROOTS=/opt/petalinux/2022.2/sysroots
cmake --preset zynq_embedded_linux_rixif
cmake --build build/zynq --config Release --parallel
```
Now you can copy the binaries in `build/zynq/example` to your target (for instance via `scp`) and execute them. You should now see on your screen the renderings.

### glX
It also builds the `libGL.so` library, which implements a very basic glX API and exposes the OpenGL API. The current implementation of the glX API should be sufficient to play games like Quake3e without modifications, or to use it with SDL.

## Reload FPGA content without rebuilding the File System
To reload the FPGA content, copy the bin file (`scp synth/rasterix.bin petalinux@192.168.2.120:/home/petalinux/`) in the synth directory onto your target. Then use the following commands on the target to load the new bit stream.
```sh
sudo rmmod dma-proxy.ko
sudo fpgautil -b rasterix.bin -f Full
sudo insmod dma-proxy.ko
```