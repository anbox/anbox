ccflags-y += -I$(src) -Wno-error=implicit-int

ifneq ($(KERNELRELEASE),)
obj-m := ashmem_linux.o
ashmem_linux-y := deps.o ashmem.o

$(obj)/deps.o: $(src)/deps.h

$(obj)/deps.h: $(src)/gen_deps.sh
	sudo $(src)/gen_deps.sh > $@
else
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_SRC) V=0 M=$$PWD

install:
	cp ashmem_linux.ko $(DESTDIR)/

clean:
	rm -rf deps.h *.o *.ko *.mod.c *.symvers *.order .*.cmd .tmp_versions
endif
