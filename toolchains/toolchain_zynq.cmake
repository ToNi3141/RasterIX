set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7-a)

set(TOOLCHAIN_PREFIX $ENV{SYSROOTS}/x86_64-petalinux-linux/usr/bin/arm-xilinx-linux-gnueabi/arm-xilinx-linux-gnueabi-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_SYSROOT $ENV{SYSROOTS}/cortexa9t2hf-neon-xilinx-linux-gnueabi)
set(CMAKE_PREFIX_PATH ${CMAKE_SYSROOT}/usr/lib/cmake)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(COMPILER_FLAGS " -marm -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -save-temps -Ofast -ffast-math -fno-rtti -fno-stack-protector -Wall -Wstrict-aliasing -Wdeprecated-copy -Wtype-limits -fno-tree-vectorize -flto -fomit-frame-pointer -frename-registers -falign-functions=32 -falign-loops=32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}" CACHE STRING "" FORCE)
