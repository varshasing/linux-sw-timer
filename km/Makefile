ifneq ($(KERNELRELEASE),)
    obj-m := mytimer.o
else
    KERNEL_DIR ?= $(EC535)/bbb/stock/stock-linux-4.19.82-ti-rt-r33
    PWD := $(shell pwd)
    ARCH := arm
    CROSS_COMPILE := arm-linux-gnueabihf-

default:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) clean

endif

