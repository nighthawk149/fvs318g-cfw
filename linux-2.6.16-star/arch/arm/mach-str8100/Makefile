#
# Makefile for the linux kernel.
#

# Object file lists.
str8100_demo_pcm_legerity_2phone-objs := str8100_demo_pcm_legerity_config_2phone.o
str8100_demo_i2s-objs := str8100_demo_i2s_config.o
str8100_demo_i2s_wm8772-objs := str8100_demo_i2s_wm8772_config.o
obj-y			:= str8100_debug.o str8100_setup.o str8100_timer.o str8100_intc.o str8100_misc.o str8100_counter.o str8100_pm.o str8100_demo_dma.o
obj-m			:=
obj-n			:=
obj-			:=

obj-$(CONFIG_PCI) += str8100_pci.o
obj-$(CONFIG_STR8100_DMA) += str8100_dma.o
obj-$(CONFIG_STR8100_HSDMA) += str8100_hsdma.o
obj-$(CONFIG_STR8100_I2S_DEMO) += str8100_demo_i2s.o
obj-$(CONFIG_STR8100_I2S_WM8772_DEMO) += str8100_demo_i2s_wm8772.o
obj-$(CONFIG_LE88221_CONTROL) += le88221_control.o
obj-$(CONFIG_STR8100_PCM_LEGERITY_2PHONE_DEMO) += str8100_demo_pcm_legerity_2phone.o
 
