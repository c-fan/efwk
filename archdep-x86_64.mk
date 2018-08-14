# bld/archdep-x86_64.mk, sub-make-file, target-arch dependent settings, for x86_64.
# Written by Yingyu YOU (yingyu.you@hsmoptics.com), 2018-04-19
# Copyright (c) 2018 Huaxin-SM Optics (Chengdu) Co., Ltd. All Rights Reserved.

# Since x86_64 is used as host, cross-compiling is unnecessary.
ifeq ($(VERBOSE),1)
    $(info PATH=$(PATH))
    $(info CROSS_COMPILE=$(CROSS_COMPILE))
endif
