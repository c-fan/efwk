#!/bin/sh
#This shell is a demo for compiling script.
#It need only to assign ARCH environment to identify 32 ot 64 bit OS,
#and CROSS_COMPLIE path and name, which can be specified in profile.

if [ "-h" = "$1" ]; then
	echo "Usage: $0 <architecture>"
	echo "architecture: arm|arm64|x86_64"
	exit
fi
export ARCH=$1

if [ "arm" = "$ARCH" ]; then
TOOL_CHAIN_PATH=/usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin
export PATH=$TOOL_CHAIN_PATH:$PATH
export CROSS_COMPILE=arm-linux-gnueabihf-

elif [ "arm64" = "$ARCH" ]; then
TOOL_CHAIN_PATH=/usr/local/arm/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin
export PATH=$TOOL_CHAIN_PATH:$PATH
export CROSS_COMPILE=aarch64-linux-gnu-

elif [ "x86_64" = "$ARCH" ]; then
echo "x86_64, no CROSS_COMPILE"

else
echo "Customized architecture."
fi

make
