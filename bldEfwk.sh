#!/bin/sh
GitRoot=/home/$USER/git

if [ "-h" = "$1" ]; then
	echo "Usage: $0 <DMC|MFH> [rebuild | ]"
	echo "DMC: Small-OTN DMC-1703 product."
	echo "MFH: Mobile Forword Housekeeping product."
	echo "rebuild: clean all and rebuild."
	exit
fi

export PLT=$1
if [ "DMC" = "$PLT" ] ; then
	export GCC_HOME=/usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf
	export INC_GCC_CPP=${GCC_HOME}/arm-linux-gnueabihf/include/c++/6.2.1
	export INC_GCC_LIBC=${GCC_HOME}/arm-linux-gnueabihf/libc/usr/include
	export PATH=${GCC_HOME}/bin:${PATH}
	export CROSS_COMPILE=arm-linux-gnueabihf-
	export ARCH=arm

elif [ "MFH" = "$PLT" ] ; then
	GCC_HOME=/usr/local/arm/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu
	export INC_GCC_CPP=${GCC_HOME}/aarch64-linux-gnu/include/c++/6.3.1
	export INC_GCC_LIBC=${GCC_HOME}/aarch64-linux-gnu/libc/usr/include
	export PATH=${GCC_HOME}/bin:${PATH}
	export CROSS_COMPILE=aarch64-linux-gnu-
	export ARCH=arm64

elif [ "HOST" = "$PLT" ] ; then
	export INC_GCC_CPP=/usr/include/c++/5
	export INC_GCC_LIBC=/usr/lib/gcc/x86_64-linux-gnu/5
	export PATH=/usr/bin:${PATH}
	export CROSS_COMPILE=
	export ARCH=x86_64

else
	echo "Unsupported platform: $PLT"
exit
fi

#BLD_OPT="CFLAGS=-DHSMO=$PLT"
export PLT_HOME=$GitRoot/${PLT}_PLT
export OS_HOME=$GitRoot/linux-4.9
PubDir=$PLT_HOME
EFWK_HOME=$GitRoot/Tools/efwk
cd $EFWK_HOME

# Build dynamic link library
dllFile=$EFWK_HOME/libefwk.so
rm -f $dllFile
make clean
make dll "DLL_CFLAGS=-fPIC"
if [ -f $dllFile ] ; then
	mv -f $dllFile $PubDir/lib/
else
	echo "Failed to generate dynamic link library."
	exit
fi

# Build static library and test suite
FlagFile=$EFWK_HOME/libefwks.a
rm -f $FlagFile
make clean
make all
if [ -f $FlagFile ] ; then
	#Release generated targets
	cp -rf $EFWK_HOME/include/fwk $PubDir/inc/
	cp -f $EFWK_HOME/efwk-demo $PubDir/bin/
	cp -f $FlagFile $PubDir/lib/
	echo "Build finished."
fi
