 /*******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#include <linux/mm.h>
#include <linux/init.h>
#include <linux/config.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>

#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/system.h>
#include <asm/memory.h>
#include <asm/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#define STR9100_UART_XTAL 14769230

#define EARLY_REGISTER_CONSOLE

/*
 * Standard IO mapping
 */
static struct map_desc str9100_std_desc[] __initdata = {
	{
		.virtual	= SYSVA_SMC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_SMC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_DDR_SDRAM_CONTROLLER_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_DDR_SDRAM_CONTROLLER_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_DMAC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_DMAC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_GSW_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_GSW_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_HNAT_SRAM_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_HNAT_SRAM_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_MISC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_MISC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_POWER_MANAGEMENT_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_POWER_MANAGEMENT_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_UART_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_UART_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_TIMER_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_TIMER_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_WATCHDOG_TIMER_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_WATCHDOG_TIMER_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_RTC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_RTC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_GPIO_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_GPIO_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_INTC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_INTC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_PCMCIA_CONTROLLER_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_PCMCIA_CONTROLLER_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_PCI_BRIDGE_CONFIG_DATA_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_PCI_BRIDGE_CONFIG_DATA_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_PCI_BRIDGE_CONFIG_ADDR_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_PCI_BRIDGE_CONFIG_ADDR_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_USB11_CONFIG_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_USB11_CONFIG_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_USB11_OPERATION_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_USB11_OPERATION_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_USB20_CONFIG_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_USB20_CONFIG_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_USB20_OPERATION_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_USB20_OPERATION_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}
};

#ifdef EARLY_REGISTER_CONSOLE
static struct uart_port str9100_serial_ports[] = {
	{
		.membase	= (char*)(SYSVA_UART_BASE_ADDR),
		.mapbase	= (SYSPA_UART_BASE_ADDR),
		.irq		= INTC_UART_BIT_INDEX,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= STR9100_UART_XTAL,
		.line		= 0,
		.type		= PORT_16550A,
		.fifosize	= 16
	}
};
#else
static struct resource str9100_uart0_resources[] = {
	[0] = {
		.start	= SYSPA_UART_BASE_ADDR,
		.end	= SYSPA_UART_BASE_ADDR + 0xff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INTC_UART_BIT_INDEX,
		.end	= INTC_UART_BIT_INDEX,
		.flags	= IORESOURCE_IRQ
	}
};

static struct plat_serial8250_port str9100_uart0_data[] = {
	{
		.membase	= (char*)(SYSVA_UART_BASE_ADDR),
		.mapbase	= (SYSPA_UART_BASE_ADDR),
		.irq		= INTC_UART_BIT_INDEX,
		.uartclk	= STR9100_UART_XTAL,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
	},
	{  },
};

static struct platform_device str9100_uart0_device = {
	.name			= "serial8250",
	.id			= 0,
	.dev.platform_data	= str9100_uart0_data,
	.num_resources		= 2,
	.resource		= str9100_uart0_resources,
};
#endif

static u64 usb_dmamask = 0xffffffffULL;
static struct resource str9100_usb11_resources[] = {
	[0] = {
		.start	= SYSPA_USB11_CONFIG_BASE_ADDR,
		.end	= SYSPA_USB11_CONFIG_BASE_ADDR + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INTC_USB11_BIT_INDEX,
		.end	= INTC_USB11_BIT_INDEX,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device str9100_usb11_device = {
	.name		= "str9100-ohci",
	.id		= -1,
	.dev = {
		.dma_mask		= &usb_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.resource	= str9100_usb11_resources,
	.num_resources	= ARRAY_SIZE(str9100_usb11_resources),
};

static struct resource str9100_usb20_resources[] = {
	[0] = {
		.start	= SYSPA_USB20_CONFIG_BASE_ADDR,
		.end	= SYSPA_USB20_CONFIG_BASE_ADDR + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INTC_USB20_BIT_INDEX,
		.end	= INTC_USB20_BIT_INDEX,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device str9100_usb20_device = {
	.name		= "str9100-ehci",
	.id		= -1,
	.dev		= {
		.dma_mask		= &usb_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.resource	= str9100_usb20_resources,
	.num_resources	= ARRAY_SIZE(str9100_usb20_resources),
};

static struct platform_device *str9100_devices[] __initdata = {
#ifndef EARLY_REGISTER_CONSOLE
	&str9100_uart0_device,
#endif
	&str9100_usb11_device,
	&str9100_usb20_device
};

static void __init str9100_fixup(struct machine_desc *desc,
	struct tag *tags, char **cmdline, struct meminfo *mi)
{
        mi->nr_banks = 1;
	mi->bank[0].start = CONFIG_SYSTEM_DRAM_BASE;
	mi->bank[0].size = CONFIG_SYSTEM_DRAM_SIZE << 20;
	mi->bank[0].node = 0;
}

/* ######################################################################### */
#ifdef CONFIG_CPU_ISPAD_ENABLE 
extern unsigned long __ispad_begin; 
extern int str9100_enable_ispad(unsigned long); 
#endif
#ifdef CONFIG_CPU_DSPAD_ENABLE
extern unsigned long __dspad_begin; 
extern int str9100_enable_dspad(unsigned long); 
#endif

u32 CPU_clock;
u32 AHB_clock;
u32 APB_clock;
// This function is called just after the
// page table and cpu have been initialized
void __init str9100_early_init(void)
{
	switch ((PWRMGT_RESET_LATCH_CONFIGURATION_REG >> 6) & 0x03) {
	case 0x00:
		CPU_clock = 175000000;
		break;

	case 0x01:
		CPU_clock = 200000000;
		break;

	case 0x02:
		CPU_clock = 225000000;
		break;

	case 0x03:
		CPU_clock = 250000000;
		break;
	}

	AHB_clock = CPU_clock >> 1;
	APB_clock = AHB_clock >> 1;

	printk("CPU clock at %dMHz\n", CPU_clock / 1000000);
	printk("AHB clock at %dMHz\n", AHB_clock / 1000000);
	printk("APB clock at %dMHz\n", APB_clock / 1000000);


#ifdef CONFIG_CPU_ISPAD_ENABLE
    printk("Enable I-ScratchPad Address Start : %x \n", (u32)&__ispad_begin);
	str9100_enable_ispad((unsigned long)&__ispad_begin);
#endif
#ifdef CONFIG_CPU_DSPAD_ENABLE
    printk("Enable D-ScratchPad Address Start : %x \n", (u32)&__dspad_begin);
    str9100_enable_dspad((unsigned long)&__dspad_begin);
#endif
}
/* ######################################################################### */

void __init str9100_init(void)
{
#if 1
	platform_add_devices(str9100_devices, ARRAY_SIZE(str9100_devices));
#else
#ifndef EARLY_REGISTER_CONSOLE
	platform_device_register(&str9100_uart0_device);
#endif
	platform_device_register(&str9100_usb11_device);
	platform_device_register(&str9100_usb20_device);
#endif
}

extern void str9100_register_map_desc(struct map_desc *map, int count);
void __init str9100_map_io(void)
{
	iotable_init(str9100_std_desc, ARRAY_SIZE(str9100_std_desc));
	str9100_register_map_desc(str9100_std_desc, ARRAY_SIZE(str9100_std_desc));
#ifdef EARLY_REGISTER_CONSOLE
	early_serial_setup(&str9100_serial_ports[0]);
#endif
}

extern void str9100_init_irq(void);
extern struct sys_timer str9100_timer;

MACHINE_START(STR9100, "STAR STR9100")
	.phys_io	= SYSPA_UART_BASE_ADDR,
	.io_pg_offst	= ((SYSVA_UART_BASE_ADDR) >> 18) & 0xfffc, // virtual, physical
	.fixup		= str9100_fixup,
	.map_io		= str9100_map_io,
	.init_irq	= str9100_init_irq,
	.timer		= &str9100_timer,
	.boot_params	= 0x0100,
	.init_machine	= str9100_init,
MACHINE_END

