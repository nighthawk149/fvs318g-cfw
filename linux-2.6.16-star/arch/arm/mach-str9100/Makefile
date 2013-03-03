#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y			:= str9100_debug.o str9100_setup.o str9100_timer.o str9100_intc.o  str9100_misc.o
obj-m			:=
obj-n			:=
obj-			:=

obj-$(CONFIG_STR9100_TIMER_INTERRUPT) += str9100_counter.o
obj-$(CONFIG_PCI) += str9100_pci.o
