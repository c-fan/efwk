# bld/archdep-arm.mk, sub-make-file, target-arch dependent settings, for arm.
# Written by Yingyu YOU (yingyu.you@hsmoptics.com), 2018-04-19
# Copyright (c) 2018 Huaxin-SM Optics (Chengdu) Co., Ltd. All Rights Reserved.

#TOOL_CHAIN_PATH:=/opt/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin
TOOL_CHAIN_PATH:=/usr/local/arm/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin
export PATH:=$(TOOL_CHAIN_PATH):$(PATH)
CROSS_COMPILE:=aarch64-linux-gnu-
ifeq ($(VERBOSE),1)
    $(info PATH=$(PATH))
    $(info CROSS_COMPILE=$(CROSS_COMPILE))
endif
