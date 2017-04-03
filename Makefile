ifeq (${KERNELRELEASE},)
KERNEL_SOURCE := /home/vvdn/Desktop/Kernel/kernel-pandaboard-ES-RevB3-master
PWD := $(shell pwd)
default:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} modules

clean:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} clean
else
	obj-m := pandaboard_gpio.o
endif
