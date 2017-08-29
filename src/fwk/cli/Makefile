# Build dynamic library by default
DYNAMIC_LIB ?= 0
# Build static library by default
STATIC_LIB ?= 1
# Run tests by default
TESTS ?= 1

UNAME = $(shell sh -c 'uname -s 2>/dev/null || echo not')
DESTDIR =
PREFIX = /usr/local

MAJOR = 1
MINOR = 9
REVISION = 7
LIB = libcli.so
LIB_STATIC = libcli.a
CLI = cli.so
CLI_STATIC = cli.a

CC = gcc
AR = ar
ARFLAGS = rcs
DEBUG = -g
OPTIM = -O3
override CFLAGS += $(DEBUG) $(OPTIM) -Wall -std=c99 -pedantic -Wformat-security -Wno-format-zero-length -Werror -Wwrite-strings -Wformat -Wextra -Wsign-compare -Wcast-align -Wno-unused-parameter
override LDFLAGS += -shared
override LIBPATH += -L.

ifeq ($(UNAME),Darwin)
override LDFLAGS += -Wl,-install_name,$(LIB).$(MAJOR).$(MINOR)
else
override LDFLAGS += -Wl,-soname,$(LIB).$(MAJOR).$(MINOR)
LIBS = -lcrypt
endif

ifeq (1,$(DYNAMIC_LIB))
TARGET_LIBS += $(LIB)
endif
ifeq (1,$(STATIC_LIB))
TARGET_LIBS += $(LIB_STATIC)
endif

TARGET_LIBS += $(CLI)
TARGET_LIBS += $(CLI_STATIC)

all: $(TARGET_LIBS) $(if $(filter 1,$(TESTS)),clidemo)

$(LIB): libcli.o
	$(CC) -o $(LIB).$(MAJOR).$(MINOR).$(REVISION) $^ $(LDFLAGS) $(LIBS)
	-rm -f $(LIB) $(LIB).$(MAJOR).$(MINOR)
	ln -s $(LIB).$(MAJOR).$(MINOR).$(REVISION) $(LIB).$(MAJOR).$(MINOR)
	ln -s $(LIB).$(MAJOR).$(MINOR) $(LIB)

$(LIB_STATIC): libcli.o
	$(AR) $(ARFLAGS) $@ $^

$(CLI):
	gcc cli.c -fPIC -shared -o cli.so

$(CLI_STATIC): cli.o libcli.o
	$(AR) $(ARFLAGS) cli.a cli.o libcli.o

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -o $@ -c $<

libcli.o: ../../../include/fwk/cli/libcli.h

#clitest: clitest.o $(LIB)
#	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< -L. -lcli

# for dyn
#clidemo: clidemo.o $(LIB_MY_CLI)
#	gcc clidemo.c -L. -lmycli -lcli -o clidemo

clidemo: clidemo.c $(CLI_STATIC)
	gcc clidemo.c -o clidemo $(CLI_STATIC) -lcrypt -lpthread

	
#clitest.exe: clitest.c libcli.o
#	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< libcli.o -lws2_32

clean:
	rm -f *.o $(LIB)* $(LIB_STATIC) clitest clidemo cli.so cli.a

#install: $(TARGET_LIBS)
#	install -d $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(PREFIX)/lib
#	install -m 0644 libcli.h $(DESTDIR)$(PREFIX)/include
#  ifeq (1,$(STATIC_LIB))
#	install -m 0644 $(LIB_STATIC) $(DESTDIR)$(PREFIX)/lib
#  endif
#  ifeq (1,$(DYNAMIC_LIB))
#	install -m 0755 $(LIB).$(MAJOR).$(MINOR).$(REVISION) $(DESTDIR)$(PREFIX)/lib
#	cd $(DESTDIR)$(PREFIX)/lib && \
#	    ln -fs $(LIB).$(MAJOR).$(MINOR).$(REVISION) $(LIB).$(MAJOR).$(MINOR) && \
#	    ln -fs $(LIB).$(MAJOR).$(MINOR) $(LIB)
#  endif
#
#rpm:
#	mkdir libcli-$(MAJOR).$(MINOR).$(REVISION)
#	cp -R *.c *.h Makefile Doc README *.spec libcli-$(MAJOR).$(MINOR).$(REVISION)
#	tar zcvf libcli-$(MAJOR).$(MINOR).$(REVISION).tar.gz --exclude CVS --exclude *.tar.gz libcli-$(MAJOR).$(MINOR).$(REVISION)
#	rm -rf libcli-$(MAJOR).$(MINOR).$(REVISION)
#	rpm -ta libcli-$(MAJOR).$(MINOR).$(REVISION).tar.gz --clean
