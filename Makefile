.PHONY: all configure install clean

all: configure
	@$(MAKE) -C src all

clean:
	@$(MAKE) -C src clean
	rm -f config.mk config.mk.tmp

install: configure
	@$(MAKE) -C src install

configure: config.mk

config.mk: configure.mk
	@$(MAKE) -s -f configure.mk > config.mk.tmp
	@mv config.mk.tmp config.mk

