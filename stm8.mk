export GCC_BASE=/opt/sdcc-3.4.0
export GCC_CROSS=sd

export GCC_VERSION=4.7.4
export PATH := ":$(GCC_BASE)/bin:/opt/stm8flash:$(OPT_HOME)/scripts:$(PATH)"
export CROSS_COMPILE=$(GCC_CROSS)
export CC=$(CROSS_COMPILE)cc
export LD=$(CROSS_COMPILE)ld
export AR=$(CROSS_COMPILE)ar
OBJCOPY=$(CROSS_COMPILE)objcopy
OBJDUMP=$(CROSS_COMPILE)objdump
