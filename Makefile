ARCH ?= aarch64
ifeq ($(ARCH), aarch64)
	include aarch64/Makefile
else ifeq ($(ARCH), i386)
	include i386/Makefile
endif
