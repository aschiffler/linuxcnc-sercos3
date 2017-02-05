include ../config.mk
include Kbuild

SUBDIRS := \
SIII \
SICE \
S3SM \
RTLX \
CSMD \

-include SIII/subdir.mk
-include SICE/subdir.mk
-include S3SM/subdir.mk
-include RTLX/subdir.mk
-include CSMD/subdir.mk

cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

.PHONY: all install

ifeq ($(BUILDSYS),kbuild)

module = $(patsubst %.o,%.ko,$(obj-m))

ifeq (,$(findstring -Wframe-larger-than=,$(EXTRA_CFLAGS)))
  EXTRA_CFLAGS += $(call cc-option,-Wframe-larger-than=2560)
endif

$(module):
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_EXTRA_SYMBOLS="$(RTLIBDIR)/Module.symvers $(RTAIDIR)/modules/ethercat/Module.symvers" -C $(KERNELDIR) SUBDIRS=`pwd` CC=$(CC) V=0 modules

else

module = $(patsubst %.o,%.so,$(obj-m))

EXTRA_CFLAGS := $(filter-out -Wframe-larger-than=%,$(EXTRA_CFLAGS))

$(module): $(OBJS) $(USER_OBJS)
	$(CC) -shared -o $@ $(OBJS) -Wl,-rpath,$(LIBDIR) -L$(LIBDIR) -llinuxcnchal -lrt -lpci -lpthread

%.o: %.c
	$(CC) -o $@ $(EXTRA_CFLAGS) -Os -c $<

endif

all: $(module)

install: $(module)
	mkdir -p $(DESTDIR)$(RTLIBDIR)
	cp $(module) $(DESTDIR)$(RTLIBDIR)/
clean:
	rm -f *.so *.ko *.o
	rm -f *.mod.c .*.cmd
	rm -f modules.order Module.symvers
	rm -rf .tmp_versions
	rm -rf $(OBJS)$(C_DEPS)
