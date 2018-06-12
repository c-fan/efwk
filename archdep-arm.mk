# bld/archdep-arm.mk, sub-make-file, target-arch dependent settings, for arm.
# Written by Yingyu YOU (yingyu.you@hsmoptics.com), 2018-04-19
# Copyright (c) 2018 Huaxin-SM Optics (Chengdu) Co., Ltd. All Rights Reserved.

TOOL_CHAIN_PATH:=/usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin
export PATH:=$(TOOL_CHAIN_PATH):$(PATH)
CROSS_COMPILE:=arm-linux-gnueabihf-
ifeq ($(VERBOSE),1)
    $(info PATH=$(PATH))
    $(info CROSS_COMPILE=$(CROSS_COMPILE))
endif
