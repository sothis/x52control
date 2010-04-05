VERSION := $(shell ./version)
ifeq ($(shell uname), Linux)
	PLAT_LINUX	:= Yes
	PLATFORM	:= linux
else ifeq ($(shell uname), Darwin)
	PLAT_DARWIN	:= Yes
	PLATFORM	:= darwin
endif
################################################################################

### x52control
SRC		+= ./src/main.cpp
SRC		+= ./src/x52session.cpp
SRC		+= ./src/x52tools.cpp
SRC		+= ./src/x52data.cpp
SRC		+= ./src/x52out.cpp
SRC		+= ./src/x52in.cpp
SRC		+= ./src/x52mfdpage.cpp
SRC		+= ./src/x52time.cpp
SRC		+= ./src/usb/usb.c
SRC		+= ./src/usb/$(PLATFORM).c
SRC		+= ./src/usb/descriptors.c
SRC		+= ./src/usb/error.c



################################################################################

ifeq ($(CONF), debug)
	DEBUG	:= Yes
endif
ifeq ($(CONF), release)
	RELEASE	:= Yes
endif
ifeq ($(CONF), release_dist)
	RELEASE	:= Yes
	DIST	:= Yes
endif

# preprocessor definitions
ifdef RELEASE
DEFINES		+= -DNDEBUG
endif
DEFINES		+= -D_GNU_SOURCE=1
DEFINES		+= -D_FILE_OFFSET_BITS=64
DEFINES		+= -D_LARGEFILE64_SOURCE=1
DEFINES		+= -D_LARGEFILE_SOURCE=1
DEFINES		+= -D_BSD_SOURCE=1
DEFINES		+= -D$(PLATFORM)=1
DEFINES		+= -DVERSION='"$(VERSION)"'


# toolchain configuration
INCLUDES	+= -I./src

OUTDIR		:= ./build
BUILDDIR	:= $(OUTDIR)/$(CONF)

# common flags
CFLAGS		:= -pipe -Wall -g -O3 -fPIC
ifdef PLAT_LINUX
CFLAGS		+= -m32
endif
ifdef PLAT_DARWIN
CFLAGS		+= -mmacosx-version-min=10.4
CFLAGS		+= -isysroot /Developer/SDKs/MacOSX10.4u.sdk
endif

# optimization flags
ifdef RELEASE
CFLAGS		+= -fvisibility=hidden
CFLAGS		+= -fomit-frame-pointer
CFLAGS		+= -fstrict-aliasing
ifdef PLAT_DARWIN
ifdef DIST
CFLAGS		+= -arch i386 -arch ppc
endif
endif
endif #RELEASE
CXXFLAGS	:= $(CFLAGS)

# language dependent flags
CFLAGS		+= -std=c99
ifdef RELEASE
CXXFLAGS	+= -fvisibility-inlines-hidden
endif

LDFLAGS		:= $(CFLAGS) -static-libgcc
ifdef PLAT_LINUX
LDFLAGS		+= -shared
ARFLAGS		:= cru
endif

ifdef PLAT_DARWIN
LDFLAGS		+= -bundle -undefined dynamic_lookup
ARFLAGS		:= -static -o
endif


# determine intermediate object filenames
C_SRC		:= $(filter %.c, $(SRC))
CXX_SRC		:= $(filter %.cpp, $(SRC))

DEPS		:= $(patsubst %.c, $(BUILDDIR)/.obj/%_C.dep, $(C_SRC))
DEPS		+= $(patsubst %.cpp, $(BUILDDIR)/.obj/%_CXX.dep, $(CXX_SRC))

OBJECTS		:= $(patsubst %.c, $(BUILDDIR)/.obj/%_C.o, $(C_SRC))
OBJECTS		+= $(patsubst %.cpp, $(BUILDDIR)/.obj/%_CXX.o, $(CXX_SRC))
BINOBJ		:= $(filter-out $(BUILDDIR)/.obj/./libx52/%.o, $(OBJECTS))
LIBOBJ		:= $(filter $(BUILDDIR)/.obj/./libx52/%.o, $(OBJECTS))


# tools
ifdef PLAT_DARWIN
gcc_prefix	:= -4.0
endif
CC		:= gcc$(gcc_prefix)
CXX		:= g++$(gcc_prefix)
DEFINES		+= -DAPL -DPIC -DXPLM200
ifeq ($(CXX_SRC),)
LD		:= gcc$(gcc_prefix)
else
LD		:= g++$(gcc_prefix)
endif
ifdef PLAT_DARWIN
AR		:= libtool
else
AR		:= ar
DEFINES		+= -DLIN -DPIC -DXPLM200
endif

print_ar	:= echo $(eflags) "AR "
print_tar	:= echo $(eflags) "TAR"
print_ld	:= echo $(eflags) "LD "
print_cc	:= echo $(eflags) "CC "
print_cxx	:= echo $(eflags) "CXX"


# targets
all: release

help:
	@echo "following make targets are available:"
	@echo "  help        - print this"
	@echo "  release     - build an optimized release version (*)"
	@echo "  debug       - build a debug version"
	@echo "  dist        - build a version suitable for distribution,"
	@echo "                on macos x this will create an universal binary"
	@echo "  clean       - recursively delete the output directory" \
		"'$(OUTDIR)'"
	@echo ""
	@echo "(*) denotes the default target if none or 'all' is specified"

debug:
	@$(MAKE) CONF=debug -C . all-recursive
release:
	@$(MAKE) CONF=release -C . all-recursive
dist:
	@$(MAKE) CONF=release_dist -C . dist-recursive

clean:
	@echo "deleting '$(OUTDIR)'"
	@-rm -rf $(OUTDIR)

dist-recursive: $(OUTDIR)/dist/x52control-$(VERSION)-$(PLATFORM).tar.bz2 \
$(OUTDIR)/dist/x52control-$(VERSION).tar.bz2

all-recursive: $(BUILDDIR)/x52control.xpl

$(OUTDIR)/dist/x52control-$(VERSION)-$(PLATFORM).tar.bz2: \
$(BUILDDIR)/x52control.xpl
	@-mkdir -p $(dir $(@))
	@$(print_tar) $(subst $(PWD)/, ./, $(abspath $(@)))
	@tar -C $(BUILDDIR) -cjf $(@) x52control.xpl

$(OUTDIR)/dist/x52control-$(VERSION).tar.bz2:
	@-mkdir -p $(dir $(@))
	@$(print_tar) $(subst $(PWD)/, ./, $(abspath $(@)))
	@git archive --format=tar --prefix=x52control-$(VERSION)/ \
	HEAD^{tree} | \
	bzip2 > $(@)

$(BUILDDIR)/x52control.xpl: $(BINOBJ)
	@$(print_ld) $(subst $(PWD)/, ./, $(abspath $(@)))
	@-mkdir -p $(dir $(@))
	@$(LD) $(LDFLAGS) $(LPATH) $(FRAMEWORKS) \
	-o $(@) $(BINOBJ) $(LIBRARIES)
ifdef DIST
ifdef PLAT_LINUX
	@strip $(@)
endif
endif


$(BUILDDIR)/.obj/%_C.o: %.c
	@$(print_cc) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
ifndef DIST
	@$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -S -o $(@:.o=.s) $(<)
	@$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -M -MT \
		"$(@) $(@:.o=.dep)" -o $(@:.o=.dep) $(<)
endif
	@$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c -o $(@) $(<)

$(BUILDDIR)/.obj/%_CXX.o: %.cpp
	@$(print_cxx) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
ifndef DIST
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -S -o $(@:.o=.s) $(<)
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -M -MT \
		"$(@) $(@:.o=.dep)" -o $(@:.o=.dep) $(<)
endif
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c -o $(@) $(<)

-include $(DEPS)