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
EXTRA_WARNINGS := -Wextra -pedantic -Wpointer-arith -Wwrite-strings \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wstrict-prototypes -Wcast-align

#            -Wshadow -Wconversion -Wmissing-prototypes -Wmissing-declarations \

CFLAGS += -g -O0 -Wall $(EXTRA_WARNINGS) -MMD -MP -D_GNU_SOURCE $(INCLUDE)
CFLAGS += -fasynchronous-unwind-tables
LDFLAGS = -L$(LibDir) -lefwks -lpthread -lcrypt

_UtSrcs:=efwk-demo.c stw_timer_race_condtion.c
UtSrcs:=$(_UtSrcs:%=ut/%)
UtObj:=$(StaticObjDir)/$(UtSrcs:%.c=%.o)
DemoExes:=$(_UtSrcs:%.c=$(BinDir)/%)

reverse = $(if $(wordlist 2,2,$(1)),$(call reverse,$(wordlist 2,$(words $(1)),$(1))) $(firstword $(1)),$(1))

.SECONDEXPANSION:
.PHONY : all clean dll stlib demoexe create_static_obj_dir create_shared_obj_dir create_lib_dir create_bin_dir

all: stlib dll demoexe

dll: $(DynamicLib)

stlib: $(StaticLib)

demoexe: $(DemoExes)

create_static_obj_dir:
	@mkdir -p $(StaticObjDir)

create_shared_obj_dir:
	@mkdir -p $(SharedObjDir)

create_lib_dir:
	@mkdir -p $(LibDir)

create_bin_dir:
	@mkdir -p $(BinDir)

$(BinDir)/%: $(StaticObjDir)/ut/%.o $(StaticLib) | create_bin_dir
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	$(STRIP) -g $@

clean:
	$(RM) $(StaticObjs) $(SharedObjs) $(StaticObjs:%.o=%.d) $(SharedObjs:%.o=%.d) $(StaticLib) $(DynamicLib) $(UtObj) $(UtObj:%.o=%.d) $(DemoExes)
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
$(foreach src,$(UtSrcs),$(eval $(call COMPILE_SOURCE,$(subst ut,$(StaticObjDir)/ut,$(src:%.c=%.o)),$(src))))

sinclude $(Deps)
