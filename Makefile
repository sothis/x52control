SOURCES	+= ./src/x52entry.c
SOURCES	+= ./src/x52device.c
SOURCES	+= ./src/x52interface.c

# partial libusb
SOURCES	+= ./src/usb/usb.c
SOURCES	+= ./src/usb/linux.c
SOURCES	+= ./src/usb/error.c
SOURCES	+= ./src/usb/descriptors.c


INCLUDE := -I./include -I./src
DEFINES := -DLIN -DLIL -DPIC -DXPLM200


ifndef CONF
	CONF	:= debug
endif
ifeq ($(CONF), debug)
	DEBUG	:= Yes
endif
ifeq ($(CONF), debugo)
	DEBUGO	:= Yes
endif
ifeq ($(CONF), release)
	RELEASE	:= Yes
endif
ifeq ($(CONF), releases)
	RELEASE_STATIC	:= Yes
endif
OUTDIR		:= build
BUILDDIR	:= $(OUTDIR)/$(CONF)

LD	:= gcc
CC	:= gcc

ifdef DEBUG
CFLAGS	:= -m32 -O0 -g -std=c99 -Wall -fPIC -pipe -fvisibility=hidden
LDFLAGS	:= -m32 -shared
endif
ifdef DEBUGO
CFLAGS	:= -m32 -O3 -g -fstrict-aliasing -std=c99 -fPIC -fvisibility=hidden
CFLAGS	+= -pipe -Wall
LDFLAGS	:= -m32 -shared -Wl,-O1
endif
ifdef RELEASE
CFLAGS	:= -m32 -O3 -fomit-frame-pointer -fstrict-aliasing -std=c99
CFLAGS	+= -fPIC -pipe -fvisibility=hidden
LDFLAGS	:= -m32 -shared -Wl,-O1,--strip-all
endif
ifdef RELEASE_STATIC
CFLAGS	:= -m32 -O3 -fomit-frame-pointer -fstrict-aliasing -std=c99
CFLAGS	+= -fPIC -pipe -fvisibility=hidden
LDFLAGS	:= -m32 -shared -static-libgcc -Wl,-Bstatic,-O1,--strip-all
endif


DEPS	:= $(patsubst %.c, $(BUILDDIR)/obj/%.cdep, $(SOURCES))
OBJECTS	:= $(patsubst %.c, $(BUILDDIR)/obj/%.o, $(SOURCES))

print_cc	:= echo [ CC ]
print_so	:= echo [ SO ]
print_so	:= echo [ LD ]
print_memc	:= echo [ MEMCHECK ]
print_prf	:= echo [ PROFILE  ]
print_error	:= (echo [ FAILED ] && false)

.PHONY: all clean debug debug-opt release release-static x52control test
.PHONY: memcheck

all: release

debug:
	@$(MAKE) CONF=debug -s -C . x52control test
debug-opt:
	@$(MAKE) CONF=debugo -s -C . x52control test
release:
	@$(MAKE) CONF=release -s -C . x52control
release-static:
	@$(MAKE) CONF=releases -s -C . x52control

memcheck: debug-opt
# valgrind >= 3.5 supports non-zero exit code on leak occurances
	@$(print_memc)
	@valgrind --leak-check=full --error-exitcode=1 --show-reachable=yes \
	$(OUTDIR)/debugo/test 2> $(OUTDIR)/debugo/test.mem 1> /dev/null || \
	$(print_error)

profile: debug-opt
	@$(print_prf)
	@valgrind --tool=callgrind --error-exitcode=1 \
	--callgrind-out-file=$(OUTDIR)/debugo/test.cgr $(OUTDIR)/debugo/test \
	2> $(OUTDIR)/debugo/test.prf 1> /dev/null || $(print_error)

x52control: $(BUILDDIR)/lin.xpl
test:  $(BUILDDIR)/test

$(BUILDDIR)/lin.xpl: $(OBJECTS)
	@$(print_so) $(@)
	@-mkdir -p $(dir $(@))
	@$(LD) -Wl,-soname,$(notdir $(@)) $(LDFLAGS) -o $@ $(OBJECTS) || \
	$(print_error)

$(BUILDDIR)/test: $(BUILDDIR)/lin.xpl $(BUILDDIR)/obj/./test/main.o
	@$(print_so) $(@)
	@-mkdir -p $(dir $(@))
	@$(LD) -m32 -Wl,-rpath='$$ORIGIN',-z,origin -o $@ $(BUILDDIR)/lin.xpl \
	$(BUILDDIR)/obj/./test/main.o || $(print_error)

$(BUILDDIR)/obj/%.o: %.c
	@$(print_cc) $(<)
	@-mkdir -p $(dir $(@))
	@$(CC) $(CFLAGS) $(INCLUDE) $(DEFINES) -c -o$(@) $(<) || $(print_error)
	@$(CC) $(CFLAGS) $(INCLUDE) $(DEFINES) -MM -MT $(@) -MT $(@:.o=.cdep) \
	-o $(@:.o=.cdep) $(<) || $(print_error)

clean:
	@echo cleaning tree
	@-rm -rf $(OUTDIR)

-include $(DEPS)

