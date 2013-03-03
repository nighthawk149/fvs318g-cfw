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
#ifdef CONFIG_SPI_STR8100
#include <asm/arch/star_spi.h>
#include <linux/spi/spi.h>
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
#include <linux/spi/flash.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#endif
#endif // CONFIG_SPI_STR8100

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

#if 1 // on ASIC
#define STR8100_UART_XTAL 24000000
#else // on FPGA
#define STR8100_UART_XTAL 13000000
#endif

#define EARLY_REGISTER_CONSOLE
#define DEVICE_REGISTER_MULTIPLE

/*
 * Standard IO mapping
 */
static struct map_desc str8100_std_desc[] __initdata = {
	{
		.virtual	= SYSVA_IDE_DEVICE_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_IDE_DEVICE_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_GDMAC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_GDMAC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_NIC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_NIC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_SPI_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_SPI_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_PCM_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_PCM_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_I2C_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_I2C_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_I2S_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_I2S_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_DDRC_SDRC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_DDRC_SDRC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_SMC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_SMC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_IDE_CONTROLLER_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_IDE_CONTROLLER_BASE_ADDR),
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
		.virtual	= SYSVA_UART0_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_UART0_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_UART1_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_UART1_BASE_ADDR),
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
		.virtual	= SYSVA_GPIOA_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_GPIOA_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	= SYSVA_GPIOB_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_GPIOB_BASE_ADDR),
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
	}, {
		.virtual	= SYSVA_USB20_DEVICE_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_USB20_DEVICE_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
#if 1
		.virtual	= SYSVA_FLASH_SRAM_BANK1_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_FLASH_SRAM_BANK1_BASE_ADDR),
		.length		= SZ_8M,
		.type		= MT_DEVICE
	}, {
#else
		.virtual	= SYSVA_FLASH_SRAM_BANK1_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_FLASH_SRAM_BANK1_BASE_ADDR),
		.length		= SZ_64K,
		.type		= MT_DEVICE
	}, {
#endif
		.virtual	= SYSVA_VIC_BASE_ADDR,
		.pfn		= __phys_to_pfn(SYSPA_VIC_BASE_ADDR),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}
};

#ifdef EARLY_REGISTER_CONSOLE
static struct uart_port str8100_serial_ports[] = {
	{
		.membase	= (char*)(SYSVA_UART0_BASE_ADDR),
		.mapbase	= (SYSPA_UART0_BASE_ADDR),
		.irq		= INTC_UART0_BIT_INDEX,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= STR8100_UART_XTAL,
		.line		= 0,
		.type		= PORT_16550A,
		.fifosize	= 16
	} , {
		.membase	= (char*)(SYSVA_UART1_BASE_ADDR),
		.mapbase	= (SYSPA_UART1_BASE_ADDR),
		.irq		= INTC_UART1_BIT_INDEX,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= STR8100_UART_XTAL,
		.line		= 1,
		.type		= PORT_16550A,
		.fifosize	= 16
	}
};
#else
static struct resource str8100_uart0_resources[] = {
	[0] = {
		.start	= SYSPA_UART0_BASE_ADDR,
		.end	= SYSPA_UART0_BASE_ADDR + 0xff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INTC_UART0_BIT_INDEX,
		.end	= INTC_UART0_BIT_INDEX,
		.flags	= IORESOURCE_IRQ
	}
};

static struct resource str8100_uart1_resources[] = {
	[0] = {
		.start	= SYSPA_UART1_BASE_ADDR,
		.end	= SYSPA_UART1_BASE_ADDR + 0xff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INTC_UART1_BIT_INDEX,
		.end	= INTC_UART1_BIT_INDEX,
		.flags	= IORESOURCE_IRQ
	}
};

static struct plat_serial8250_port str8100_uart0_data[] = {
	{
		.membase	= (char*)(SYSVA_UART0_BASE_ADDR),
		.mapbase	= (SYSPA_UART0_BASE_ADDR),
		.irq		= INTC_UART0_BIT_INDEX,
		.uartclk	= STR8100_UART_XTAL,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
	},
	{  },
};

static struct plat_serial8250_port str8100_uart1_data[] = {
	{
		.membase	= (char*)(SYSVA_UART1_BASE_ADDR),
		.mapbase	= (SYSPA_UART1_BASE_ADDR),
		.irq		= INTC_UART1_BIT_INDEX,
		.uartclk	= STR8100_UART_XTAL,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
	},
	{  },
};

static struct platform_device str8100_uart0_device = {
	.name = "serial8250",
	.id = 0,
	.dev.platform_data = str8100_uart0_data,
	.num_resources = 2,
	.resource = str8100_uart0_resources,
};

static struct platform_device str8100_uart1_device = {
	.name = "serial8250",
	.id = 1,
	.dev.platform_data = str8100_uart1_data,
	.num_resources = 2,
	.resource = str8100_uart1_resources,
};
#endif


static u64 usb_dmamask = 0xffffffffULL;
static struct resource str8100_usb11_resources[] = {
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

static struct platform_device str8100_usb11_device = {
	.name		= "str8100-ohci",
	.id		= -1,
	.dev = {
		.dma_mask		= &usb_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.resource	= str8100_usb11_resources,
	.num_resources	= ARRAY_SIZE(str8100_usb11_resources),
};

static struct resource str8100_usb20_resources[] = {
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

static struct platform_device str8100_usb20_device = {
	.name		= "str8100-ehci",
	.id		= -1,
	.dev		= {
		.dma_mask		= &usb_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.resource	= str8100_usb20_resources,
	.num_resources	= ARRAY_SIZE(str8100_usb20_resources),
};

#ifdef CONFIG_SPI_STR8100
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
static struct str8100_spi_dev_attr str8100_spi_flash_attr = {
	.spi_serial_mode = STR8100_SPI_SERIAL_MODE_GENERAL,
};

static struct mtd_partition str8100_spi_flash_partitions[] = {
	{
		.name =		"ALL",
		.offset =	CONFIG_ARMBOOT_OFFSET,
		.size =		0x800000-CONFIG_ARMBOOT_OFFSET,
	},{
		.name =		"ARMBOOT",
		.offset =	CONFIG_ARMBOOT_OFFSET,
		.size =		CONFIG_KERNEL_OFFSET-CONFIG_ARMBOOT_OFFSET,
	},{
		.name =		"Linux Kernel",
		.offset =	CONFIG_KERNEL_OFFSET,
		.size =		CONFIG_ROOTFS_OFFSET-CONFIG_KERNEL_OFFSET,
	},{
		.name =		"MTD Disk1",
		.offset =	CONFIG_ROOTFS_OFFSET,
		.size =		CONFIG_CFG_OFFSET-CONFIG_ROOTFS_OFFSET,
	},{
		.name =		"MTD Disk2",
		.offset =	CONFIG_CFG_OFFSET,
		.size =		0x800000-CONFIG_CFG_OFFSET,
	}
};

static struct flash_platform_data str8100_spi_flash_data = {
	.name = "m25p80",
	.parts = str8100_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(str8100_spi_flash_partitions),
	.type = "m25p64",
};
#endif

#if defined(CONFIG_LE88221_CONTROL)
static struct str8100_spi_dev_attr str8100_spi_le88221_attr = {
	.spi_serial_mode = STR8100_SPI_SERIAL_MODE_MICROPROCESSOR,
};
#endif

#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE) || defined(CONFIG_LE88221_CONTROL)
static struct spi_board_info str8100_spi_board_info[] = {
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
        {
		.modalias	= "m25p80",
		.chip_select	= 0,
		.max_speed_hz	= 25 * 1000 * 1000,
		.bus_num	= 1,
		.platform_data	= &str8100_spi_flash_data,
		.controller_data = &str8100_spi_flash_attr,
        },
#endif
#if defined(CONFIG_LE88221_CONTROL)
	{
		.modalias	= "le88221",
		.chip_select	= 1,
		.max_speed_hz	= 25 * 1000 * 1000,
		.bus_num	= 1,
		.platform_data	= NULL,
		.controller_data = &str8100_spi_le88221_attr,
        },
#endif
};
#endif

static u64 spi_dmamask = 0xffffffffUL;
static struct resource str8100_spi_resources[] = {
	[0] = {
		.start	= SYSPA_SPI_BASE_ADDR,
		.end	= SYSPA_SPI_BASE_ADDR + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INTC_SPI_BIT_INDEX,
		.end	= INTC_SPI_BIT_INDEX,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device str8100_spi_master_device = {
	.name		= "str8100_spi",
	.id		= -1,
	.dev		= {
		.dma_mask		= &spi_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.resource	= str8100_spi_resources,
	.num_resources	= ARRAY_SIZE(str8100_spi_resources),
};
#endif // CONFIG_SPI_STR8100

#ifdef DEVICE_REGISTER_MULTIPLE
static struct platform_device *str8100_devices[] __initdata = {
#ifndef EARLY_REGISTER_CONSOLE
	&str8100_uart0_device,
	&str8100_uart1_device,
#endif
#ifdef CONFIG_SPI_STR8100
	&str8100_spi_master_device,
#endif
	&str8100_usb11_device,
	&str8100_usb20_device
};
#endif

static void __init str8100_fixup(struct machine_desc *desc,
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
#endif
u32 PLL_clock;
u32 CPU_clock;
u32 AHB_clock;
u32 APB_clock;
EXPORT_SYMBOL(PLL_clock);
EXPORT_SYMBOL(CPU_clock);
EXPORT_SYMBOL(AHB_clock);
EXPORT_SYMBOL(APB_clock);
// This function is called just after the
// page table and cpu have been initialized
void __init str8100_early_init(void)
{
	switch (PWRMGT_SYSTEM_CLOCK_CONTROL_REG & 0x3) {
	case 0x0:
		PLL_clock = 175000000;
		break;
	case 0x1:
		PLL_clock = 200000000;
		break;
	case 0x2:
		PLL_clock = 225000000;
		break;
	case 0x3:
		PLL_clock = 250000000;
		break;
	}

	CPU_clock = PLL_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 2) & 0x3) + 1);
	AHB_clock = CPU_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 4) & 0x3) + 1);
	APB_clock = AHB_clock / (((PWRMGT_SYSTEM_CLOCK_CONTROL_REG >> 8) & 0x3) + 1);

	printk("PLL clock at %dMHz\n", PLL_clock / 1000000);
	printk("CPU clock at %dMHz\n", CPU_clock / 1000000);
	printk("AHB clock at %dMHz\n", AHB_clock / 1000000);
	printk("APB clock at %dMHz\n", APB_clock / 1000000);
}
/* ######################################################################### */

void __init str8100_init(void)
{
#ifdef DEVICE_REGISTER_MULTIPLE
	platform_add_devices(str8100_devices, ARRAY_SIZE(str8100_devices));
#ifdef CONFIG_SPI_STR8100
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE) || defined(CONFIG_LE88221_CONTROL)
	spi_register_board_info(str8100_spi_board_info, ARRAY_SIZE(str8100_spi_board_info));
#endif
#endif // CONFIG_SPI_STR8100
#else // DEVICE_REGISTER_MULTIPLE
#ifndef EARLY_REGISTER_CONSOLE
	platform_device_register(&str8100_uart0_device);
	platform_device_register(&str8100_uart1_device);
#endif
#ifdef CONFIG_SPI_STR8100
	platform_device_register(&str8100_spi_master_device);
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE) || defined(CONFIG_LE88221_CONTROL)
	spi_register_board_info(str8100_spi_board_info, ARRAY_SIZE(str8100_spi_board_info));
#endif
#endif
	platform_device_register(&str8100_usb11_device);
	platform_device_register(&str8100_usb20_device);
#endif
}

extern void str8100_register_map_desc(struct map_desc *map, int count);
void __init str8100_map_io(void)
{
	iotable_init(str8100_std_desc, ARRAY_SIZE(str8100_std_desc));
	str8100_register_map_desc(str8100_std_desc, ARRAY_SIZE(str8100_std_desc));
#ifdef EARLY_REGISTER_CONSOLE
	early_serial_setup(&str8100_serial_ports[0]);
	early_serial_setup(&str8100_serial_ports[1]);
#endif
}

extern void str8100_init_irq(void);
extern struct sys_timer str8100_timer;

MACHINE_START(STR8100, "STAR STR8100")
	.phys_io	= SYSPA_UART0_BASE_ADDR,
	.io_pg_offst	= ((SYSVA_UART0_BASE_ADDR) >> 18) & 0xfffc, // virtual, physical
	.fixup		= str8100_fixup,
	.map_io		= str8100_map_io,
	.init_irq	= str8100_init_irq,
	.timer		= &str8100_timer,
	.boot_params	= 0x0100,
	.init_machine	= str8100_init,
MACHINE_END

