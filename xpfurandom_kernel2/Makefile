all: compile

obj-m    := xpfurandom_kernel.o
#xpfurandom_kernel_linux_test-y := xpfurandom_kernel.o

KDIR    := /lib/modules/$(shell uname -r)/build
#KDIR	:= /home/data/src/kernel/linux
#KBUILD_EXTRA_SYMBOLS=$(PWD)/Module.symvers
PWD    := $(shell pwd)

.PHONY: clean default
clean:
	rm -f *.o *.ko .*.d .*.tmp .*.cmd *.mod.c Module.symvers modules.order
	rm -fr .tmp_versions


compile:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules


