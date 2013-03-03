/******************************************************************************
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


#include <linux/platform_device.h>
#include <asm/arch/hardware.h>

static int str8100_ehci_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci(hcd);
	int			retval;

	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(readl(&ehci->caps->hc_capbase));
	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);

	retval = ehci_halt(ehci);
	if (retval)
		return retval;

	return ehci_init(hcd);
}
#ifdef	CONFIG_PM
//mkl070226: the functionality of suspend/resume is not complete
static int ehci_suspend (struct usb_hcd *hcd)
{
	printk("%s: not implemented, just pass it\n",__FUNCTION__);
	return 0;
}
static int ehci_resume (struct usb_hcd *hcd)
{
	printk("%s: not implemented, just pass it\n",__FUNCTION__);
	return 0;
}
#endif

static const struct hc_driver str8100_ehci_driver = {
	.description =		hcd_name,
        .product_desc =		"str8100-ehci",
        .hcd_priv_size =	sizeof(struct ehci_hcd),
	/*
	 * generic hardware linkage
	 */
	.irq =			ehci_irq,
	.flags =		HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	.reset =		str8100_ehci_setup, 
	.start =		ehci_run,
#ifdef	CONFIG_PM
//mkl070226: the functionality of suspend/resume is not complete
	.suspend =		ehci_suspend,
	.resume =		ehci_resume,

	.bus_suspend =		ehci_suspend,
	.bus_resume =		ehci_resume,
#endif
	.stop =			ehci_stop,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ehci_urb_enqueue,
	.urb_dequeue =		ehci_urb_dequeue,
	.endpoint_disable =	ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ehci_hub_status_data,
	.hub_control =		ehci_hub_control,
};

static void __init str8100_usb20_config_reg_init(void)
{
#if 0
	__asm__ __volatile__(
		"mov	r1, #0			\n"
		"mcr	p15, 0, r1, c7, c5, 0	\n"
		"mov	r1, #0			\n"
		"mcr	p15, 0, r1, c7, c14, 0	\n"
	);
#endif
	__raw_writel(0x106, SYSVA_USB20_CONFIG_BASE_ADDR + 0x04);
	__raw_writel((3 << 5) | 0x20000, SYSVA_USB20_OPERATION_BASE_ADDR + 0x40);
	mdelay(100);
}

int str8100_ehci_usb_hcd_probe(const struct hc_driver *driver, struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	char *name = "str8100-ehci";
	int retval = 0;

	str8100_usb20_config_reg_init();
	hcd = usb_create_hcd(driver, &pdev->dev, name);
	if (!hcd) { 
		retval = -ENOMEM;
		return retval;
	}
	hcd->regs = (unsigned int *)SYSVA_USB20_OPERATION_BASE_ADDR;
	hcd->rsrc_start = SYSPA_USB20_OPERATION_BASE_ADDR;
	hcd->rsrc_len = 4096;
	hcd->driver = driver;
	retval = usb_add_hcd(hcd, INTC_USB20_BIT_INDEX, SA_INTERRUPT);
	if (retval == 0) {
		return retval;
	}
	printk("str8100 ehci init fail, %d\n", retval);
	usb_put_hcd(hcd);
	return retval;
}

int str8100_ehci_usb_hcd_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	return 0;
}

static int str8100_ehci_hcd_drv_probe(struct platform_device *pdev)
{
	return str8100_ehci_usb_hcd_probe(&str8100_ehci_driver, pdev);
}

static struct platform_driver str8100_ehci_hcd_driver = {
	.probe		= str8100_ehci_hcd_drv_probe,
	.remove		= str8100_ehci_usb_hcd_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "str8100-ehci",
	},
};

static int __init str8100_ehci_hcd_init(void)
{
	if (usb_disabled())
		return -ENODEV;

	pr_debug("%s: block sizes: qh %Zd qtd %Zd itd %Zd sitd %Zd\n",
		hcd_name,
		sizeof(struct ehci_qh), sizeof(struct ehci_qtd),
		sizeof(struct ehci_itd), sizeof(struct ehci_sitd));

	return platform_driver_register(&str8100_ehci_hcd_driver);
}
module_init(str8100_ehci_hcd_init);

static void __exit str8100_ehci_hcd_cleanup(void)
{
	platform_driver_unregister(&str8100_ehci_hcd_driver);
}
module_exit(str8100_ehci_hcd_cleanup);

