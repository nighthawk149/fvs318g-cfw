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

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>

#define CONFIG_CMD(bus, device_fn, where) (0x80000000 | ((bus) << 16) | ((device_fn) << 8) | ((where) & ~3))

static struct pci_dev *pci_bridge = NULL;
static u32 pci_config_addr;				// PCI configuration register address port
static u32 pci_config_data;				// PCI configuration register data port
u32 str9100_pci_irqs[4] = {0, INTC_PCI_INTA_BIT_INDEX, INTC_PCI_INTB_BIT_INDEX, 0};

static int str9100_pci_read_config(struct pci_bus *bus,
	unsigned int devfn, int where, int size, u32 *val)
{
	u32 v = 0;
	u32 shift;
	unsigned long flags;

	switch (size) {
	case 1:
		shift = (where & 0x3) << 3;
		local_irq_save(flags);
		__raw_writel(CONFIG_CMD(bus->number, devfn, where), pci_config_addr);
		v = __raw_readl(pci_config_data);
		local_irq_restore(flags);
		v = (v >> shift) & 0xff;
		break;

	case 2:
		shift = (where & 0x3) << 3;
		local_irq_save(flags);
		__raw_writel(CONFIG_CMD(bus->number, devfn, where), pci_config_addr);
		v = __raw_readl(pci_config_data);
		local_irq_restore(flags);
		v = (v >> shift) & 0xffff;

		break;

	case 4:
		local_irq_save(flags);
		__raw_writel(CONFIG_CMD(bus->number, devfn, where), pci_config_addr);
		v = __raw_readl(pci_config_data);
		local_irq_restore(flags);
		break;
	}

	*val = v;
	return PCIBIOS_SUCCESSFUL;
}

static int str9100_pci_write_config(struct pci_bus *bus,
	unsigned int devfn, int where, int size, u32 val)
{
	u32 v;
	u32 shift;
	unsigned long flags;

	switch (size) {
	case 1:
		shift = (where & 0x3) << 3;
		local_irq_save(flags);
		__raw_writel(CONFIG_CMD(bus->number, devfn, where), pci_config_addr);
		v = __raw_readl(pci_config_data);
		v = (v & ~(0xff << shift)) | (val << shift);
		__raw_writel(v, pci_config_data);
		local_irq_restore(flags);
		break;

	case 2:
		shift = (where & 0x3) << 3;
		local_irq_save(flags);
		__raw_writel(CONFIG_CMD(bus->number, devfn, where), pci_config_addr);
		v = __raw_readl(pci_config_data);
		v = (v & ~(0xffff << shift)) | (val << shift);
		__raw_writel(v, pci_config_data);
		local_irq_restore(flags);
		break;

	case 4:
		local_irq_save(flags);
		__raw_writel(CONFIG_CMD(bus->number, devfn, where), pci_config_addr);
		__raw_writel(val, pci_config_data);
		local_irq_restore(flags);
		break;
	}

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops str9100_pci_ops = {
	.read	= str9100_pci_read_config,
	.write	= str9100_pci_write_config,
};

static struct resource str9100_pci_io = {
	.name	= "PCI I/O space",
	.start	= PCI_IO_SPACE_START,
	.end	= PCI_IO_SPACE_END, //albert : 20040714
	.flags	= IORESOURCE_IO,
};

static struct resource str9100_pci_nprefetch_mem = {
	.name	= "PCI non-prefetchable",
	.start	= PCI_NPREFETCH_MEMORY_SPACE_START,
	.end	= PCI_NPREFETCH_MEMORY_SPACE_END,
	.flags	= IORESOURCE_MEM,
};

static struct resource str9100_pci_prefetch_mem = {
	.name	= "PCI prefetchable",
	.start	= PCI_PREFETCH_MEMORY_SPACE_START,
	.end	= PCI_PREFETCH_MEMORY_SPACE_END,
	.flags	= IORESOURCE_MEM | IORESOURCE_PREFETCH,
};

static int __init str9100_pci_setup_resources(struct resource **resource)
{
	int ret = -1;

	ret = request_resource(&iomem_resource, &str9100_pci_io);
	if (ret) {
		printk(KERN_ERR "PCI: unable to allocate I/O "
		       "memory region (%d)\n", ret);
		goto out;
	}
	ret = request_resource(&iomem_resource, &str9100_pci_nprefetch_mem);
	if (ret) {
		printk(KERN_ERR "PCI: unable to allocate non-prefetchable "
		       "memory region (%d)\n", ret);
		goto release_io;
	}
	ret = request_resource(&iomem_resource, &str9100_pci_prefetch_mem);
	if (ret) {
		printk(KERN_ERR "PCI: unable to allocate prefetchable "
		       "memory region (%d)\n", ret);
		goto release_nprefetch_mem;
	}

	/*
	 * bus->resource[0] is the IO resource for this bus
	 * bus->resource[1] is the mem resource for this bus
	 * bus->resource[2] is the prefetch mem resource for this bus
	 */
	resource[0] = &str9100_pci_io;
	resource[1] = &str9100_pci_nprefetch_mem;
	resource[2] = &str9100_pci_prefetch_mem;

	ret = 0;

	goto out;

release_nprefetch_mem:
	release_resource(&str9100_pci_nprefetch_mem);
release_io:
	release_resource(&str9100_pci_io);
out:
	return ret;
}

static irqreturn_t PCI_AHB2PCIB_ISR(int irq, void *dev_id, struct pt_regs * regs)
{
	u32 status;

	//disable_irq(INTC_PCI_AHB2BRIDGE_BIT_INDEX);
	pci_read_config_dword(pci_bridge, PCI_COMMAND, &status);
	printk("AHB to bridge interrupt status: 0x%x\n", status);
	pci_write_config_dword(pci_bridge, PCI_COMMAND, status);
	//enable_irq(INTC_PCI_AHB2BRIDGE_BIT_INDEX);

	return IRQ_HANDLED;
}

static irqreturn_t PCI_BROKEN_ISR(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 status;

	status = MISC_PCI_BROKEN_STATUS_REG & 0x1f;
	printk("PCI BROKEN interrupt status: 0x%x\n", status);
	MISC_PCI_BROKEN_STATUS_REG = status;

	return IRQ_HANDLED;
}

int __init str9100_pci_setup(int nr, struct pci_sys_data *sys)
{
	if (nr != 0) {
		return 0;
	}

	if (str9100_pci_setup_resources(sys->resource)) {
		BUG();
	}

	return 1;
}

struct pci_bus *str9100_pci_scan_bus(int nr, struct pci_sys_data *sys)
{
	return pci_scan_bus(sys->busnr, &str9100_pci_ops, sys);
}

void __init str9100_pci_preinit(void)
{
	pci_config_addr = SYSVA_PCI_BRIDGE_CONFIG_ADDR_BASE_ADDR + PCI_BRIDGE_CONFIG_ADDR_REG_OFFSET;
	pci_config_data = SYSVA_PCI_BRIDGE_CONFIG_DATA_BASE_ADDR + PCI_BRIDGE_CONFIG_DATA_REG_OFFSET;

#if defined(CONFIG_STR9100_PCI66M)
	printk("PCI clock at 66M\n");
	HAL_PWRMGT_ENABLE_PCI_BRIDGE_66M();
#elif defined(CONFIG_STR9100_PCI33M)
	printk("PCI clock at 33M\n");
	HAL_PWRMGT_ENABLE_PCI_BRIDGE_33M();
#else
	printk("PCI clock at 33M\n");
	HAL_PWRMGT_ENABLE_PCI_BRIDGE_33M();
#endif
}

void __init str9100_pci_postinit(void)
{
	pci_bridge = pci_find_device(PCIB_VENDOR_ID, PCIB_DEVICE_ID, NULL);
	if (pci_bridge == NULL) {
		printk("PCI Bridge not found\n");
		return;
	} else {
		printk("PCI Bridge found\n");
	}

	request_irq(INTC_PCI_AHB2BRIDGE_BIT_INDEX, PCI_AHB2PCIB_ISR, SA_INTERRUPT, "pci bridge", pci_bridge);

	MISC_PCI_ARBITER_INTERRUPT_MASK_REG &= ~0x1f;

	request_irq(INTC_PCI_ARBITOR_BIT_INDEX, PCI_BROKEN_ISR, SA_INTERRUPT, "pci broken", pci_bridge);

	pci_write_config_dword(pci_bridge, PCI_BASE_ADDRESS_0, 0x0); // = 0x0, can NOT use 0x20000000
	pci_write_config_dword(pci_bridge, PCI_BASE_ADDRESS_1, 0x0); // = 0x0, can NOT use 0x20000000

	// if we enable pci on u-boot
	// the pci_enable_device will complain with resource collisions
	// use this to fixup
	{
		int i;
		struct resource *r;

		for (i = 0; i < 6; i++) {
			r = pci_bridge->resource + i;
			r->start = 0;
			r->end = 0;
		}
	}

	pci_enable_device(pci_bridge);
	pci_set_master(pci_bridge);
}

/*
 * map the specified device/slot/pin to an IRQ.   Different backplanes may need to modify this.
 */
static int __init str9100_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	/* slot,  pin,	irq
	 * 0      1     0
	 * 1      1     5
	 * 2      1     6
	 * 3      1     0
	 */
	irq = str9100_pci_irqs[((slot + pin - 1) & 3)];

	printk("PCI map irq: %02x:%02x.%02x slot %d, pin %d, irq: %d\n",
		dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn),
		slot, pin, irq);

	return irq;
}

static struct hw_pci str9100_pci __initdata = {
	.swizzle		= pci_std_swizzle,
	.map_irq		= str9100_pci_map_irq,
	.nr_controllers		= 1,
	.setup			= str9100_pci_setup,
	.scan			= str9100_pci_scan_bus,
	.preinit		= str9100_pci_preinit,
	.postinit		= str9100_pci_postinit,
};

static int __init str9100_pci_init(void)
{
	pci_common_init(&str9100_pci);
	return 0;
}

subsys_initcall(str9100_pci_init);
