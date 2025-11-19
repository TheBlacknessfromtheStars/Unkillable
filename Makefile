KDIR = $(KDIR)
KERNEL_OUT ?= out

ARCH = arm64
CROSS_COMPILE = aarch64-linux-gnu-
CROSS_COMPILE_ARM32 = arm-linux-gnueabihf-
CC = clang
LD = ld.lld
HOSTCC = clang
HOSTLD = ld.lld
LLVM = -18

all:
	$(MAKE) -C $(KDIR) O=$(KERNEL_OUT) LLVM=$(LLVM) ARCH=$(ARCH) \
	CROSS_COMPILE=$(CROSS_COMPILE) CROSS_COMPILE_ARM32=$(CROSS_COMPILE_ARM32) \
	CC=$(CC) LD=$(LD) HOSTCC=$(HOSTCC) HOSTLD=$(HOSTLD) \
	M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) O=$(KERNEL_OUT) ARCH=$(ARCH) \
	CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean

.PHONY: all clean
