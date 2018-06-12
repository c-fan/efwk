AS  = $(CROSS_COMPILE)as
LD  = $(CROSS_COMPILE)ld
CC  = $(CROSS_COMPILE)gcc
CPP = $(CC) -E
AR  = $(CROSS_COMPILE)ar
NM  = $(CROSS_COMPILE)nm

STRIP    = $(CROSS_COMPILE)strip
OBJCOPY  = $(CROSS_COMPILE)objcopy
OBJDUMP  = $(CROSS_COMPILE)objdump

SHELL = /bin/bash

SUPPORTED_ARCHS:=x86 arm64 arm
ifneq (,$(filter $(ARCH),$(SUPPORTED_ARCHS)))
    include $(SELF_DIR)archdep-$(ARCH).mk
else
    $(error Unsupported ARCH '$(ARCH)' provided, SUPPORTED_ARCHS: $(SUPPORTED_ARCHS).)
endif

SUPPORTED_PLTS:=DMC MFH APB HOST
ifneq (,$(filter $(PLT),$(SUPPORTED_PLTS)))
    include $(SELF_DIR)pltdep-$(PLT).mk
else
    $(error Unsupported PLT '$(PLT)' provided, SUPPORTED_PLTS: $(SUPPORTED_PLTS).)
endif

AllDirs := $(shell find src -type d)
Sources := $(foreach n,$(AllDirs) , $(wildcard $(n)/*.c))
StaticObjDir := obj-$(ARCH)-static
SharedObjDir := obj-$(ARCH)-shared
LibDir := lib-$(ARCH)
BinDir := bin-$(ARCH)
StaticObjs := $(patsubst src/%.c,$(StaticObjDir)/%.o, $(Sources))
SharedObjs := $(patsubst src/%.c,$(SharedObjDir)/%.o, $(Sources))
Deps := $(patsubst src/%.c,$(ObjDir)/%.d, $(Sources))
StaticLib = $(LibDir)/libefwks.a
DynamicLib = $(LibDir)/libefwk.so
AllLibs = $(StaticLib) $(DynamicLib)

INCLUDE = -Iinclude
CFLAGS += -g -O0 -Wall -MMD -MP -D_GNU_SOURCE $(INCLUDE) -D$(PLT)
LDFLAGS = -L$(LibDir) -lefwks -lpthread -lcrypt

UtSrc:=ut/efwk-demo.c
UtObj = $(StaticObjDir)/$(UtSrc:%.c=%.o)
DemoExe = $(BinDir)/efwk-demo

reverse = $(if $(wordlist 2,2,$(1)),$(call reverse,$(wordlist 2,$(words $(1)),$(1))) $(firstword $(1)),$(1))

.SECONDEXPANSION:
.PHONY : all clean dll stlib demoexe create_static_obj_dir create_shared_obj_dir create_lib_dir create_bin_dir

all: stlib dll demoexe

dll: $(DynamicLib)

stlib: $(StaticLib)

demoexe: $(DemoExe)

create_static_obj_dir:
	@mkdir -p $(StaticObjDir)

create_shared_obj_dir:
	@mkdir -p $(SharedObjDir)

create_lib_dir:
	@mkdir -p $(LibDir)

create_bin_dir:
	@mkdir -p $(BinDir)

$(DemoExe): $(StaticLib) $(UtObj) | create_bin_dir
	$(CC) $(CFLAGS) -o $(DemoExe) $(UtObj) $(LDFLAGS)
	#$(STRIP) $(DemoExe)

clean:
	$(RM) $(StaticObjs) $(SharedObjs) $(StaticObjs:%.o=%.d) $(SharedObjs:%.o=%.d) $(StaticLib) $(DynamicLib) $(UtObj) $(UtObj:%.o=%.d) $(DemoExe)
	-@rmdir $(StaticObjDir)/ut
	-@rmdir $(call reverse,$(filter-out src,$(AllDirs:src/%=$(StaticObjDir)/%)))
	-@rmdir $(call reverse,$(filter-out src,$(AllDirs:src/%=$(SharedObjDir)/%)))
#	rmdir -p --ignore-fail-on-non-empty $(filter-out src,$(AllDirs:src/%=$(ObjDir)/%))
	-@rmdir $(LibDir) $(StaticObjDir) $(SharedObjDir) $(BinDir)

$(StaticLib) : $(StaticObjs) | create_lib_dir
	$(AR) rcs $@ $^

$(DynamicLib) : DLL_CFLAGS=-fPIC
$(DynamicLib) : $(SharedObjs) | create_lib_dir
	$(CC) -fPIC -shared -o $@ $^

#$(ObjDir)/%.d : src/%.c ThisObjDir=$(dir $@)
#	mkdir -p $(ThisObjDir)
#	$(CC) -MT"$(<:.c=$(ObjDir)/.o) $@" -MM $(CFLAGS) $< > $@

$(StaticObjs): ObjDir=$(StaticObjDir)
$(SharedObjs): ObjDir=$(SharedObjDir)
$(SharedObjs): create_shared_obj_dir
$(StaticObjs): create_static_obj_dir
$(SharedObjs): create_shared_obj_dir

define COMPILE_SOURCE
$(1) : $(2)
#	$(info Make rule for $(1) from $(2))
	@mkdir -p $(dir $(1))
	$$(CC) $$(CFLAGS) $$(DLL_CFLAGS) -c $(2) -o $(1)
endef

$(foreach src,$(Sources),$(eval $(call COMPILE_SOURCE,$(subst src,$(StaticObjDir),$(src:%.c=%.o)),$(src))))
$(foreach src,$(Sources),$(eval $(call COMPILE_SOURCE,$(subst src,$(SharedObjDir),$(src:%.c=%.o)),$(src))))
$(foreach src,$(UtSrc),$(eval $(call COMPILE_SOURCE,$(subst ut,$(StaticObjDir)/ut,$(src:%.c=%.o)),$(src))))

sinclude $(Deps)
