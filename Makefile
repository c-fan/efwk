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
HOME = $(shell pwd)

AllDirs := $(shell find ./src -type d)
Sources := $(foreach n,$(AllDirs) , $(wildcard $(n)/*.c))
Objs := $(patsubst %.c,%.o, $(Sources))
Deps := $(patsubst %.c,%.d, $(Sources))
StaticLib = libefwks.a
DynamicLib = libefwk.so
AllLibs = $(StaticLib) $(DynamicLib)

INCLUDE = -I$(HOME)/include
CFLAGS += -g -O0 -Wall -D_GNU_SOURCE $(INCLUDE) -D$(PLT)
LDFLAGS = -L. -lefwks -lpthread -lcrypt

UtObj = ut/efwk-demo.o
TARGET = efwk-demo

.PHONY : all clean dll
dll: $(DynamicLib)

all: $(StaticLib) $(UtObj)
	$(CC) $(CFLAGS) -o $(TARGET) $(UtObj) $(LDFLAGS)
	#$(STRIP) $(TARGET)

clean:
	rm -f $(Objs) $(Deps) $(StaticLib) $(DynamicLib) $(UtObj) $(TARGET)

$(StaticLib) : $(Objs)
	$(AR) rcs $@ $^

$(DynamicLib) : $(Objs)
	$(CC) -fPIC -shared -o $@ $^

%.d : %.c
	$(CC) -MT"$(<:.c=.o) $@" -MM $(CFLAGS) $< > $@

%.o : %.c
	$(CC) $(CFLAGS) $(DLL_CFLAGS) -c $< -o $@

sinclude $(Deps)
