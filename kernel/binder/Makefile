ccflags-y += -I$(src)

ifneq ($(KERNELRELEASE),)
obj-m := binder_linux.o
binder_linux-y := deps.o binder.o

$(obj)/deps.o: $(src)/deps.h

$(obj)/deps.h: $(src)/gen_deps.sh
	sudo $(src)/gen_deps.sh > $@
else
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_SRC) V=0 M=$$PWD

clean:
	rm -rf deps.h *.o *.ko *.mod.c *.symvers *.order .*.cmd .tmp_versions
endif