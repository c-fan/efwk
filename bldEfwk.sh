#!/bin/sh

ARCH=arm PLT=DMC make clean
ARCH=arm PLT=DMC make

ARCH=arm64 PLT=APB make clean
ARCH=arm64 PLT=APB make

ARCH=x86 PLT=HOST make clean
ARCH=x86 PLT=HOST make
