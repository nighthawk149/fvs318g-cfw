/*
 * Faraday USB220 ("FUSB220") USB Device Controller driver
 *
 * Copyright (C) 2004-2005 Lineo
 *      by John Chiang
 * Copyright (C) 2004 Faraday tech corp.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

/*
 * This device has ep0 and four bulk/interrupt endpoints.
 *
 *  - Endpoint numbering is fixed: EP1-bulk in, EP2-bulk out, EP3-interrupt in, EP4 interrupt out 
 *  - EP maxpacket (if full speed:64, if high speed:512)
 *  - no DMA supporting in the first version
 *  - support AHB_DMA in the 2nd version
 */

#undef DEBUG
// #define      VERBOSE         /* extra debug messages (success too) */
// #define      USB_TRACE       /* packet-level success messages */

#if 1
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>

#include <asm/byteorder.h>
#include <asm/dma.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <asm/unaligned.h>
#include <asm/hardware.h>

#include <linux/usb_ch9.h>
#include <linux/usb_gadget.h>
#else
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>

#include <linux/usb_ch9.h>
#include <linux/usb_gadget.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pci.h>
#include <asm/system.h>
#include <asm/unaligned.h>
//#include <asm/arch/cpe_int.h>
//#include <asm/arch/ahb_dma.h>
#endif

#include "FTC_FUSB220_udc.h"

#define	DRIVER_DESC		"FUSB220 USB Device Controller"
#define	DRIVER_VERSION	"04-Oct 2004"

static const char driver_name[] = "FTC_FUSB220_udc";
static const char driver_desc[] = DRIVER_DESC;

static char *names[] =
    { "ep0", "ep1-bulkin", "ep2-bulkout", "ep3-intin", "ep4-intout", "ep5", "ep6", "ep7", "ep8", "ep9", "ep10" };
#define BULK_IN_EP_NUM		1
#define BULK_OUT_EP_NUM		2
#define INTR_IN_EP_NUM		3
#define INTR_OUT_EP_NUM		4

static struct FTC_udc *the_controller = 0;

MODULE_AUTHOR("john@faraday-tech.com");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define DBG_TEMP_udc(fmt,args...) printk(KERN_INFO "%s : " fmt , "fsg" , ## args)	//Bruce;;Remove

/*-------------------------------------------------------------------------*/

static void nuke(struct FTC_ep *, int status);

//*******************************************************************
// Name:FIFO_Int
// Description:FUSB220 to enable/disable FIFO interrupt of dedicated EP
//*******************************************************************
static int
FIFO_Int(struct FTC_ep *ep, unsigned int val)
{
	u8 u8fifo_n;
	u32 offset;
	u8 mval;

	DBG_FUNCC("+FIFO_Int()(ed=%d INT=%d)\n", ep->num, val);

	//<1>.Get the fifo number
	u8fifo_n = mUsbEPMapRd(ep->num);	// get the relatived FIFO number
	if (ep->is_in)
		u8fifo_n &= 0x0F;
	else
		u8fifo_n >>= 4;
	if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
		return -EINVAL;

	if (ep->is_in) {	// IN 
		offset = 0x16;
		while (u8fifo_n > 8) {
			offset++;
			u8fifo_n -= 8;
		}
		mval = 1 << u8fifo_n;
	} else {
		offset = 0x12;
		while (u8fifo_n > 4) {
			offset++;
			u8fifo_n -= 4;
		}
		mval = 1 << (u8fifo_n * 2) | 1 << ((u8fifo_n * 2) + 1);
	}

	if (val) {		// enable FIFO interrupt
		mUsbIntFIFOEn(offset, mval);
	} else {
		mUsbIntFIFODis(offset, mval);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////
// Enable endpoint 
// EP0 : has been enabled while driver booting up
// Need to give this EP's descriptor
static int
FTC_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct FTC_udc *dev;
	struct FTC_ep *ep;
	u16 max;
	unsigned long flags;

	ep = container_of(_ep, struct FTC_ep, ep);

	DBG_FUNCC("+FTC_ep_enable() : _ep = %x desc = %x ep->desc= %x\n", (u32) _ep, (u32) desc, (u32) ep->desc);

	// check input variable, if there ia any variable undefined, return false
	if (!_ep || !desc || ep->desc || desc->bDescriptorType != USB_DT_ENDPOINT) {
		return -EINVAL;
	}
	// if this is used to enable ep0, return false
	dev = ep->dev;
	if (ep == &dev->ep[0]) {	//no EP0 need to be enabled
		return -EINVAL;
	}
	// if upper level driver not ready or device speed unknown, return false
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {
		return -EINVAL;
	}

	if (ep->num != (desc->bEndpointAddress & 0x0f)) {
		return -EINVAL;
	}
	// EP should be Bulk or intr
	switch (desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
	case USB_ENDPOINT_XFER_BULK:
	case USB_ENDPOINT_XFER_INT:
		break;
	default:
		return -EINVAL;
	}

	/* enabling the no-toggle interrupt mode would need an api hook */
	max = le16_to_cpu(get_unaligned(&desc->wMaxPacketSize));

	// 11/2/05' AHB_DMA
	// Only bulk use AHB_DMA, and not always use DMA, so change while running
	//if ((desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
	//   ep->dma = 1;
	//else
	ep->dma = 0;		// right now we choice not to use dma

	ep->is_in = (USB_DIR_IN & desc->bEndpointAddress) != 0;

	spin_lock_irqsave(&ep->dev->lock, flags);

	//Do not enable the Interrupt for this ep, enable interrupt when upper level set queue for this endpoint 
	FIFO_Int(ep, 0);

	ep->ep.maxpacket = max;
	ep->stopped = 0;
	ep->dma_running = FALSE;
	ep->desc = desc;
	spin_unlock_irqrestore(&ep->dev->lock, flags);

	printk("enable %s %s %s maxpacket %u\n", ep->ep.name, ep->is_in ? "IN" : "OUT", ep->dma ? "dma" : "pio", max);

	return 0;
}

static void
ep_reset(struct FTC_ep *ep)
{
	//struct FTC_udc                *dev = ep->dev;
	DBG_FUNCC("+ep_reset\n");

	ep->ep.maxpacket = MAX_FIFO_SIZE;
	ep->desc = 0;
	ep->stopped = 1;
	ep->irqs = 0;
	ep->dma = 0;
}

static int
FTC_ep_disable(struct usb_ep *_ep)
{
	struct FTC_ep *ep;
	struct FTC_udc *dev;
	unsigned long flags;

	DBG_FUNCC("+FTC_ep_disable()\n");

	ep = container_of(_ep, struct FTC_ep, ep);
	if (!_ep || !ep->desc)
		return -ENODEV;

	//printk("+FTC_ep_disable() : _ep = 0x%x ep->desc = 0x%x\n", _ep , ep->desc);
	dev = ep->dev;

	//John mark for in suspend will reset system
	//john if (dev->ep0state == EP0_SUSPEND)
	//john  return -EBUSY;

	if (ep == &dev->ep[0])	//john no EP0 need to be enabled
		return -EINVAL;

	VDBG(dev, "disable %s\n", _ep->name);

	spin_lock_irqsave(&dev->lock, flags);
	nuke(ep, -ESHUTDOWN);
	ep_reset(ep);

	////John FUSB220, disable FIFO interrupt
	FIFO_Int(ep, 0);
	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/*-------------------------------------------------------------------------*/

static struct usb_request *
FTC_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct FTC_request *req;

	DBG_FUNCC("+FTC_alloc_request\n");
	if (!_ep)
		return 0;

	req = kmalloc(sizeof *req, gfp_flags);
	if (!req)
		return 0;

	memset(req, 0, sizeof *req);
	req->req.dma = DMA_ADDR_INVALID;
	INIT_LIST_HEAD(&req->queue);

	DBG_FUNCC("-FTC_alloc_request\n");
	return &req->req;
}

static void
FTC_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct FTC_request *req;

	DBG_FUNCC("+FTC_free_request()\n");

	if (!_ep || !_req)
		return;

	req = container_of(_req, struct FTC_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/*-------------------------------------------------------------------------*/

/* many common platforms have dma-coherent caches, which means that it's
 * safe to use kmalloc() memory for all i/o buffers without using any
 * cache flushing calls.  (unless you're trying to share cache lines
 * between dma and non-dma activities, which is a slow idea in any case.)
 *
 * other platforms need more care, with 2.6 having a moderately general
 * solution except for the common "buffer is smaller than a page" case.
 */
#undef USE_KMALLOC		// right now we chose not to use kmalloc() to get memory buffer
//#define USE_KMALLOC

/* allocating buffers this way eliminates dma mapping overhead, which
 * on some platforms will mean eliminating a per-io buffer copy.  with
 * some kinds of system caches, further tweaks may still be needed.
 */
static void *
FTC_alloc_buffer(struct usb_ep *_ep, unsigned bytes, dma_addr_t * dma, gfp_t gfp_flags)
{
	void *retval;
	struct FTC_ep *ep;

	DBG_FUNCC("+FTC_alloc_buffer():ep = 0x%x\n", (u32) _ep);

	ep = container_of(_ep, struct FTC_ep, ep);
	if (!_ep)
		return 0;

	*dma = DMA_ADDR_INVALID;

#if defined(USE_KMALLOC)
	retval = kmalloc(bytes, gfp_flags);

	if (retval)
		*dma = virt_to_phys(retval);
#else
	/* one problem with this call is that it wastes memory on
	 * typical 1/N page allocations: it allocates 1-N pages.
	 * another is that it always uses GFP_ATOMIC.
	 */
#warning Using pci_alloc_consistent even with buffers smaller than a page.
	retval = pci_alloc_consistent(NULL, bytes, dma);
#endif
	return retval;
}

static void
FTC_free_buffer(struct usb_ep *_ep, void *buf, dma_addr_t dma, unsigned bytes)
{

#ifndef	USE_KMALLOC
	struct FTC_ep *ep;
	/* free memory into the right allocator */
	DBG_FUNCC("+FTC_free_buffer()\n");
	ep = container_of(_ep, struct FTC_ep, ep);
	if (!_ep)
		return;
	/* one problem with this call is that some platforms
	 * don't allow it to be used in_irq().
	 */
	pci_free_consistent((struct pci_dev *) ep->dev, bytes, buf, dma);
#else
	DBG_FUNCC("+FTC_free_buffer()\n");
	kfree(buf);
#endif
}

/*-------------------------------------------------------------------------*/
// finish/abort one request
static void
done(struct FTC_ep *ep, struct FTC_request *req, int status)
{
	struct FTC_udc *dev;
	unsigned stopped = ep->stopped;
	u32 temp;

	DBG_FUNCC("+done()\n");

	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))	// still ongoing
		req->req.status = status;
	else			// has finished
		status = req->req.status;

	dev = ep->dev;
	if (req->mapped)	// DMA mapped
	{
		DBG_CTRLL("....pci_unmap_single len = %d\n", req->req.length);

		// important : DMA length will set as 16*n bytes
		temp = req->req.length / 16;
		if (req->req.length % 16)
			temp++;
		pci_unmap_single((void *) dev, req->req.dma, temp * 16,	//USB_EPX_BUFSIZ,  //req->req.length+32,
				 ep->is_in ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);
		req->req.dma = DMA_ADDR_INVALID;
		req->mapped = 0;
	}
#ifndef USB_TRACE
	if (status && status != -ESHUTDOWN)
#endif
		VDBG(dev, "complete %s req %p stat %d len %u/%u\n",
		     ep->ep.name, &req->req, status, req->req.actual, req->req.length);

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;

	if (ep->num > 0)
		FIFO_Int(ep, 0);	//Bruce;;Disable FIFO interrupt

	spin_unlock(&dev->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&dev->lock);
	if (ep->num == 0)
		mUsbEP0DoneSet();
	ep->stopped = stopped;	//recover 

	//Bruce;;
	if (!list_empty(&ep->queue))
		FIFO_Int(ep, 1);	//Bruce;;Enable FIFO interrupt

	DBG_FUNCC("-done() stopped=%d\n", stopped);
}

/*-------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
//              vUsbDxFWr()
//              Description: Write the buffer content to USB220 PAM through data port
//              input: Buffer pointer, byte number to write, Change Endian type or NOT
//              output: this time write data length 
///////////////////////////////////////////////////////////////////////////////
static int
vUsbDxFWr(u8 FIFONum, u8 * pu8Buffer, u16 u16Num)
{
	u16 u16_i;
	u8 *pp;
	u8 *pBufPtrOrg;

	DBG_FUNCC("+vUsbDxFWr()(u16Num=%d)\n", u16Num);

	if (u16Num <= 0)
		return u16Num;
	pBufPtrOrg = NULL;

	//Check if start address is 4 bytes alignment
	// if it's not, must malloc a new buffer, and set to 4 byte alignment.
	if ((((u32) pu8Buffer) % 4) != 0) {
		DBG_BULKK("Data Ptr not alignment, malloc a temp buf.\n");
		pBufPtrOrg = kmalloc(u16Num + 4, GFP_KERNEL);
		pp = pBufPtrOrg;
		while ((((u32) pp) % 4) != 0)
			pp = pp + 1;
		memcpy(pp, pu8Buffer, u16Num);
	} else
		pp = pu8Buffer;

	for (u16_i = (u16Num >> 2); u16_i > 0; u16_i--) {
		mUsbWrDWord(FIFONum, *((u32 *) (pp)));
		pp = pp + 4;
	}

	switch (u16Num % 4) {
	case 1:
		mUsbWrByte0(FIFONum, *((u8 *) pp));
		break;
	case 2:
		mUsbWrWord(FIFONum, *((u16 *) (pp)));
		break;
	case 3:
#if (USB_DataPort == NoFixPort)
		// for this version of data port, if want to write data length= 3 bytes,
		// we must write first 2 bytes in offset 0, and last byte write to offset 2.
		mUsbWrWord(FIFONum, *((u16 *) (pp)));
		pp = pp + 2;

		mUsbWrByte2(FIFONum, *((u8 *) pp));
#elif(USB_DataPort == FixPort)
		// for newest version of data port, if want to write data length= 3 bytes,
		// we must write first 2 bytes in offset 0, and last byte still write to offset 0.
		mUsbWrWord(FIFONum, *((u16 *) (pp)));
		pp = pp + 2;

		mUsbWrByte0(FIFONum, *((u8 *) pp));
#endif
		break;
	default:
		break;
	}
	mUsbFIFODone(FIFONum);

	// if create a buffer, free it now.
	//if((((u32) pu8Buffer) % 4)!=0)
	if (pBufPtrOrg != NULL)
		kfree(pBufPtrOrg);

	return u16Num;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsbDxFRd()
//              Description: Read FUSB220 FIFO data using PIO mode
//              input: Buffer pointer, byte number to write, Change Endian type or NOT
//              output: this time read data length 
///////////////////////////////////////////////////////////////////////////////
static int
vUsbDxFRd(u8 FIFONum, u8 * pu8Buffer, u16 u16Num)
{
	u16 u16_i;
	u8 *pp;
	u8 *pBufPtrOrg;
	u8 *pBufPtrSave;
	u32 dwTemp;
	u32 *dwpp;
	DBG_FUNCC("+vUsbDxFRd() : byte = %d\n", u16Num);
	if (u16Num <= 0)
		return u16Num;
	pBufPtrOrg = NULL;
	pBufPtrSave = NULL;

	//Check if start address is 4 bytes alignment ?
	// if it's not, must malloc a new buffer, and set to 4 byte alignment.
	if ((((u32) pu8Buffer) % 4) != 0) {
		DBG_BULKK("Data Ptr not alignment, malloc a temp buf.\n");
		pBufPtrOrg = kmalloc(u16Num + 4, GFP_KERNEL);
		pp = pBufPtrOrg;
		while ((((u32) pp) % 4) != 0)
			pp = pp + 1;
		pBufPtrSave = pp;
	} else
		pp = pu8Buffer;

	DBG_BULKK("FIFO DATA=");

	// read data use 4 bytes each time.
	dwpp = (u32 *) pp;
	for (u16_i = (u16Num >> 2); u16_i > 0; u16_i--) {
		dwTemp = mUsbRdDWord(FIFONum);
		DBG_BULKK("0x%08x,", dwTemp);
		*dwpp++ = dwTemp;
	}

	// translate data pointer record from 4 bytes to 1 byte, and read left data from FIFO   
	pp = (u8 *) dwpp;
	switch (u16Num & 0x3) {
	case 1:
		dwTemp = mUsbRdDWord(FIFONum);
		*(pp) = (u8) (dwTemp);
		DBG_BULKK("0x%08x,", dwTemp);
		break;
	case 2:
		dwTemp = mUsbRdDWord(FIFONum);
		*(pp) = (u8) (dwTemp);
		*(pp + 1) = (u8) (dwTemp >> 8);
		DBG_BULKK("0x%08x,", dwTemp);
		break;
	case 3:
		dwTemp = mUsbRdDWord(FIFONum);
		*(pp) = (u8) (dwTemp);
		*(pp + 1) = (u8) (dwTemp >> 8);
		*(pp + 2) = (u8) (dwTemp >> 16);
		DBG_BULKK("0x%08x,", dwTemp);
		break;
	default:
		break;
	}
	DBG_BULKK("\n");

	// if create a buffer, free it now.
	//if((((u32) pu8Buffer) % 4)!=0)
	if (pBufPtrOrg != NULL) {
		DBG_BULKK("Copy temp buf data to real buf.\n");
		memcpy(pu8Buffer, pBufPtrSave, u16Num);
		kfree(pBufPtrOrg);
	}

	return u16Num;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsbCxFWr()
//              Description: Write the buffer content to USB220 CxF using PIO mode
//              input: Buffer pointer, byte number to write
//              output: this time write data length 
///////////////////////////////////////////////////////////////////////////////
static int
vUsbCxFWr(u8 * pu8Buffer, u16 u16Num)
{
	u16 u16_i;
	u8 *pp;
	u8 *pBufPtrOrg;
	u16 wTemp;
	u8 bTemp;
	u32 *dwpp;

	DBG_FUNCC("+vUsbCxFWr()(Len=%d)\n", u16Num);
	if (u16Num == 0)
		return 0;
	pBufPtrOrg = NULL;
	while (!mUsbIntBufEmptyRd()) ;
	DBG_CTRLL("Cx config and status:0x%x\n", bFUSBPort(0x0B));

	//Check if start address is 4 bytes alignment
	// if it's not, must malloc a new buffer, and set to 4 byte alignment.
	if ((((u32) pu8Buffer) % 4) != 0) {
		DBG_CTRLL("Data Ptr not alignment, malloc a temp buf.\n");
		pBufPtrOrg = kmalloc(u16Num + 4, GFP_KERNEL);
		pp = pBufPtrOrg;
		while ((((u32) pp) % 4) != 0)
			pp = pp + 1;
		memcpy(pp, pu8Buffer, u16Num);
	} else
		pp = pu8Buffer;

	DBG_CTRLL("Write data into Cx FIFO.\n");

	dwpp = (u32 *) pp;
	for (u16_i = (u16Num >> 2); u16_i > 0; u16_i--) {
		//while((bFUSBPort(0x0B)&0x10));// CX FIFO is not full
		mUsbEP0DataWrDWord(*dwpp);
		dwpp++;
	}

	pp = (u8 *) dwpp;

	//while((bFUSBPort(0x0B)&0x10)); // CX FIFO is not full
	switch (u16Num % 4) {
	case 1:
		bTemp = *(pp);
		mUsbEP0DataWrByte0(bTemp);
		break;
	case 2:
		wTemp = ((u16) (*(pp + 1)));
		wTemp = (wTemp << 8) | ((u16) (*(pp)));
		mUsbEP0DataWrWord(wTemp);
		break;
	case 3:
		wTemp = (u16) (*(pp + 1));
		wTemp = (wTemp << 8) | (u16) (*(pp));
		mUsbEP0DataWrWord(wTemp);

		pp = pp + 2;
#if (USB_DataPort == NoFixPort)
		mUsbEP0DataWrByte2((*(pp)));
#elif(USB_DataPort == FixPort)
		mUsbEP0DataWrByte0((*(pp)));
#endif
		break;
	default:
		break;
	}

	// if create a buffer, free it now.
	//if((((u32) pu8Buffer) % 4)!=0)
	if (pBufPtrOrg != NULL)
		kfree(pBufPtrOrg);

	DBG_CTRLL("Exit Cx Write.\n");
	return u16Num;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsbCxFRd()
//              Description: Fill the buffer from USB200 via Dbus
//              input: Buffer pointer, byte number to write, Change Endian type or NOT
//              output: this time read data length 
///////////////////////////////////////////////////////////////////////////////
static int
vUsbCxFRd(u8 * pu8Buffer, u16 u16Num)
{
	u16 u16_i;
	u8 *pp;
	u8 *pBufPtrOrg;
	u8 *pBufPtrSave;
	u32 dwTemp;
	u16 wTemp;
	u32 *dwpp;

	DBG_FUNCC("+vUsbCxFRd()(Len=%d)\n", u16Num);
	pBufPtrOrg = NULL;
	pBufPtrSave = NULL;

	// Check if start address is 4 bytes alignment ?
	// if it's not, must malloc a new buffer, and set to 4 byte alignment.
	if ((((u32) pu8Buffer) % 4) != 0) {
		DBG_CTRLL("Data Ptr not alignment, malloc a temp buf.\n");
		pBufPtrOrg = kmalloc(u16Num + 4, GFP_KERNEL);
		pp = pBufPtrOrg;
		while ((((u32) pp) % 4) != 0)
			pp = pp + 1;
		pBufPtrSave = pp;
	} else
		pp = pu8Buffer;

	DBG_CTRLL("Read data from Cx FIFO.\n");
	dwpp = (u32 *) pp;
	for (u16_i = (u16Num >> 2); u16_i > 0; u16_i--) {
		//while((bFUSBPort(0x21)& BIT2));               // Check CX FIFO is not empty, if it's empty, wait host send data.
		dwTemp = mUsbEP0DataRdDWord();
		*dwpp = dwTemp;
		dwpp++;
	}
	pp = (u8 *) dwpp;

	//while((bFUSBPort(0x21)& BIT2));           // Check CX FIFO is not empty, if it's empty, wait host send data.
	switch (u16Num % 4) {
	case 1:
		*(pp) = mUsbEP0DataRdByte0();
		break;
	case 2:
		wTemp = mUsbEP0DataRdWord();
		*pp++ = (u8) wTemp;
		*pp = (u8) (wTemp >> 8);
		break;
	case 3:
		wTemp = mUsbEP0DataRdWord();
		*pp = (u8) wTemp;
		*(pp + 1) = (u8) (wTemp >> 8);
		pp = pp + 2;

#if (USB_DataPort == NoFixPort)
		*(pp) = mUsbEP0DataRdByte2();
#elif(USB_DataPort == FixPort)
		*(pp) = mUsbEP0DataRdByte0();
#endif
		break;
	default:
		break;
	}

	// if create a buffer, save data into real buffer and free it now.
	//if((((u32) pu8Buffer) % 4)!=0)
	if (pBufPtrOrg != NULL) {
		DBG_CTRLL("Copy temp buf data to real buf.\n");
		memcpy(pu8Buffer, pBufPtrSave, u16Num);
		kfree(pBufPtrOrg);
	}
	DBG_CTRLL("Exit Cx Read.\n");
	return u16Num;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsbCxFRd()
//              Description: Fill the buffer from USB200 via Dbus
//              input: Buffer pointer, byte number to write, Change Endian type or NOT
//              output: this time read data length 
///////////////////////////////////////////////////////////////////////////////
static int
vUsbCxFRd8ByteCmd(u8 * pu8Buffer)
{
	u16 u16_i;
	u8 *pp;
	u8 *pBufPtrOrg;
	u8 *pBufPtrSave;
	u32 dwTemp;
	u16 wTemp;
	u32 *dwpp;
	u16 u16Num = 8;

	DBG_FUNCC("+vUsbCxFRd8ByteCmd()\n");
	pBufPtrOrg = NULL;
	pBufPtrSave = NULL;

	// Check if start address is 4 bytes alignment ?
	// if it's not, must malloc a new buffer, and set to 4 byte alignment.
	if ((((u32) pu8Buffer) % 4) != 0) {
		DBG_CTRLL("Data Ptr not alignment, malloc a temp buf.\n");
		pBufPtrOrg = kmalloc(u16Num + 4, GFP_KERNEL);
		pp = pBufPtrOrg;
		while ((((u32) pp) % 4) != 0)
			pp = pp + 1;
		pBufPtrSave = pp;
	} else
		pp = pu8Buffer;

	DBG_CTRLL("Read data from Cx FIFO.\n");
	dwpp = (u32 *) pp;
	for (u16_i = (u16Num >> 2); u16_i > 0; u16_i--) {
		//while((bFUSBPort(0x0B)& BIT5));               // Check CX FIFO is not empty, if it's empty, wait host send data.
		dwTemp = mUsbEP0DataRdDWord();
		*dwpp = dwTemp;
		dwpp++;
	}
	pp = (u8 *) dwpp;

	//while((bFUSBPort(0x0B)& BIT5));           // Check CX FIFO is not empty, if it's empty, wait host send data.
	switch (u16Num % 4) {
	case 1:
		*(pp) = mUsbEP0DataRdByte0();
		break;
	case 2:
		wTemp = mUsbEP0DataRdWord();
		*pp++ = (u8) wTemp;
		*pp = (u8) (wTemp >> 8);
		break;
	case 3:
		wTemp = mUsbEP0DataRdWord();
		*pp = (u8) wTemp;
		*(pp + 1) = (u8) (wTemp >> 8);
		pp = pp + 2;

#if (USB_DataPort == NoFixPort)
		*(pp) = mUsbEP0DataRdByte2();
#elif(USB_DataPort == FixPort)
		*(pp) = mUsbEP0DataRdByte0();
#endif
		break;
	default:
		break;
	}

	// if create a buffer, save data into real buffer and free it now.
	//if((((u32) pu8Buffer) % 4)!=0)
	if (pBufPtrOrg != NULL) {
		DBG_CTRLL("Copy temp buf data to real buf.\n");
		memcpy(pu8Buffer, pBufPtrSave, u16Num);
		kfree(pBufPtrOrg);
	}
	DBG_CTRLL("Exit Cx Read.\n");
	return u16Num;
}

// return:  0 = still running, 1 = completed, negative = errno
static int
write_fifo(struct FTC_ep *ep, struct FTC_request *req)
{
	struct FTC_udc *dev = ep->dev;
	u8 *buf;
	int is_last;
	u8 u8fifo_n, usb_interrupt_Source;
	unsigned length, count;
	u8 usb_interrupt_reg;

	DBG_FUNCC("+write_fifo() : actual = 0x%x\n", req->req.actual);

write_fifo_top:

	buf = req->req.buf + req->req.actual;	// current location
	prefetch(buf);

	dev = ep->dev;
	if (unlikely(ep->num == 0 && dev->ep0state != EP0_IN))
		return -EL2HLT;

	if (ep->num > 0) {
		// EP1,2,3...
		u8fifo_n = mUsbEPMapRd(ep->num);	// get the relatived FIFO number
		if (ep->is_in)
			u8fifo_n &= 0x0F;
		else
			u8fifo_n >>= 4;
		if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
			return -EINVAL;

		// Check the FIFO had been enable ?
		if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
			return -EINVAL;
	} else
		u8fifo_n = 0;	//useless

	length = FTC_MIN(req->req.length - req->req.actual, ep->ep.maxpacket);
	req->req.actual += length;
	if (ep->num == 0)	//EP0
		count = vUsbCxFWr(buf, length);	// return this time real transfer length
	else {
		//For Bulk-in Interrupt Enable/Disable
		if (req->req.length <= (req->req.actual))
			FIFO_Int(ep, 0);	//Disable the Bulk-In, Before write last packet
		else
			FIFO_Int(ep, 1);	//Enable the Bulk-In, Because Remain Size>0
		count = vUsbDxFWr(u8fifo_n, buf, length);
		if (unlikely(count != length))
			ERROR(dev, "Write FIFO Fail(count!=length)...\n");
	}

	/* last packet often short (sometimes a zlp, especially on ep0) */
	// If this time real transfer length!= max packet size, this time is a last packet.
	if ((unlikely(count != ep->ep.maxpacket))) {
		if (ep->num == 0) {
//                      dev->ep[0].stopped = 1;
			dev->ep0state = EP0_STATUS;
		}
		is_last = 1;
	} else {
		if (likely(req->req.length != req->req.actual))	//|| req->req.zero)
			is_last = 0;
		else
			is_last = 1;
	}
	/* requests complete when all IN data is in the FIFO,
	 * or sometimes later, if a zlp was needed.
	 */
	// In here we also need to check if there is a reset or suspend int, must stop this transmit.
	usb_interrupt_reg = mUsbIntByte7Rd();
	if ((is_last) || (usb_interrupt_reg & (BIT1 | BIT2))) {
		done(ep, req, 0);
		if (usb_interrupt_reg & (BIT1 | BIT2))
			printk("There is a Rst/Susp int when write_fifo(), exit this function.\n");
		return 1;
	} else {		//Bruce
		//For Host-In ==> Write FIFO
		if (ep->num > 0) {
			usb_interrupt_Source = mUsbIntByte5Rd();	//For Host-In 
			if (usb_interrupt_Source & (BIT0))	//For Host-In (FIFO-Empty)
				goto write_fifo_top;
		}
		if (ep->num == 0) {
			while (!(bFUSBPort(0x21) & BIT1)) ;	// if this is not the last cx packet and CX IN interrupt not rising,
			// wait for CX IN interrupt rising
			goto write_fifo_top;
		}
	}

	return 0;
}

static int
read_fifo(struct FTC_ep *ep, struct FTC_request *req)
{
	u32 size;
	u8 *buf;
	unsigned is_short;
	u8 u8fifo_n, usb_interrupt_Source;
	int count;
	unsigned bufferspace;
	u8 usb_interrupt_reg;
	DBG_FUNCC("+read_fifo(): len = 0x%x, actual = 0x%x\n", req->req.length, req->req.actual);

      read_fifo_top:
	buf = req->req.buf + req->req.actual;
	prefetchw(buf);

	//EP0 should be OUT stage
	if (unlikely(ep->num == 0 && ep->dev->ep0state != EP0_OUT))
		return -EL2HLT;

	if (ep->num > 0) {
		// EP1,2,3...
		u8fifo_n = mUsbEPMapRd(ep->num);	// get the relatived FIFO number
		if (ep->is_in)
			u8fifo_n &= 0x0F;
		else
			u8fifo_n >>= 4;
		if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
			return -EINVAL;

		// Check the FIFO had been enable ?
		if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
			return -EINVAL;
	} else
		u8fifo_n = 0;	//useless

	bufferspace = req->req.length - req->req.actual;
	if (likely(ep->num != 0)) {
		// For non-ep0 endpoint, we must read FIFO byte Cnt to deceide
		// data length we are going to read
		if (bufferspace != 0)
			size = mUsbFIFOOutByteCount(u8fifo_n);
		else
			size = 0;
	} else {
		// because there is no byte counter for cx fifo, so we must count 
		// this time transmit data length
		size = FTC_MIN(req->req.length - req->req.actual, ep->ep.maxpacket);
	}

	req->req.actual += size;
	is_short = (size < ep->ep.maxpacket);

#ifdef USB_TRACE
	VDBG(ep->dev, "read %s %u bytes%s OUT req %p %u/%u\n",
	     ep->ep.name, size, is_short ? "/S" : "", req, req->req.actual, req->req.length);
#endif
	if (ep->num == 0) {	//EP0
		if (size > 0) {
			while (!(bFUSBPort(0x21) & BIT2)) ;	// wait for CX OUT interrupt rising     
			count = vUsbCxFRd(buf, size);
		}
	} else {
		//For Bulk-Out Interrupt Enable/Disable
		count = vUsbDxFRd(u8fifo_n, buf, size);
		if (req->req.length > req->req.actual)
			FIFO_Int(ep, 1);	//Enable the Bulk-Out, Because Remain Size>0
		else
			FIFO_Int(ep, 0);	//Disable the Bulk-Out, Because Remain Size=0
	}

	/* completion */
	// In here we also need to check if there is a reset or suspend int, must stop this transmit.
	usb_interrupt_reg = mUsbIntByte7Rd();
	if ((unlikely(is_short || (req->req.actual == req->req.length))) || (usb_interrupt_reg & (BIT1 | BIT2))) {
		if (unlikely(ep->num == 0)) {
			/* ep0out status stage */
			ep->stopped = 1;
			ep->dev->ep0state = EP0_STATUS;
		}

		done(ep, req, 0);

		if (usb_interrupt_reg & (BIT1 | BIT2)) {
			printk("There is a Rst/Susp int when read_fifo(), exit this function.\n");
			return 1;
		}
		//Accelerate Bulk Out EP=> Check the Bulk OUT FIFO Full
		// and read Bulk OUT FIFO again (only for Bulk transfer)
		// If next queue already existand BULK OUT interrupt rising,
		// just go to read FIFO again.
		if (ep->num == BULK_OUT_EP_NUM) {
			usb_interrupt_Source = mUsbIntByte1Rd();	//Check BULK-Out Interrupt 
			if (usb_interrupt_Source & (BIT5 | BIT4)) {	//Check BULK-Out Interrupt
				if (!list_empty(&ep->queue)) {
					req = list_entry(ep->queue.next, struct FTC_request, queue);
					goto read_fifo_top;
				}
			}
		}
		return 1;
	} else {
		//Accelerate Bulk Out EP=> Check the Bulk OUT FIFO Full
		// and read Bulk OUT FIFO again (only for Bulk transfer)
		if (ep->num == BULK_OUT_EP_NUM) {
			usb_interrupt_Source = mUsbIntByte1Rd();
			if (usb_interrupt_Source & (BIT5 | BIT4))	//Check BULK-Out Interrupt 
				goto read_fifo_top;
		}
		if (ep->num == 0)
			goto read_fifo_top;
	}
	return 0;
}

static inline void
pio_advance(struct FTC_ep *ep)
{
	struct FTC_request *req;

	DBG_FUNCC("+pio_advance()\n");

	if (unlikely(list_empty(&ep->queue)))
		return;
	DBG_BULKK("FTC_udc => pio_advance() ==> list_entry(ep->queue.next, struct FTC_request, queue)");
	req = list_entry(ep->queue.next, struct FTC_request, queue);
	DBG_BULKK("FTC_udc => pio_advance() ==> ep->is_in ?...");
	(ep->is_in ? write_fifo : read_fifo) (ep, req);
}

#if 0
/*-------------------------------------------------------------------------*/
//============= AHB_DMA function start ==================
// FUSB220 to enable/disable DMA of dedicated EP 
static int
USB_DMA_Enable(struct FTC_ep *ep, unsigned int val)
{
	u8 u8fifo_n;
	u32 offset;
	u8 mval;
	DBG_FUNCC("+USB_DMA_Enable()\n");

	u8fifo_n = mUsbEPMapRd(ep->num);	// get the relatived FIFO number
	if (ep->is_in)
		u8fifo_n &= 0x0F;
	else
		u8fifo_n >>= 4;
	if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
		return -EINVAL;

	offset = 0x7e;
	if (u8fifo_n >= 8) {
		offset++;
		mval = 1 << (u8fifo_n - 8);
	} else {
		mval = 1 << u8fifo_n;
	}

	if (val)		//enable DMA
		mUsbFIFODMAEn(offset, mval);
	else
		mUsbFIFODMADis(offset, mval);
	return 0;
}
#endif

#if 0
// return:  0 = q running, 1 = q stopped, negative = errno
static int
start_dma(struct FTC_udc *dev, struct FTC_ep *ep, struct FTC_request *req)
{
//      struct FTC_udc *dev = ep->dev;
	u32 start;
	//u32                      end = start + req->req.length - 1;
	ahb_dma_parm_t parm;
	u8 u8fifo_n;
	u32 temp;

	DBG_FUNCC("+start_dma()\n");

	/* set up dma mapping in case the caller didn't set */
	if (req->req.dma == DMA_ADDR_INVALID) {
		DBG_TEMP("....pci_map_single len = %d\n", req->req.length);

		// important : DMA length will set as 16*n bytes
		temp = req->req.length / 16;
		if (req->req.length % 16)
			temp++;
		req->req.dma = pci_map_single((void *) dev, req->req.buf, temp * 16,	//USB_EPX_BUFSIZ,  
					      ep->is_in ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);
		req->mapped = 1;
	}
	start = req->CurDmaStartAddr;

	// EP1,2,3...
	u8fifo_n = mUsbEPMapRd(ep->num);	// get the relatived FIFO number
	if (ep->is_in)
		u8fifo_n &= 0x0F;
	else
		u8fifo_n >>= 4;
	if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
		return -EINVAL;

	// Check the FIFO had been enable ?
	if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
		return -EINVAL;

	dev->EPUseDMA = ep->num;
	dev->ReqForDMA = req;
	// If use DMA, no FIFO interrupt for FIFO
	FIFO_Int(ep, 0);

	// We must seperate into several transmit times.
	// It's because AHB_DMA max tx length is only 8k bytes.
	if (req->req.actual < req->req.length) {
		if ((req->req.length - req->req.actual) > AHB_DMA_MAX_LEN)
			req->u32DMACurrTxLen = AHB_DMA_MAX_LEN;
		else
			req->u32DMACurrTxLen = req->req.length - req->req.actual;
	}
	DBG_FUNCC("+start_dma, start addr= 0x%x,total len=%d, Cur len=%d, Dir=%s\n",
		  start, req->req.length, req->u32DMACurrTxLen, (ep->is_in) ? "IN" : "OUT");

	/* re-init the bits affecting IN dma; careful with zlps */
	DBG_TEMP("start_dma()-fill AHB dma structure.\n");
	if (likely(ep->is_in)) {
		parm.src = start;
		parm.dest = (u32) (&(mDMAUsbRdDWord(u8fifo_n)));
		parm.sw = AHBDMA_WIDTH_32BIT;
		parm.dw = AHBDMA_WIDTH_32BIT;
		parm.sctl = AHBDMA_CTL_INC;
		parm.dctl = AHBDMA_CTL_FIX;
		parm.size = req->u32DMACurrTxLen >> 2;	//beacuse we xfr 4 bytes/each time
		parm.irq = AHBDMA_TRIGGER_IRQ;
	} else {
		parm.src = (u32) (&(mDMAUsbRdDWord(u8fifo_n)));
		parm.dest = start;
		parm.sw = AHBDMA_WIDTH_32BIT;
		parm.dw = AHBDMA_WIDTH_32BIT;
		parm.sctl = AHBDMA_CTL_FIX;
		parm.dctl = AHBDMA_CTL_INC;
		parm.size = req->u32DMACurrTxLen >> 2;	//beacuse we xfr 4 bytes/each time
		parm.irq = AHBDMA_TRIGGER_IRQ;
	}
	DBG_TEMP("start_dma()-add AHB dma structure to AHB link.(ahb_dma=0x%x)\n", (u32) dev->ahb_dma);
	ahb_dma_add(dev->ahb_dma, &parm);

	DBG_TEMP("start_dma()-enable FUSB220 DMA reg.\n");
	USB_DMA_Enable(ep, 1);

	DBG_TEMP("start_dma()-start AHB dma.\n");
	ahb_dma_start(dev->ahb_dma);
	ep->dma_running = TRUE;

	// important : Some A320d platform quality is not good,
	// if use DMA mode to transmit data fail, please add mdelay() as bellow.
	// Then transmit fail problem maybe fixed.
	//  mdelay(20);

	DBG_FUNCC("-start_dma finish\n");
	return 0;
}
#endif

#if 0
///////////////////////////////////////////////////////////////////////////////
//              vCheckDMA()
//              Description: This function is just like the end of read_fifo() or write_fifo()
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vCheckDMA(struct FTC_udc *dev)
{
	u8 bDMA_Finish = FALSE;
	u32 IntStatus;
	struct FTC_ep *ep;
	struct FTC_request *req;
	u8 u8fifo_n;

	DBG_TEMP("vCheckDMA()-AHB dma prcess finish.\n");
	IntStatus = ahbdma_get_status(dev->ahb_dma);

	if (IntStatus & INT_ERROR) {
		printk("FUSB220 FIFO%d use AHB DMA Error\n", dev->EPUseDMA);
		bDMA_Finish = TRUE;
	} else if (IntStatus & INT_TRIGGER) {
		bDMA_Finish = TRUE;
	} else
		DBG_TEMP_udc("Error: Rising DMA int, but no finish or error status\n");

	if (bDMA_Finish) {
		// Get endpoint information
		ep = &dev->ep[dev->EPUseDMA];
		// Get endpoint request for DMA
		req = dev->ReqForDMA;

		req->req.actual += req->u32DMACurrTxLen;	//just set "actual transmit length"="request length"
		req->CurDmaStartAddr += req->u32DMACurrTxLen;
		if (req->req.actual < req->req.length) {
			// if DMA not yet finish the job, just re-open ep int, and wait 
			// interrupt to enable dma
			FIFO_Int(ep, 1);
		} else {
			// Use DMA EP NUM to get relatived FIFO number
			u8fifo_n = mUsbEPMapRd(dev->EPUseDMA);
			if (ep->is_in)
				u8fifo_n &= 0x0F;
			else
				u8fifo_n >>= 4;

			if (u8fifo_n >= FUSB220_MAX_FIFO) {	// over the Max. fifo count ?
				printk("....OOP: vCheckDMA() FIFO out of range\n");
				return;
			}

			if (ep->is_in) {
				// if it's IN EP, must set done to finish this transfer.
				mUsbFIFODone(u8fifo_n);
			} else {
				// out endpoint 
				if ((mUsbIntByte1Rd() & (BIT5 | BIT4)))	//check HW error
					DBG_BULKK("FUSB220 Bulk OUT use DMA finish, but still have %d bytes in FIFO\n",
						  mUsbFIFOOutByteCount(u8fifo_n));
			}

			dev->EPUseDMA = FUSB220_DMA_IS_IDLE_NOW;	//reset
			USB_DMA_Enable(ep, 0);
			done(ep, req, 0);
			dev->ReqForDMA = 0;
		}
		ep->dma_running = FALSE;
	}
}

static irqreturn_t
dma_interrupt_handler(int irq, void *_dev, struct pt_regs *dummy)
{
	struct FTC_udc *dev = _dev;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);

	//printk("dev addr = 0x%x\n", (u32) dev);
	//printk("struc = 0x%x\n", (u32) dev->ahb_dma);
	//printk("DMA base = 0x%x\n", dev->ahb_dma->base);
	//printk("DMA channel base = 0x%x\n", dev->ahb_dma->channel_base);
	vCheckDMA(dev);

	// clear interrupt only after processing 
	ahbdma_clear_int(dev->ahb_dma);
	spin_unlock_irqrestore(&dev->lock, flags);

	return IRQ_RETVAL(1);
}

//==================== AHB DMA function finish ==============
#endif

static int
FTC_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct FTC_request *req;
	struct FTC_ep *ep;
	struct FTC_udc *dev;
	unsigned long flags;
	int status;
	int temp;
	//Remove;;u32 temp;

	DBG_FUNCC("+FTC_queue()\n");

	/* always require a cpu-view buffer so pio works */
	req = container_of(_req, struct FTC_request, req);
	if (unlikely(!_req || !_req->complete || !_req->buf || !list_empty(&req->queue))) {
		if (!_req)
			printk("FTC_queue() return fail-1-1,_req=0x%x \n", (u32) _req);
		if (!_req->complete)
			printk("FTC_queue() return fail-1-2,_req->complete=0x%x \n", (u32) _req->complete);
		if (!_req->buf)
			printk("FTC_queue() return fail-1-3,_req->buf=0x%x \n", (u32) _req->buf);
		if (!list_empty(&req->queue))
			printk("FTC_queue() return fail-1-4\n");

		printk("FTC_queue() return fail-1\n");
		return -EINVAL;
	}
	ep = container_of(_ep, struct FTC_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->num != 0))) {
		printk("FTC_queue() return fail-2\n");
		return -EINVAL;
	}
	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN))
		return -ESHUTDOWN;

	/* can't touch registers when suspended */
	if (dev->ep0state == EP0_SUSPEND) {
		printk("FTC_queue() return fail (ep0state == EP0_SUSPEND) \n");
		return -EBUSY;
	}
#ifdef USB_TRACE
	VDBG(dev, "%s queue req %p, len %u buf %p\n", _ep->name, _req, _req->length, _req->buf);
#endif

	spin_lock_irqsave(&dev->lock, flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* for ep0 IN without premature status, zlp is required and
	 * writing EOP starts the status stage (OUT).
	 */
	if (unlikely(ep->num == 0 && ep->is_in))
		_req->zero = 1;

	/* kickstart this i/o queue? */
	status = 0;

	//Bruce;;if (list_empty(&ep->queue) && likely(!ep->stopped)) {
	//In  => Write data to the FIFO directly
	//Out => Only Enable the FIFO-Read interrupt 

	temp = list_empty(&ep->queue);
	if (list_empty(&ep->queue) && likely(!ep->stopped)) {
		/* dma:  done after dma completion IRQ (or error)
		 * pio:  done after last fifo operation
		 */
		if (ep->num > 0)
			FIFO_Int(ep, 1);	//Enable the Bulk-Out, Because Remain Size>0
		else {
			//For Bulk-In or ep->num=0
			status = (ep->is_in ? write_fifo : read_fifo) (ep, req);
			if (unlikely(status != 0)) {
				if (status > 0)
					status = 0;
				req = 0;
			}
		}
	}
	/* else pio or dma irq handler advances the queue. */
	if (likely(req != 0)) {
		DBG_CTRLL("add request into queue(ep->stopped=%d)(list_empty(&ep->queue)=%d)\n", (int) (ep->stopped),
			  temp);
		list_add_tail(&req->queue, &ep->queue);
	}

	spin_unlock_irqrestore(&dev->lock, flags);
	return status;
}

/* dequeue ALL requests */
static void
nuke(struct FTC_ep *ep, int status)
{
	struct FTC_request *req;
	DBG_FUNCC("+nuke() ep addr= 0x%x\n", (u32) ep);

	ep->stopped = 1;
	if (list_empty(&ep->queue))
		return;
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct FTC_request, queue);
		printk("release req = %x\n", (u32) req);
		done(ep, req, status);
	}
}

/* dequeue JUST ONE request */
static int
FTC_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct FTC_request *req;
	struct FTC_ep *ep;
	struct FTC_udc *dev;
	unsigned long flags;

	DBG_FUNCC("+FTC_dequeue()\n");

	ep = container_of(_ep, struct FTC_ep, ep);
	if (!_ep || !_req || (!ep->desc && ep->num != 0))
		return -EINVAL;
	dev = ep->dev;
	if (!dev->driver)
		return -ESHUTDOWN;

	/* we can't touch (dma) registers when suspended */
	if (dev->ep0state == EP0_SUSPEND)
		return -EBUSY;

	VDBG(dev, "%s %s %s %s %p\n", __FUNCTION__, _ep->name, ep->is_in ? "IN" : "OUT", ep->dma ? "dma" : "pio", _req);

	spin_lock_irqsave(&dev->lock, flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore(&dev->lock, flags);
		return -EINVAL;
	}
	//Remove;;if (ep->dma && ep->queue.next == &req->queue && !ep->stopped) {
	//Remove;;      abort_dma(ep, -ECONNRESET);
	//Remove;;      done(ep, req, -ECONNRESET);
	//Remove;;      dma_advance(dev, ep);
	//Remove;;} 
	//Remove;;else
	if (!list_empty(&req->queue))
		done(ep, req, -ECONNRESET);
	else
		req = 0;
	spin_unlock_irqrestore(&dev->lock, flags);

	return req ? 0 : -EOPNOTSUPP;
}

/*-------------------------------------------------------------------------*/

static void
FTC_clear_halt(struct FTC_ep *ep)
{
	DBG_FUNCC("+FTC_clear_halt()(ep->num=%d)\n", ep->num);

	// assert (ep->num !=0)
	VDBG(ep->dev, "%s clear halt\n", ep->ep.name);

	if (ep->num == 0) {
		ep->dev->ep0state = EP0_IDLE;
		ep->dev->ep[0].stopped = 0;
	} else {
		if (ep->is_in)	// IN direction ?
		{
			DBG_CTRLL("FTC_udc==>FTC_clear_halt()==>IN direction, EP%d \n", ep->num);

			mUsbEPinRsTgSet(ep->num);	// Set Rst_Toggle Bit
			mUsbEPinRsTgClr(ep->num);	// Clear Rst_Toggle Bit
			mUsbEPinStallClr(ep->num);	// Clear Stall Bit
		} else {
			DBG_CTRLL("FTC_udc==>FTC_clear_halt()==>OUT direction, EP%d \n", ep->num);

			mUsbEPoutRsTgSet(ep->num);	// Set Rst_Toggle Bit
			mUsbEPoutRsTgClr(ep->num);	// Clear Rst_Toggle Bit
			mUsbEPoutStallClr(ep->num);	// Clear Stall Bit
		}
	}
	DBG_CTRLL("FTC_udc==>FTC_clear_halt()==>ep->stopped = %d\n", ep->stopped);

	if (ep->stopped) {
		ep->stopped = 0;
		//pio_advance(ep);
	}
}

static int
FTC_set_halt(struct usb_ep *_ep, int value)
{
	struct FTC_ep *ep;
	unsigned long flags;
	int retval = 0;

	DBG_FUNCC("+FTC_set_halt()\n");
	if (!_ep)
		return -ENODEV;
	ep = container_of(_ep, struct FTC_ep, ep);

	DBG_BULKK("FTC_set_halt()===> (ep->num=%d)(Value=%d)\n", ep->num, value);
	//*********** Process the EP-0 SetHalt *******************
	if (ep->num == 0) {
		if (value == 1) {	// protocol stall, need H/W to reset
			mUsbEP0StallSet();
		} else if (value == 2) {	// function stall, SW to set/clear, nad EP0 work normally 
			ep->dev->ep0state = EP0_STALL;
			ep->dev->ep[0].stopped = 1;
		} else if (value == 0) {	// clear function stall, SW to set/clear, nad EP0 work normally 
			ep->dev->ep0state = EP0_IDLE;
			ep->dev->ep[0].stopped = 0;
		}
		return retval;	//EP0 Set Halt will return here          
	} /* don't change EPxSTATUS_EP_INVALID to READY */
	else if (!ep->desc) {
		DBG(ep->dev, "%s %s inactive?\n", __FUNCTION__, ep->ep.name);
		return -EINVAL;
	}
	//*********** Process the EP-X SetHalt *******************

	spin_lock_irqsave(&ep->dev->lock, flags);
	if (!list_empty(&ep->queue))	// something in queue 
		retval = -EAGAIN;
	else if (!value)
		FTC_clear_halt(ep);
	else {
		ep->stopped = 1;
		VDBG(ep->dev, "%s set halt\n", ep->ep.name);

		if (ep->is_in)	// IN direction ?
			mUsbEPinStallSet(ep->num);	// Set in Stall Bit
		else
			mUsbEPoutStallSet(ep->num);	// Set out Stall Bit
	}
	spin_unlock_irqrestore(&ep->dev->lock, flags);
	return retval;
}

//********************************************************
//Name: FTC_fifo_status 
//Description: 
//
//********************************************************
static int
FTC_fifo_status(struct usb_ep *_ep)
{
	struct FTC_ep *ep;
	u8 u8fifo_n;		//john
	u32 size;

	DBG_FUNCC("+FTC_fifo_status()\n");

	if (!_ep)
		return -ENODEV;
	ep = container_of(_ep, struct FTC_ep, ep);

	DBG_BULKK("FTC_udc-->FTC_fifo_status-->Check (size is only reported sanely for OUT)");
	/* size is only reported sanely for OUT */
	if (ep->is_in) {
		DBG_BULKK("FTC_udc-->FTC_fifo_status-->return -EOPNOTSUPP (ep->is_in)");
		return -EOPNOTSUPP;
	}
	//John for FUSB220
	if (ep->num == 0) {	//EP0 
		// note : for EP0, only know empty or not
		size = !mUsbEP0EMPFIFO();
	} else {
		DBG_BULKK("FTC_udc-->FTC_fifo_status-->ep->num >0 ");

		u8fifo_n = mUsbEPMapRd(ep->num);	// get the relatived FIFO number
		if (ep->is_in)
			u8fifo_n &= 0x0F;
		else
			u8fifo_n >>= 4;
		if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
			return -ENOBUFS;

		// Check the FIFO had been enable ?
		if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
			return -ENOBUFS;

		size = mUsbFIFOOutByteCount(u8fifo_n);
		VDBG(ep->dev, "%s %s %u\n", __FUNCTION__, ep->ep.name, size);
	}
	return size;
}

static void
FTC_fifo_flush(struct usb_ep *_ep)
{
	struct FTC_ep *ep;
	u8 u8fifo_n;		//john

	DBG_FUNCC("+FTC_fifo_flush()\n");

	if (!_ep)
		return;
	ep = container_of(_ep, struct FTC_ep, ep);
	VDBG(ep->dev, "%s %s\n", __FUNCTION__, ep->ep.name);

	/* don't change EPxSTATUS_EP_INVALID to READY */
	if (!ep->desc && ep->num != 0) {
		DBG(ep->dev, "%s %s inactive?\n", __FUNCTION__, ep->ep.name);
		return;
	}
	//John for FUSB220
	if (ep->num == 0) {	//EP0 
		mUsbEP0ClearFIFO();
	} else {
		u8fifo_n = mUsbEPMapRd(ep->num);	// get the relatived FIFO number
		if (ep->is_in)
			u8fifo_n &= 0x0F;
		else
			u8fifo_n >>= 4;
		if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
			return;

		// Check the FIFO had been enable ?
		if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
			return;

		mUsbFIFOReset(u8fifo_n);	//reset FIFO
		udelay(10);
		mUsbFIFOResetOK(u8fifo_n);	//reset FIFO finish
	}
	return;
}

static struct usb_ep_ops FTC_ep_ops = {
	.enable = FTC_ep_enable,
	.disable = FTC_ep_disable,

	.alloc_request = FTC_alloc_request,
	.free_request = FTC_free_request,

	.alloc_buffer = FTC_alloc_buffer,
	.free_buffer = FTC_free_buffer,

	.queue = FTC_queue,
	.dequeue = FTC_dequeue,

	.set_halt = FTC_set_halt,
	.fifo_status = FTC_fifo_status,
	.fifo_flush = FTC_fifo_flush,
};

/*-------------------------------------------------------------------------*/

static int
FTC_get_frame(struct usb_gadget *_gadget)
{
	struct FTC_udc *dev;
	u16 retval;
	unsigned long flags;

	DBG_FUNCC("+FTC_get_frame()\n");

	if (!_gadget)
		return -ENODEV;
	dev = container_of(_gadget, struct FTC_udc, gadget);
	spin_lock_irqsave(&dev->lock, flags);
	retval = ((mUsbFrameNoHigh() & 0x07) << 8) | mUsbFrameNoLow();
	spin_unlock_irqrestore(&dev->lock, flags);

	return retval;
}

static int
FTC_wakeup(struct usb_gadget *_gadget)
{
	struct FTC_udc *dev;
	unsigned long flags;
	DBG_FUNCC("+FTC_wakeup()\n");

	if (!_gadget)
		return -ENODEV;
	dev = container_of(_gadget, struct FTC_udc, gadget);
	spin_lock_irqsave(&dev->lock, flags);

	// Set "Device_Remote_Wakeup", Turn on the"RMWKUP" bit in Mode Register
	mUsbRmWkupSet();
	spin_unlock_irqrestore(&dev->lock, flags);
	return 0;
}

static int
FTC_set_selfpowered(struct usb_gadget *_gadget, int value)
{
	DBG_FUNCC("+FTC_set_selfpowered()\n");
	return -EOPNOTSUPP;
}

static int
FTC_ioctl(struct usb_gadget *_gadget, unsigned code, unsigned long param)
{
	unsigned long flags;
	struct FTC_udc *dev;
	struct FTC_ep *ep;
	struct usb_ep *_ep;

	DBG_FUNCC("+FTC_ioctl()\n");

	if (!_gadget)
		return -ENODEV;
	dev = container_of(_gadget, struct FTC_udc, gadget);
	spin_lock_irqsave(&dev->lock, flags);

	switch (code) {
	case 1:		//DMA enable from others
		_ep = (struct usb_ep *) param;
		ep = container_of(_ep, struct FTC_ep, ep);
		ep->dma = 1;
		break;
	case 2:		//DMA disable from others
		_ep = (struct usb_ep *) param;
		ep = container_of(_ep, struct FTC_ep, ep);
		ep->dma = 0;
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&dev->lock, flags);
	return -EOPNOTSUPP;
}

static const struct usb_gadget_ops FTC_ops = {
	.get_frame = FTC_get_frame,
	.wakeup = FTC_wakeup,
	.set_selfpowered = FTC_set_selfpowered,
	.ioctl = FTC_ioctl,
};

/*-------------------------------------------------------------------------*/

static inline char *
dmastr(struct FTC_udc *dev)
{
	DBG_FUNCC("+dmastr()\n");

	if (dev->Dma_Status == PIO_Mode)
		return "(dma disabled)";
	else if (dev->Dma_Status == AHB_DMA)
		return "(AHB DMA)";
	else
		return "(APB DMA)";
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// config FIFO
//-----------------------------------------------------------------------
/////////////////////////////////////////////////////
//              vUsbFIFO_EPxCfg_HS(void)
//              Description:
//                      1. Configure the FIFO and EPx map
//              input: none
//              output: none
/////////////////////////////////////////////////////
static void
vUsbFIFO_EPxCfg_FS(void)
{
	int i;
	DBG_FUNCC("+vUsbFIFO_EPxCfg_FS()\n");

	//EP4
	mUsbEPMap(EP4, FULL_EP4_Map);
	mUsbFIFOMap(FULL_ED4_FIFO_START, FULL_EP4_FIFO_Map);
	mUsbFIFOConfig(FULL_ED4_FIFO_START, FULL_EP4_FIFO_Config);

	for (i = (FULL_ED4_FIFO_START + 1); i < (FULL_ED4_FIFO_START + (FULL_ED4_bBLKNO * FULL_ED4_bBLKSIZE)); i++) {
		mUsbFIFOConfig(i, (FULL_EP4_FIFO_Config & (~BIT7)));
	}
	mUsbEPMxPtSzHigh(EP4, FULL_ED4_bDIRECTION, (FULL_ED4_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP4, FULL_ED4_bDIRECTION, (FULL_ED4_MAXPACKET & 0x7ff));
	mUsbEPinHighBandSet(EP4, FULL_ED4_bDIRECTION, FULL_ED4_MAXPACKET);

	//EP3
	mUsbEPMap(EP3, FULL_EP3_Map);
	mUsbFIFOMap(FULL_ED3_FIFO_START, FULL_EP3_FIFO_Map);
	mUsbFIFOConfig(FULL_ED3_FIFO_START, FULL_EP3_FIFO_Config);

	for (i = FULL_ED3_FIFO_START + 1; i < FULL_ED3_FIFO_START + (FULL_ED3_bBLKNO * FULL_ED3_bBLKSIZE); i++) {
		mUsbFIFOConfig(i, (FULL_EP3_FIFO_Config & (~BIT7)));
	}
	mUsbEPMxPtSzHigh(EP3, FULL_ED3_bDIRECTION, (FULL_ED3_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP3, FULL_ED3_bDIRECTION, (FULL_ED3_MAXPACKET & 0x7ff));
	mUsbEPinHighBandSet(EP3, FULL_ED3_bDIRECTION, FULL_ED3_MAXPACKET);

	//EP2
	mUsbEPMap(EP2, FULL_EP2_Map);
	mUsbFIFOMap(FULL_ED2_FIFO_START, FULL_EP2_FIFO_Map);
	mUsbFIFOConfig(FULL_ED2_FIFO_START, FULL_EP2_FIFO_Config);

	for (i = FULL_ED2_FIFO_START + 1; i < FULL_ED2_FIFO_START + (FULL_ED2_bBLKNO * FULL_ED2_bBLKSIZE); i++) {
		mUsbFIFOConfig(i, (FULL_EP2_FIFO_Config & (~BIT7)));
	}
	mUsbEPMxPtSzHigh(EP2, FULL_ED2_bDIRECTION, (FULL_ED2_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP2, FULL_ED2_bDIRECTION, (FULL_ED2_MAXPACKET & 0x7ff));
	mUsbEPinHighBandSet(EP2, FULL_ED2_bDIRECTION, FULL_ED2_MAXPACKET);

	//EP1
	mUsbEPMap(EP1, FULL_EP1_Map);
	mUsbFIFOMap(FULL_ED1_FIFO_START, FULL_EP1_FIFO_Map);
	mUsbFIFOConfig(FULL_ED1_FIFO_START, FULL_EP1_FIFO_Config);

	for (i = FULL_ED1_FIFO_START + 1; i < FULL_ED1_FIFO_START + (FULL_ED1_bBLKNO * FULL_ED1_bBLKSIZE); i++) {
		mUsbFIFOConfig(i, (FULL_EP1_FIFO_Config & (~BIT7)));
	}

	mUsbEPMxPtSzHigh(EP1, FULL_ED1_bDIRECTION, (FULL_ED1_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP1, FULL_ED1_bDIRECTION, (FULL_ED1_MAXPACKET & 0x7ff));

	mUsbEPinHighBandSet(EP1, FULL_ED1_bDIRECTION, FULL_ED1_MAXPACKET);
}

static void
vUsbFIFO_EPxCfg_HS(void)
{
	int i;
	DBG_FUNCC("+vUsbFIFO_EPxCfg_HS()\n");

	//EP4
	mUsbEPMap(EP4, HIGH_EP4_Map);
	mUsbFIFOMap(HIGH_ED4_FIFO_START, HIGH_EP4_FIFO_Map);
	mUsbFIFOConfig(HIGH_ED4_FIFO_START, HIGH_EP4_FIFO_Config);

	for (i = HIGH_ED4_FIFO_START + 1; i < HIGH_ED4_FIFO_START + (HIGH_ED4_bBLKNO * HIGH_ED4_bBLKSIZE); i++) {
		mUsbFIFOConfig(i, (HIGH_EP4_FIFO_Config & (~BIT7)));
	}
	mUsbEPMxPtSzHigh(EP4, HIGH_ED4_bDIRECTION, (HIGH_ED4_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP4, HIGH_ED4_bDIRECTION, (HIGH_ED4_MAXPACKET & 0x7ff));
	mUsbEPinHighBandSet(EP4, HIGH_ED4_bDIRECTION, HIGH_ED4_MAXPACKET);

	//EP3
	mUsbEPMap(EP3, HIGH_EP3_Map);
	mUsbFIFOMap(HIGH_ED3_FIFO_START, HIGH_EP3_FIFO_Map);
	mUsbFIFOConfig(HIGH_ED3_FIFO_START, HIGH_EP3_FIFO_Config);

	for (i = HIGH_ED3_FIFO_START + 1; i < HIGH_ED3_FIFO_START + (HIGH_ED3_bBLKNO * HIGH_ED3_bBLKSIZE); i++) {
		mUsbFIFOConfig(i, (HIGH_EP3_FIFO_Config & (~BIT7)));
	}
	mUsbEPMxPtSzHigh(EP3, HIGH_ED3_bDIRECTION, (HIGH_ED3_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP3, HIGH_ED3_bDIRECTION, (HIGH_ED3_MAXPACKET & 0x7ff));
	mUsbEPinHighBandSet(EP3, HIGH_ED3_bDIRECTION, HIGH_ED3_MAXPACKET);

	//EP2
	mUsbEPMap(EP2, HIGH_EP2_Map);
	mUsbFIFOMap(HIGH_ED2_FIFO_START, HIGH_EP2_FIFO_Map);
	mUsbFIFOConfig(HIGH_ED2_FIFO_START, HIGH_EP2_FIFO_Config);

	for (i = HIGH_ED2_FIFO_START + 1; i < HIGH_ED2_FIFO_START + (HIGH_ED2_bBLKNO * HIGH_ED2_bBLKSIZE); i++) {
		mUsbFIFOConfig(i, (HIGH_EP2_FIFO_Config & (~BIT7)));
	}
	mUsbEPMxPtSzHigh(EP2, HIGH_ED2_bDIRECTION, (HIGH_ED2_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP2, HIGH_ED2_bDIRECTION, (HIGH_ED2_MAXPACKET & 0x7ff));
	mUsbEPinHighBandSet(EP2, HIGH_ED2_bDIRECTION, HIGH_ED2_MAXPACKET);

	//EP1
	mUsbEPMap(EP1, HIGH_EP1_Map);
	mUsbFIFOMap(HIGH_ED1_FIFO_START, HIGH_EP1_FIFO_Map);
	mUsbFIFOConfig(HIGH_ED1_FIFO_START, HIGH_EP1_FIFO_Config);

	for (i = HIGH_ED1_FIFO_START + 1; i < HIGH_ED1_FIFO_START + (HIGH_ED1_bBLKNO * HIGH_ED1_bBLKSIZE); i++) {
		mUsbFIFOConfig(i, (HIGH_EP1_FIFO_Config & (~BIT7)));
	}
	mUsbEPMxPtSzHigh(EP1, HIGH_ED1_bDIRECTION, (HIGH_ED1_MAXPACKET & 0x7ff));
	mUsbEPMxPtSzLow(EP1, HIGH_ED1_bDIRECTION, (HIGH_ED1_MAXPACKET & 0x7ff));
	mUsbEPinHighBandSet(EP1, HIGH_ED1_bDIRECTION, HIGH_ED1_MAXPACKET);
}

////////////////////////////////////////////////
// check the endpoint to Read/Write Data 
static void
EPX_check_advance(struct FTC_udc *dev, struct FTC_ep *ep)
{
	struct FTC_request *req;
	// u8 ep_num;
	// struct zero_dev *pzero_dev;
	DBG_FUNCC("+EPX_check_advance()\n");

	if (unlikely(list_empty(&ep->queue))) {
		//queue is empty => Disable Interrupt enable
		FIFO_Int(ep, 0);	//Disable Interrupt
	} else {
		if (dev->EPUseDMA == FUSB220_DMA_IS_IDLE_NOW) {
			req = list_entry(ep->queue.next, struct FTC_request, queue);
			req->CurDmaStartAddr = req->req.dma;
		}
#if (USE_DMA==TRUE)
		else if ((dev->EPUseDMA == ep->num) && (dev->ReqForDMA != 0)) {
			// DMA does not finish it's last work.
			req = dev->ReqForDMA;	//get the request                        
		}
#endif
		else
			goto TO_DO_NOTHING;

		DBG_CTRLL("ep **NON queue empty EP num = %d\n", ep->num);

		if ((u32) req->req.length == 0)
			DBG_TEMP_udc("Error: Transfer length in request = 0\n");

#if (USE_DMA==TRUE)
		if (ep->dma_running == TRUE) {
			DBG_TEMP_udc("Error: DMA running but rising EP%d interrupt, bFUSBPort(0x12)=0x%x\n",
				     ep->num, bFUSBPort(0x12));
			return;
		} else {
			if (ep->num == BULK_IN_EP_NUM) {
				// If this is an Bulk IN endpoint and the length we want to transmit is 4 byte alignment, 
				// we can use DMA to transmit this data.
				if (((u32) req->req.length & 0xf) == 0) {
					start_dma(dev, ep, req);
					return;
				}
			}
			if (ep->num == BULK_OUT_EP_NUM) {
				// If this is an Bulk OUT endpoint and the length we want to receive is 4 byte alignment, 
				// we can use DMA to receive this data.
				// ( in here we use byte count to check receive data length and request data length to check)
				if ((((u32) mUsbFIFOOutByteCount(((mUsbEPMapRd(ep->num)) >> 4)) & 0xf) == 0) &&
				    (((u32) req->req.length & 0xf) == 0)) {
					start_dma(dev, ep, req);
					return;
				}
			}
		}
#endif

		(ep->is_in ? write_fifo : read_fifo) (ep, req);
		return;

	      TO_DO_NOTHING:
		DBG_TEMP("DMA is enable now, can't service any other data transfer for ep%d, dma is busy for ep%d\n",
			 ep->num, dev->EPUseDMA);

	}
}

/*-------------------------------------------------------------------------*/
// Faraday USB initial code
///////////////////////////////////////////////////////////////////////////////
//              vFUSB220Init()
//              Description:
//                      1. Turn on the "Global Interrupt Enable" bit of FUSB220
//                      2. Turn on the "Chip Enable" bit of FUSB200
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vFUSB220Init(void)
{
	DBG_FUNCC("+vFUSB220Init()\n");

	// suspend counter
	mUsbIdleCnt(7);

	// Clear interrupt
	mUsbIntBusRstClr();
	mUsbIntSuspClr();
	mUsbIntResmClr();

	// Disable all fifo interrupt
	mUsbIntFIFO0_3OUTDis();
	mUsbIntFIFO4_7OUTDis();
	mUsbIntFIFO8_9OUTDis();
	mUsbIntFIFO0_7INDis();
	mUsbIntFIFO8_9INDis();

	// Soft Reset
	mUsbSoftRstSet();	// All circuit change to which state after Soft Reset?
	mUsbSoftRstClr();

	// Clear all fifo
	mUsbClrAllFIFOSet();	// will be cleared after one cycle.

	// move to udc_enable
	//// Enable usb200 global interrupt
	//mUsbGlobIntEnSet();
	//mUsbChipEnSet();

	//Bruce;;Clear mUsbIntEP0EndDis
	mUsbIntEP0EndDis();
}

///////////////////////////////////////////////////////////////////////////////
//              vUsbInit(struct FTC_udc *dev)
//              Description:
//                      1. Configure the FIFO and EPx map.
//                      2. Init FUSB220.
//                      3. Set the usb interrupt source as edge-trigger.
//                      4. Enable Usb interrupt.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsbInit(struct FTC_udc *dev)
{
	DBG_FUNCC("+vUsbInit()\n");

	// init variables
	dev->u16TxRxCounter = 0;
	dev->eUsbCxCommand = CMD_VOID;
	dev->u8UsbConfigValue = 0;
	dev->u8UsbInterfaceValue = 0;
	dev->u8UsbInterfaceAlternateSetting = 0;

	// init hardware
	vFUSB220Init();
}

/*-------------------------------------------------------------------------*/
static void
udc_reinit(struct FTC_udc *dev)
{
	unsigned i;

	DBG_FUNCC("+udc_reinit()\n");

	INIT_LIST_HEAD(&dev->gadget.ep_list);
	dev->gadget.ep0 = &dev->ep[0].ep;
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	dev->ep0state = EP0_DISCONNECT;
	dev->irqs = 0;

	for (i = 0; i < FUSB220_CURRENT_SUPPORT_EP; i++) {
		struct FTC_ep *ep = &dev->ep[i];

		ep->num = i;
		ep->ep.name = names[i];
		DBG_CTRLL("EP%d Name = %s\n", i, ep->ep.name);

		ep->ep.ops = &FTC_ep_ops;
		list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);
		ep->dev = dev;
		INIT_LIST_HEAD(&ep->queue);
		ep_reset(ep);
	}
	for (i = 0; i < FUSB220_CURRENT_SUPPORT_EP; i++)
		dev->ep[i].irqs = 0;

	dev->ep[0].ep.maxpacket = MAX_EP0_SIZE;
	list_del_init(&dev->ep[0].ep.ep_list);
}

static void
udc_reset(struct FTC_udc *dev)
{
	DBG_FUNCC("+udc_reset()\n");

	//default value
	dev->Dma_Status = PIO_Mode;
	dev->u8LineCount = 0;
	INFO(dev, "***** FTC USB Device 2.0 (FUSB220) Linux Lower Driver *****\n");
	INFO(dev, "L%x: System initial, Please wait...\n", dev->u8LineCount++);

	// initial Reg setup
	mUsbIntRdBufErrDis();	// Disable Read buffer error interrupt (for AXD memory table) 0x11 BIT5
	mUsbIntBufEmptyDis();	// 0x18 BIT0
	mUsbTstHalfSpeedEn();	// Set for FPGA Testing 0x02 BIT7 
	mUsbUnPLGClr();		// 0x08 BIT0
	vUsbInit(dev);

	INFO(dev, "L%x: System reset finish...\n", dev->u8LineCount++);
	//INFO(dev,"\nInterrupt Mask:0x%x\n",bFUSBPort(0x10));
}

static void
udc_enable(struct FTC_udc *dev)
{
	DBG_FUNCC("+udc_enable()\n");

	// Enable usb200 global interrupt
	mUsbGlobIntEnSet();
	mUsbChipEnSet();
}

/*-------------------------------------------------------------------------*/

/* keeping it simple:
 * - one bus driver, initted first;
 * - one function driver, initted second
 */

/* when a driver is successfully registered, it will receive
 * control requests including set_configuration(), which enables
 * non-control requests.  then usb traffic follows until a
 * disconnect is reported.  then a host may connect again, or
 * the driver might get unbound.
 */
int
usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct FTC_udc *dev = the_controller;
	int retval;
	DBG_FUNCC("+usb_gadget_register_driver()\n");
	if (!dev)
		return -ENODEV;
	if (driver->speed != USB_SPEED_HIGH ||
	    !driver->bind || !driver->unbind || !driver->disconnect || !driver->setup)
		return -EINVAL;
	if (dev->driver)
		return -EBUSY;

	/* hook up the driver */
	dev->driver = driver;
	dev->gadget.dev.driver = &driver->driver;

	device_add(&dev->gadget.dev);
	retval = driver->bind(&dev->gadget);
	if (retval) {
		DBG(dev, "bind to driver %s --> error %d\n", driver->driver.name, retval);
		device_del(&dev->gadget.dev);

		dev->driver = NULL;
		dev->gadget.dev.driver = NULL;
		return retval;
	}
	//device_create_file(dev->dev, &dev_attr_function);

	/* then enable host detection and ep0; and we're ready
	 * for set_configuration as well as eventual disconnect.
	 */
	udc_enable(dev);

	DBG(dev, "registered gadget driver '%s'\n", driver->driver.name);
	DBG_FUNCC("-usb_gadget_register_driver()\n");
	return 0;
}

EXPORT_SYMBOL(usb_gadget_register_driver);

/*
static void
stop_activity(struct goku_udc *dev, struct usb_gadget_driver *driver)
{
	unsigned	i;

	DBG (dev, "%s\n", __FUNCTION__);

	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = 0;

	// disconnect gadget driver after quiesceing hw and the driver 
	udc_reset (dev);
	for (i = 0; i <= FUSB220_MAX_EP; i++)
		nuke(&dev->ep [i], -ESHUTDOWN);
	if (driver) {
		spin_unlock(&dev->lock);
		driver->disconnect(&dev->gadget);
		spin_lock(&dev->lock);
	}

	if (dev->driver)
		udc_enable(dev);
}
*/

int
usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct FTC_udc *dev = the_controller;
	unsigned long flags;
	DBG_FUNCC("+usb_gadget_unregister_driver()\n");

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver)
		return -EINVAL;
	spin_lock_irqsave(&dev->lock, flags);
	dev->driver = 0;

	//john stop_activity(dev, driver);
	spin_unlock_irqrestore(&dev->lock, flags);
	driver->unbind(&dev->gadget);

	DBG(dev, "unregistered driver '%s'\n", driver->driver.name);
	return 0;
}

EXPORT_SYMBOL(usb_gadget_unregister_driver);

/*-------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
//              vUsb_rst(struct FTC_udc *dev)
//              Description:
//                      1. Change descriptor table (High or Full speed).
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_rst(struct FTC_udc *dev)
{
	//struct zero_dev *pzero_dev = (struct zero_dev *) dev->gadget.ep0->driver_data; 

	//Move to higher Init AP
	//vUsb_BulkInit();
	//vUsb_IntInit();

	DBG_FUNCC("+vUsb_rst()\n");

	// stop
	INFO(dev, "L%x, Bus reset\n", dev->u8LineCount++);

	//Bruce;;Mark;;12282004;;dev->driver->disconnect(&dev->gadget);
	mUsbIntBusRstClr();
	dev->gadget.speed = USB_SPEED_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_suspend(dev)
//              Description:
//                      1. .
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_suspend(struct FTC_udc *dev)
{
	DBG_FUNCC("+vUsb_suspend()\n");
	INFO(dev, "L%x, Bus suspend\n", dev->u8LineCount++);
	// uP must do-over everything it should handle and do before into the suspend mode
	// Go Suspend status
	mUsbIntSuspClr();

	//Bruce;;mUsbGoSuspend();
	dev->ep0state = EP0_SUSPEND;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_resm(struct FTC_udc        *dev)
//              Description:
//                      1. Change descriptor table (High or Full speed).
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_resm(struct FTC_udc *dev)
{
	DBG_FUNCC("+vUsb_resm()\n");

	INFO(dev, "L%x, Bus resume\n", dev->u8LineCount++);
	// uP must do-over everything it should handle and do before into the suspend mode
	// uP must wakeup immediately
	mUsbIntResmClr();

	dev->ep0state = EP0_IDLE;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_Iso_SeqErr(struct FTC_udc *dev)
//              Description:
//                      1. FUSB200 Detects High bandwidth isochronous Data PID sequential error.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_Iso_SeqErr(struct FTC_udc *dev)
{
	u8 u8Tmp0 = mUsbIntIsoSeqErr0Rd();
	u8 u8Tmp1 = mUsbIntIsoSeqErr1Rd();
	u8 i;

	DBG_FUNCC("+vUsb_Iso_SeqErr()\n");

	for (i = 1; i < 8; i++) {
		if (u8Tmp0 & (BIT0 << i))
			WARN(dev, "L%x, EP%x Isochronous Sequential Error\n", dev->u8LineCount++, i);
	}
	for (i = 0; i < 8; i++) {
		if (u8Tmp1 & (BIT0 << i))
			WARN(dev, "L%x, EP%x Isochronous Sequential Error\n", dev->u8LineCount++, i + 8);
	}

	mUsbIntIsoSeqErrClr();
	mUsbIntIsoSeqErr0Clr();
	mUsbIntIsoSeqErr1Clr();
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_Iso_SeqAbort(struct FTC_udc *dev)
//              Description:
//                      1. FUSB200 Detects High bandwidth isochronous  Data PID sequential abort.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_Iso_SeqAbort(struct FTC_udc *dev)
{
	u8 u8Tmp0 = mUsbIntIsoSeqAbort0Rd();
	u8 u8Tmp1 = mUsbIntIsoSeqAbort1Rd();
	u8 i;

	DBG_FUNCC("+vUsb_Iso_SeqAbort()\n");

	for (i = 1; i < 8; i++) {
		if (u8Tmp0 & (BIT0 << i))
			WARN(dev, "L%x, EP%x Isochronous Sequential Abort\n", dev->u8LineCount++, i);
	}
	for (i = 0; i < 8; i++) {
		if (u8Tmp1 & (BIT0 << i))
			WARN(dev, "L%x, EP%x Isochronous Sequential Abort\n", dev->u8LineCount++, i + 8);
	}

	mUsbIntIsoSeqAbortClr();
	mUsbIntIsoSeqAbort0Clr();
	mUsbIntIsoSeqAbort1Clr();
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_TX0Byte(struct FTC_udc *dev)
//              Description:
//                      1. Send 0 byte data to host.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_TX0Byte(struct FTC_udc *dev)
{
	u8 u8Tmp0 = mUsbIntTX0Byte0Rd();
	u8 u8Tmp1 = mUsbIntTX0Byte1Rd();
	u8 i;

	DBG_FUNCC("+vUsb_TX0Byte()\n");

	for (i = 1; i < 8; i++) {
		if (u8Tmp0 & (BIT0 << i))
			DBG_CTRLL("L%x, EP%x IN data 0 byte to host\n", dev->u8LineCount++, i);
	}
	for (i = 0; i < 8; i++) {
		if (u8Tmp1 & (BIT0 << i))
			DBG_CTRLL("L%x, EP%x IN data 0 byte to host\n", dev->u8LineCount++, i + 8);
	}

	mUsbIntTX0ByteClr();
	mUsbIntTX0Byte0Clr();
	mUsbIntTX0Byte1Clr();
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_RX0Byte(struct FTC_udc *dev)
//              Description:
//                      1. Receive 0 byte data from host.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_RX0Byte(struct FTC_udc *dev)
{
	u8 u8Tmp0 = mUsbIntRX0Byte0Rd();
	u8 u8Tmp1 = mUsbIntRX0Byte1Rd();
	u8 i;

	DBG_FUNCC("+vUsb_RX0Byte()\n");

	for (i = 1; i < 8; i++) {
		if (u8Tmp0 & (BIT0 << i))
			DBG_CTRLL("L%x, EP%x OUT data 0 byte to Device\n", dev->u8LineCount++, i);
	}
	for (i = 0; i < 8; i++) {
		if (u8Tmp1 & (BIT0 << i))
			DBG_CTRLL("L%x, EP%x OUT data 0 byte to Device\n", dev->u8LineCount++, i + 8);
	}

	mUsbIntRX0ByteClr();
	mUsbIntRX0Byte0Clr();
	mUsbIntRX0Byte1Clr();
}

void
vUsbClrEPx(void)
{
	u8 u8ep;

	DBG_FUNCC("+vUsbClrEPx()\n");

	// Clear All EPx Toggle Bit
	for (u8ep = 1; u8ep <= FUSB220_MAX_EP; u8ep++) {
		mUsbEPinRsTgSet(u8ep);
		mUsbEPinRsTgClr(u8ep);
	}
	for (u8ep = 1; u8ep <= FUSB220_MAX_EP; u8ep++) {
		mUsbEPoutRsTgSet(u8ep);
		mUsbEPoutRsTgClr(u8ep);
	}
}

///////////////////////////////////////////////////////////////////////////////
//              bGet_status(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
//              Description: (add by Andrew)
//                      1. Send 2 bytes status to host.
//              input: none
//              output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8
bGet_status(struct FTC_udc *dev, const struct usb_ctrlrequest *ctrl)
{
	u8 u8ep_n, u8fifo_n, RecipientStatusLow, RecipientStatusHigh;
	u8 u8Tmp[2];
	u8 bdir;

	DBG_FUNCC("+bGet_status()  ==> Add by Andrew\n");

	RecipientStatusLow = 0;
	RecipientStatusHigh = 0;
	switch ((ctrl->bRequestType) & 0x3)	// Judge which recipient type is at first
	{
	case 0:		// Device
		// Return 2-byte's Device status (Bit1:Remote_Wakeup, Bit0:Self_Powered) to Host
		// Notice that the programe sequence of RecipientStatus
		RecipientStatusLow = mUsbRmWkupST() << 1;
		// Bit0: Self_Powered--> DescriptorTable[0x23], D6(Bit 6)
		// Now we force device return data as self power. (Andrew)
		RecipientStatusLow |= ((USB_CONFIG_ATT_SELFPOWER >> 6) & 0x01);
		break;
	case 1:		// Interface
		// Return 2-byte ZEROs Interface status to Host
		break;

	case 2:		// Endpoint
		if (ctrl->wIndex == 0x00) {
			if (dev->ep0state == EP0_STALL)
				RecipientStatusLow = TRUE;
		} else {
			u8ep_n = (u8) ctrl->wIndex & 0x7F;	// which ep will be clear
			bdir = (u8) ctrl->wIndex >> 7;	// the direction of this ep
			if (u8ep_n > FUSB220_MAX_EP)	// over the Max. ep count ?
				return FALSE;
			else {
				u8fifo_n = mUsbEPMapRd(u8ep_n);	// get the relatived FIFO number
				if (bdir == 1)
					u8fifo_n &= 0x0F;
				else
					u8fifo_n >>= 4;
				if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
					return FALSE;

				// Check the FIFO had been enable ?
				if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
					return FALSE;
				if (bdir == 1)	// IN direction ?
					RecipientStatusLow = mUsbEPinStallST(u8ep_n);
				else
					RecipientStatusLow = mUsbEPoutStallST(u8ep_n);
				DBG_TEMP("+bGet_status()  ==> EP=0x%x, Status=%d\n",
					 (u8) ctrl->wIndex, RecipientStatusLow);
			}
		}
		break;
	default:
		return FALSE;
	}

	// return RecipientStatus;
	u8Tmp[0] = RecipientStatusLow;
	u8Tmp[1] = RecipientStatusHigh;
	vUsbCxFWr(u8Tmp, 2);
	mUsbEP0DoneSet();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//              bSet_feature(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
//              Description: (add by Andrew)
//                      1. Process Cx Set feature command.
//              input: none
//              output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8
bSet_feature(struct FTC_udc *dev, const struct usb_ctrlrequest *ctrl)
{
	u8 i, u8ep_n, u8fifo_n;
	u8 u8Tmp[52];
	u8 *pp;
	u8 bdir;

	DBG_FUNCC("+bSet_feature()  ==> Add by Andrew\n");

	switch (ctrl->wValue)	// FeatureSelector
	{
	case 0:		// ENDPOINT_HALE
		// Set "Endpoint_Halt", Turn on the "STALL" bit in Endpoint Control Function Register
		if (ctrl->wIndex == 0x00)
			FTC_set_halt(dev->gadget.ep0, 2);	// Set EP0 to functional stall
		else {
			u8ep_n = (u8) ctrl->wIndex & 0x7F;	// which ep will be clear
			bdir = (u8) ctrl->wIndex >> 7;	// the direction of this ep
			if (u8ep_n > FUSB220_MAX_EP)	// over the Max. ep count ?
				return FALSE;
			else {
				u8fifo_n = mUsbEPMapRd(u8ep_n);	// get the relatived FIFO number
				if (bdir == 1)
					u8fifo_n &= 0x0F;
				else
					u8fifo_n >>= 4;
				if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
					return FALSE;

				// Check the FIFO had been enable ?
				if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
					return FALSE;
				if (bdir == 1)	// IN direction ?
					mUsbEPinStallSet(u8ep_n);	// Clear Stall Bit
				else
					mUsbEPoutStallSet(u8ep_n);	// Set Stall Bit
				DBG_TEMP("+bSet_feature()  ==> EP=0x%x\n", (u8) ctrl->wIndex);
			}
		}
		mUsbEP0DoneSet();
		break;
	case 1:		// Device Remote Wakeup
		// Set "Device_Remote_Wakeup", Turn on the"RMWKUP" bit in Mode Register
		mUsbRmWkupSet();
		mUsbEP0DoneSet();
		break;

	case 2:		// Test Mode
		switch (ctrl->wIndex >> 8)	// TestSelector
		{
		case 0x1:	// Test_J
			mUsbTsMdWr(TEST_J);
			mUsbEP0DoneSet();
			break;

		case 0x2:	// Test_K
			mUsbTsMdWr(TEST_K);
			mUsbEP0DoneSet();
			break;

		case 0x3:	// TEST_SE0_NAK
			mUsbTsMdWr(TEST_SE0_NAK);
			mUsbEP0DoneSet();
			break;

		case 0x4:	// Test_Packet
			mUsbTsMdWr(TEST_PKY);
			mUsbEP0DoneSet();	// special case: follow the test sequence
			//////////////////////////////////////////////
			// Jay ask to modify, 91-6-5 (Begin)            //
			//////////////////////////////////////////////
			pp = u8Tmp;
			for (i = 0; i < 9; i++)	// JKJKJKJK x 9
			{
				(*pp) = (0x00);
				pp++;
			}

			(*pp) = (0xAA);
			pp++;
			(*pp) = (0x00);
			pp++;

			for (i = 0; i < 8; i++)	// 8*AA
			{
				(*pp) = (0xAA);
				pp++;
			}

			for (i = 0; i < 8; i++)	// 8*EE
			{
				(*pp) = (0xEE);
				pp++;
			}
			(*pp) = (0xFE);
			pp++;

			for (i = 0; i < 11; i++)	// 11*FF
			{
				(*pp) = (0xFF);
				pp++;
			}

			(*pp) = (0x7F);
			pp++;
			(*pp) = (0xBF);
			pp++;
			(*pp) = (0xDF);
			pp++;
			(*pp) = (0xEF);
			pp++;
			(*pp) = (0xF7);
			pp++;
			(*pp) = (0xFB);
			pp++;
			(*pp) = (0xFD);
			pp++;
			(*pp) = (0xFC);
			pp++;
			(*pp) = (0x7E);
			pp++;
			(*pp) = (0xBF);
			pp++;
			(*pp) = (0xDF);
			pp++;
			(*pp) = (0xFB);
			pp++;
			(*pp) = (0xFD);
			pp++;
			(*pp) = (0xFB);
			pp++;
			(*pp) = (0xFD);
			pp++;
			(*pp) = (0x7E);
			vUsbCxFWr(u8Tmp, 52);

			//////////////////////////////////////////////
			// Jay ask to modify, 91-6-5 (End)                      //
			//////////////////////////////////////////////

			// Turn on "r_test_packet_done" bit(flag) (Bit 5)
			mUsbTsPkDoneSet();
			break;

		case 0x5:	// Test_Force_Enable
			//FUSBPort[0x08] = 0x20;        //Start Test_Force_Enable
			break;

		default:
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//              bSet_Descriptor(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
//              Description: (add by Andrew)
//                      1. Send 2 bytes status to host.
//              input: none
//              output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8
bSet_Descriptor(struct FTC_udc *dev, const struct usb_ctrlrequest *ctrl)
{
	DBG_FUNCC("+bSet_Descriptor()  ==> Not Implement\n");
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//              bSynch_frame(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
//              Description: (add by Andrew)
//                      1. If the EP is a Iso EP, then return the 2 bytes Frame number.
//                               else stall this command
//              input: none
//              output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static u8
bSynch_frame(struct FTC_udc *dev, const struct usb_ctrlrequest *ctrl)
{
	u8 TransferType;
	u8 u8Tmp[2];
	DBG_FUNCC("+bSynch_frame()  ==> add by Andrew\n");

	if ((ctrl->wIndex == 0) || (ctrl->wIndex > 4))
		return FALSE;

	// Does the Endpoint support Isochronous transfer type? 
	TransferType = (dev->ep[ctrl->wIndex].desc->bmAttributes) & 0x03;
	if (TransferType == 1)	// Isochronous
	{
		u8Tmp[0] = mUsbFrameNoLow();
		u8Tmp[1] = mUsbFrameNoHigh();

		vUsbCxFWr(u8Tmp, 2);
		mUsbEP0DoneSet();
		return TRUE;
	} else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//              bSet_address(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
//              Description: (add by Bruce)
//                      1. Set addr to FUSB200 register.
//              input: none
//              output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8
bSet_address(struct FTC_udc *dev, const struct usb_ctrlrequest *ctrl)
{
	DBG_FUNCC("+bSet_address() = %d\n", ctrl->wValue);

	if (ctrl->wValue >= 0x0100)
		return FALSE;
	else {
		mUsbDevAddrSet(ctrl->wValue);
		mUsbEP0DoneSet();
		return TRUE;
	}
}

///////////////////////////////////////////////////////////////////////////////
//              bClear_feature(struct FTC_udc *dev)
//              Description: (add by Bruce)
//                      1. Send 2 bytes status to host.
//              input: none
//              output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8
bClear_feature(struct FTC_udc *dev, const struct usb_ctrlrequest *ctrl)
{
	u8 u8ep_n;
	u8 u8fifo_n;
	u8 bdir;

	DBG_FUNCC("+bClear_feature()\n");

	switch (ctrl->wValue)	// FeatureSelector
	{
	case 0:		// ENDPOINT_HALE
		// Clear "Endpoint_Halt", Turn off the "STALL" bit in Endpoint Control Function Register
		if (ctrl->wIndex == 0x00)
			FTC_set_halt(dev->gadget.ep0, 0);
		else {
			u8ep_n = ctrl->wIndex & 0x7F;	// which ep will be clear
			bdir = ctrl->wIndex >> 7;	// the direction of this ep
			if (u8ep_n > FUSB220_MAX_EP)	// over the Max. ep count ?
				return FALSE;
			else {
				u8fifo_n = mUsbEPMapRd(u8ep_n);	// get the relatived FIFO number
				if (bdir == 1)
					u8fifo_n &= 0x0F;
				else
					u8fifo_n >>= 4;
				if (u8fifo_n >= FUSB220_MAX_FIFO)	// over the Max. fifo count ?
					return FALSE;

				// Check the FIFO had been enable ?
				if ((mUsbFIFOConfigRd(u8fifo_n) & 0x80) == 0)
					return FALSE;
				// Clear ep stall
				FTC_clear_halt(&dev->ep[u8ep_n]);	//Bruce
			}
			mUsbEP0DoneSet();
			//   FTC_clear_halt(&dev->ep[u8ep_n]);//Bruce
			//      pio_advance(&dev->ep[u8ep_n]);
			return TRUE;
		}
		break;
	case 1:		// Device Remote Wakeup
		// Clear "Device_Remote_Wakeup", Turn off the"RMWKUP" bit in Main Control Register
		mUsbRmWkupClr();
		break;
	case 2:		// Test Mode
		return FALSE;
	default:
		return FALSE;
	}
	mUsbEP0DoneSet();
	return TRUE;
}

#define Cmd_Service_Fail	 		0
#define Cmd_Already_Service 		1
#define Cmd_Let_Gadget_To_Service 	2
///////////////////////////////////////////////////////////////////////////////
//              u8StandardCommand(struct FTC_udc *dev, struct usb_ctrlrequest   *ctrl)
//              Description:
//              input: none
//              output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static u8
u8StandardCommand(struct FTC_udc *dev, struct usb_ctrlrequest *ctrl)
{
	DBG_FUNCC("+bStandardCommand()\n");
	switch (ctrl->bRequest) {
	case USB_REQ_GET_STATUS:	// get statue, add by Andrew
		if (bGet_status(dev, (ctrl)) == FALSE)
			return Cmd_Service_Fail;
		break;

	case USB_REQ_SET_FEATURE:	// set feature, add by Andrew
		if (bSet_feature(dev, (ctrl)) == FALSE)
			return Cmd_Service_Fail;
		break;

	case USB_REQ_SET_DESCRIPTOR:	// set descriptor, add by Andrew
		if (dev->ep0state == EP0_STALL)
			return Cmd_Service_Fail;
		if (bSet_Descriptor(dev, (ctrl)) == FALSE)
			return Cmd_Service_Fail;
		break;

	case USB_REQ_SYNCH_FRAME:	// Synch frame, add by Andrew
		if (dev->ep0state == EP0_STALL)
			return Cmd_Service_Fail;
		if (bSynch_frame(dev, (ctrl)) == FALSE)
			return Cmd_Service_Fail;
		break;

	case USB_REQ_CLEAR_FEATURE:	// clear feature, add by Bruce
		if (bClear_feature(dev, (ctrl)) == FALSE)
			return Cmd_Service_Fail;
		break;

	case USB_REQ_SET_ADDRESS:	// set address, add by Bruce
		if (dev->ep0state == EP0_STALL)
			return Cmd_Service_Fail;
		if (bSet_address(dev, (ctrl)) == FALSE)
			return Cmd_Service_Fail;
		break;

		// for set configuration and set interface, some setting need to be set in lower driver 
		// and other information must set in upper driver
	case USB_REQ_SET_CONFIGURATION:
		if (ctrl->wValue == 0)
			mUsbCfgClr();
		else {
			if (dev->gadget.speed == USB_SPEED_HIGH)
				vUsbFIFO_EPxCfg_HS();
			else if (dev->gadget.speed == USB_SPEED_FULL)
				vUsbFIFO_EPxCfg_FS();

			mUsbCfgSet();
		}
		// reset dma status
		dev->EPUseDMA = FUSB220_DMA_IS_IDLE_NOW;	//reset

		return Cmd_Let_Gadget_To_Service;
		break;

	default:		/* left some commands and pass to gadget driver to service */
		return Cmd_Let_Gadget_To_Service;
		break;
	}
	return Cmd_Already_Service;
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_ep0setup(struct FTC_udc *dev)
//              Description:
//                      1. Read the speed
//                      2. Read 8-byte setup packet.
//                      3. Process the standard command:
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_ep0setup(struct FTC_udc *dev)
{
	u8 u8UsbCmd[8], u8Tmp;
	struct usb_ctrlrequest ctrl;
	int tmp;

	DBG_FUNCC("+vUsb_ep0setup()\n");

	//<1>.Read the speed
	if (dev->gadget.speed == USB_SPEED_UNKNOWN) {
		// first ep0 command after usb reset, means we can check usb speed right now.
		if (mUsbHighSpeedST()) {	// First we should judge HS or FS
			INFO(dev, "L%x, high speed mode\n", dev->u8LineCount++);
			dev->gadget.speed = USB_SPEED_HIGH;
//                vUsbFIFO_EPxCfg_HS();//Set the FIFO information
		} else {
			INFO(dev, "L%x, full speed mode\n", dev->u8LineCount++);
			dev->gadget.speed = USB_SPEED_FULL;
//                vUsbFIFO_EPxCfg_FS();//Set the FIFO information
		}
		dev->ep0state = EP0_IDLE;
	}
	//<2>.Dequeue ALL requests
	nuke(&dev->ep[0], 0);
	dev->ep[0].stopped = 0;

	//<3>.Read 8-byte setup packet from FIFO
	vUsbCxFRd8ByteCmd(u8UsbCmd);
	DBG_CTRLL("L%x, EP0Cmd:%02x %02x %02x %02x %02x %02x %02x %02x\n", dev->u8LineCount++,
		  u8UsbCmd[0], u8UsbCmd[1], u8UsbCmd[2], u8UsbCmd[3], u8UsbCmd[4], u8UsbCmd[5], u8UsbCmd[6],
		  u8UsbCmd[7]);

	/* read SETUP packet and enter DATA stage */
	ctrl.bRequestType = u8UsbCmd[0];
	ctrl.bRequest = u8UsbCmd[1];
	ctrl.wValue = (u8UsbCmd[3] << 8) | u8UsbCmd[2];
	ctrl.wIndex = (u8UsbCmd[5] << 8) | u8UsbCmd[4];
	ctrl.wLength = (u8UsbCmd[7] << 8) | u8UsbCmd[6];

	if (likely(ctrl.bRequestType & USB_DIR_IN)) {
		dev->ep[0].is_in = 1;
		dev->ep0state = EP0_IN;
	} else {
		dev->ep[0].is_in = 0;
		dev->ep0state = EP0_OUT;
	}

	// Check if 
	if (dev->ep0state == EP0_STALL) {
		if (((ctrl.bRequestType & 0x60) != 0) ||
		    !((ctrl.bRequest == USB_REQ_GET_STATUS) ||
		      (ctrl.bRequest == USB_REQ_SET_FEATURE) || (ctrl.bRequest == USB_REQ_CLEAR_FEATURE))) {
			goto stall;
		}
	}
	//Parsing some Standard Commands 
	if ((ctrl.bRequestType & 0x60) == 0) {	// Standard Request codes for USB  lower driver
		u8Tmp = u8StandardCommand(dev, &ctrl);
		if (u8Tmp == Cmd_Service_Fail)
			goto stall;
		else if (u8Tmp == Cmd_Let_Gadget_To_Service)
			goto PassToGadget;
		else if (u8Tmp == Cmd_Already_Service)
			return;	// exit this function
	}

      PassToGadget:		/* pass to gadget driver */
	spin_unlock(&dev->lock);
	tmp = dev->driver->setup(&dev->gadget, &(ctrl));
	spin_lock(&dev->lock);
	if (unlikely(tmp < 0))
		goto stall;
	return;			//Normal Exit

//Stall the command
      stall:
#ifdef USB_TRACE
	VDBG(dev, "req %02x.%02x protocol STALL; err %d\n", ctrl.bRequestType, ctrl.bRequest, tmp);
#endif
	INFO(dev, "Set STALL in vUsb_ep0setup\n");
	DBG_TEMP("+vUsb_ep0setup() fail, set stall\n");
	FTC_set_halt(dev->gadget.ep0, 1);	// Return EP0_Stall
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_ep0tx()
//              Description:
//                      1. Transmit data to EP0 FIFO.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_ep0tx(struct FTC_udc *dev)
{
	struct FTC_request *req;
	DBG_FUNCC("+vUsb_ep0tx()\n");

	req = container_of(dev->EP0req, struct FTC_request, req);
	if (!req) {
		WARN(dev, "vUsb_ep0tx(): No req allocated for EP0, but rising ep0tx int, just skip int\n");
		return;
	}

	switch (dev->eUsbCxCommand) {
	case CMD_GET_DESCRIPTOR:
		write_fifo(&(dev->ep[0]), req);
		//vUsbEP0TxData();
		break;
	default:
		DBG_TEMP("+vUsb_ep0tx() fail, set stall\n");

		// ****** Caution *********
		// Normally, in here we should set ep0 stall(halt) without any condition,
		// that means "if(dev->gadget.speed == USB_SPEED_HIGH)" is unnecessary.
		// But in here, because FUSB220 in A320D still have some bugs. So i must add
		// this condition to let FUSB220 work well.
		if (dev->gadget.speed == USB_SPEED_HIGH)
			FTC_set_halt(dev->gadget.ep0, 1);	// Return EP0_Stall
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_ep0rx()
//              Description:
//                      1. Receive data from EP0 FIFO.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_ep0rx(struct FTC_udc *dev)
{
	DBG_FUNCC("+vUsb_ep0rx()\n");

	switch (dev->eUsbCxCommand) {
	case CMD_SET_DESCRIPTOR:
		FTC_queue(dev->gadget.ep0, dev->EP0req, GFP_ATOMIC);
		//vUsbEP0RxData(dev);
		break;
	default:
		DBG_TEMP("+vUsb_ep0rx() fail, set stall\n");
		FTC_set_halt(dev->gadget.ep0, 1);	// Return EP0_Stall
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_ep0end(struct FTC_udc *dev)
//              Description:
//                      1. End this transfer.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_ep0end(struct FTC_udc *dev)
{
	DBG_FUNCC("+vUsb_ep0end()\n");

	dev->eUsbCxCommand = CMD_VOID;
	mUsbEP0DoneSet();
}

///////////////////////////////////////////////////////////////////////////////
//              vUsb_ep0fail(struct FTC_udc *dev)
//              Description:
//                      1. Stall this transfer.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsb_ep0fail(struct FTC_udc *dev)
{
	DBG_FUNCC("+vUsb_ep0fail()\n");
	DBG_TEMP("+vUsb_ep0fail() fail, set stall\n");
	FTC_set_halt(dev->gadget.ep0, 1);	// Return EP0_Stall
}

///////////////////////////////////////////////////////////////////////////////
//              vUsbHandler(struct FTC_udc      *dev)
//              Description:
//                      1. Service all Usb events
//                      2. ReEnable Usb interrupt.
//              input: none
//              output: none
///////////////////////////////////////////////////////////////////////////////
static void
vUsbHandler(struct FTC_udc *dev)
{
	u8 usb_interrupt_level2;
	u8 usb_interrupt_Mask;
	u8 usb_interrupt_Origan;

	DBG_FUNCC("+vUsbHandler()\n");

	DBG_CTRLL("usb_interrupt_level1:0x%x\n", dev->usb_interrupt_level1);

	if (dev->usb_interrupt_level1 & BIT7)	//Group Byte 7
	{
		usb_interrupt_Origan = mUsbIntByte7Rd();
		usb_interrupt_Mask = mUsbIntByte7MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;

		DBG_CTRLL("IntSCR7:0x%x\n", usb_interrupt_level2);

		if (usb_interrupt_level2 & BIT0) {
			DBG_CTRLL("HBF is Empty\n");
			dev->bUsbBufferEmpty = TRUE;
		} else {
			DBG_CTRLL("HBF is not Empty\n");
			dev->bUsbBufferEmpty = FALSE;
		}

		if (usb_interrupt_level2 & BIT1)
			vUsb_rst(dev);
		if (usb_interrupt_level2 & BIT2) {
			vUsb_suspend(dev);
			return;	// once enter suspend, exit immediately
		}
		if (usb_interrupt_level2 & BIT3)
			vUsb_resm(dev);
		if (usb_interrupt_level2 & BIT4)
			vUsb_Iso_SeqErr(dev);
		if (usb_interrupt_level2 & BIT5)
			vUsb_Iso_SeqAbort(dev);
		if (usb_interrupt_level2 & BIT6)
			vUsb_TX0Byte(dev);
		if (usb_interrupt_level2 & BIT7)
			vUsb_RX0Byte(dev);
	}

	if (dev->usb_interrupt_level1 & BIT0)	//Group Byte 0
	{
		usb_interrupt_Origan = mUsbIntByte0Rd();
		usb_interrupt_Mask = mUsbIntByte0MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;

		DBG_CTRLL("IntSCR0:0x%x\n", usb_interrupt_level2);
		dev->ep[0].irqs++;
		//      Stop APB DMA if DMA is still running 
		//      record buffer counter, and clear buffer. Later  
		//      will re-input data use DMA.     
		if (usb_interrupt_level2 & BIT0) {
			DBG_CTRLL("USB ep0 Setup\n");
			vUsb_ep0setup(dev);
		} else if (usb_interrupt_level2 & BIT3) {
			DBG_CTRLL("USB ep0 end\n");
			vUsb_ep0end(dev);
		}
		if (usb_interrupt_level2 & BIT1) {
			DBG_CTRLL("USB ep0 TX\n");
			vUsb_ep0tx(dev);
		}
		if (usb_interrupt_level2 & BIT2) {
			DBG_CTRLL("USB ep0 RX\n");
			vUsb_ep0rx(dev);
		}
		if (usb_interrupt_level2 & BIT4) {
			WARN(dev, "USB ep0 fail\n");
			vUsb_ep0fail(dev);
		}
		if (usb_interrupt_level2 & BIT5) {
			WARN(dev, "RBUF error\n");
			mUsbIntRdBufErrClr();
		}
	}

	if (dev->usb_interrupt_level1 & BIT1)	//Group Byte 1
	{
		usb_interrupt_Origan = mUsbIntByte1Rd();
		usb_interrupt_Mask = mUsbIntByte1MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;

		DBG_CTRLL("IntSCR1:0x%x\n", usb_interrupt_level2);

		dev->ep[2].irqs++;
		// YPING : use FIFO2 for ep2( bulk out)
		if (usb_interrupt_level2 & BIT5) {	// short packet
			DBG_BULKK("Bulk Out <EP2> Short Packet\n");
			EPX_check_advance(dev, &dev->ep[2]);	// EP2 bulk out,receive from Host
		} else if (usb_interrupt_level2 & BIT4) {	// full packet
			DBG_BULKK("Bulk Out <EP2> Full Packet\n");
			EPX_check_advance(dev, &dev->ep[2]);	// EP2 bulk out
		}
	}

	if (dev->usb_interrupt_level1 & BIT2)	//Group Byte 2
	{
		usb_interrupt_Origan = mUsbIntByte2Rd();
		usb_interrupt_Mask = mUsbIntByte2MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;

		DBG_CTRLL("IntSCR2:0x%x\n", usb_interrupt_level2);
		dev->ep[4].irqs++;
		// YPING : use FIFO5 for ep4( interrupt out)
		if (usb_interrupt_level2 & BIT3) {	// short packet
			DBG_CTRLL("Interrupt OUT <EP4> Short Packet\n");
			EPX_check_advance(dev, &dev->ep[4]);	// EP4 interrupt out
		} else if (usb_interrupt_level2 & BIT2) {	// full packet
			DBG_CTRLL("Interrupt OUT <EP4> Full Packet\n");
			EPX_check_advance(dev, &dev->ep[4]);	// EP4 interrupt out
		}
	}

	if (dev->usb_interrupt_level1 & BIT5)	//Group Byte 5
	{
		usb_interrupt_Origan = mUsbIntByte5Rd();
		usb_interrupt_Mask = mUsbIntByte5MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;

		DBG_CTRLL("IntSCR5:0x%x\n", usb_interrupt_level2);
		if (usb_interrupt_level2 & BIT0) {
			DBG_CTRLL("Bulk IN <EP1> \n");
			dev->ep[1].irqs++;
			EPX_check_advance(dev, &dev->ep[1]);	// EP1 bulk in            
		}
		// YPING : use FIFO4 for ep3( Interrupt In)
		if (usb_interrupt_level2 & BIT4) {
			DBG_CTRLL("Interrupt IN <EP3> \n");
			dev->ep[3].irqs++;
			EPX_check_advance(dev, &dev->ep[3]);	// EP1 interrupt in
		}
	}

	if (dev->usb_interrupt_level1 & BIT6)	//Group Byte 6
	{
		usb_interrupt_Origan = mUsbIntByte6Rd();
		usb_interrupt_Mask = mUsbIntByte6MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;

		DBG_CTRLL("IntSCR6:0x%x\n", usb_interrupt_level2);
	}
}

static irqreturn_t
FTC_irq(int irq, void *_dev, struct pt_regs *r)
{
	struct FTC_udc *dev = _dev;
	u32 handled = 0;
	spin_lock(&dev->lock);

	dev->usb_interrupt_level1_Save = mUsbIntGroupRegRd();
	dev->usb_interrupt_level1_Mask = mUsbIntGroupMaskRd();
	dev->usb_interrupt_level1 = dev->usb_interrupt_level1_Save & ~dev->usb_interrupt_level1_Mask;
	//dev->usb_interrupt_level1 = dev->usb_interrupt_level1_Save;

	if (dev->usb_interrupt_level1 != 0) {
		dev->irqs++;
		handled = 1;
		//ib_DisableInt(IRQ_USBDEV);

		//INFO(dev,"\nInterrupt Source:0x%x\n",bFUSBPort(0x20));
		vUsbHandler(dev);
		// Clear usb interrupt flags
		dev->usb_interrupt_level1 = 0;
		//ib_EnableInt(IRQ_USBDEV);
	}

	spin_unlock(&dev->lock);
	return IRQ_RETVAL(handled);
}

/*-------------------------------------------------------------------------*/

/* tear down the binding between this driver and the pci device */
//Trace ok 12212004
static void
FTC_usb_remove(void)
{
	DBG_FUNCC("+FTC_usb_remove()\n");
	DBG(the_controller, "%s\n", __FUNCTION__);

	/* start with the driver above us */
	if (the_controller->driver) {
		/* should have been done already by driver model core */
		WARN(dev, "remove driver '%s' is still registered\n", the_controller->driver->driver.name);
		usb_gadget_unregister_driver(the_controller->driver);
	}
	udc_reset(the_controller);

	if (the_controller->got_irq)	//Andrew update
	{
#if 0				//KC
#if 0
		free_irq(IRQ_CPE_AHB_DMA, the_controller);	//Andrew update
#else
		// change this setting for A320D Arm Linux. Add by YPING
		free_irq(VIRQ_USBDEV_AHB_DMA, the_controller);	//Andrew update
#endif
#endif
		free_irq(IRQ_USBDEV, the_controller);	//Andrew update
	}
#if 0				//Andrew update
	// free EP0 req, buffer
	FTC_free_buffer(&the_controller->ep[0].ep,
			the_controller->EP0req->buf, the_controller->EP0req->dma, the_controller->EP0req->length);
	FTC_free_request(&the_controller->ep[0].ep, the_controller->EP0req);
#endif
//      ahb_dma_free(the_controller->ahb_dma);
	kfree(the_controller);	//Andrew update

	the_controller = 0;
	INFO(dev, "USB device unbind\n");
}

/* wrap this driver around the specified pci device, but
 * don't respond over USB until a gadget driver binds to us.
 */
//Trace ok 12212004
static int
FTC_usb_probe(void)
{
	int retval = 0;
	DBG_FUNCC("+FTC_usb_probe()\n");

	//<1>.Init "the_controller" structure
	/* if you want to support more than one controller in a system,
	 * usb_gadget_driver_{register,unregister}() must change.
	 */
	if (the_controller) {
		WARN(dev, "ignoring : more than one device\n");
		return -EBUSY;
	}

	/* alloc, and start init */
	the_controller = kmalloc(sizeof *the_controller, SLAB_KERNEL);
	if (the_controller == NULL) {
		pr_debug("enomem USB device\n");
		retval = -ENOMEM;
		goto done;
	}
	memset(the_controller, 0, sizeof *the_controller);
	spin_lock_init(&the_controller->lock);

	device_initialize(&the_controller->gadget.dev);

	the_controller->gadget.ops = &FTC_ops;

	/* the "gadget" abstracts/virtualizes the controller */
	strncpy(the_controller->gadget.dev.bus_id, "gadget", BUS_ID_SIZE);
	the_controller->gadget.name = driver_name;
	the_controller->enabled = 1;
	the_controller->EPUseDMA = FUSB220_DMA_IS_IDLE_NOW;	//reset
	the_controller->ReqForDMA = 0;	//reset

#if 0				//KC
	/* init ahb dma */
	the_controller->ahb_dma = ahb_dma_alloc();
	//printk("dev_info->priv=0x%x\n",dev_info->priv);
	the_controller->ahb_dma->base = CPE_AHBDMA_VA_BASE;
	the_controller->ahb_dma->llp_master = AHBDMA_MASTER_0;
	the_controller->ahb_dma->src_data_master = AHBDMA_MASTER_0;
	the_controller->ahb_dma->dest_data_master = AHBDMA_MASTER_0;
	the_controller->ahb_dma->llp_count = 0;
#if 0
	the_controller->ahb_dma->channel = USB_AHBDAC;
#else
	// change this setting for A320D Arm Linux. Add by YPING
	the_controller->ahb_dma->channel = PMU_USBDEV_DMA_CHANNEL;
#endif
	the_controller->ahb_dma->hw_handshake = 1;
	ahb_dma_init(the_controller->ahb_dma);
#endif
	the_controller->EPUseDMA = FUSB220_DMA_IS_IDLE_NOW;	//reset

#if 0
	//init PMU for AHB_DMA
	*(volatile u32 *) (CPE_PMU_VA_BASE + 0xc8) = (0x8 | the_controller->ahb_dma->channel);
#endif

	//<2>. udc Reset/udc reinit
	/* init to known state, then setup irqs */
	udc_reset(the_controller);
	udc_reinit(the_controller);

#if 0				//KC
	//<3>.Init AHB DMA ISR
#if 0
	cpe_int_set_irq(IRQ_CPE_AHB_DMA, LEVEL, H_ACTIVE);
	if (request_irq(IRQ_CPE_AHB_DMA, dma_interrupt_handler, SA_INTERRUPT, driver_name, the_controller) != 0) {
		WARN(dev, "Unable to allocate AHB_DMA IRQ %d, maybe already allocated.\n", IRQ_CPE_AHB_DMA);
//              retval = -EBUSY;
//              goto done;
	}
#else
	// change this setting for A320D Arm Linux. Add by YPING
	cpe_int_set_irq(VIRQ_USBDEV_AHB_DMA, LEVEL, H_ACTIVE);
	if (request_irq(VIRQ_USBDEV_AHB_DMA, dma_interrupt_handler, SA_SHIRQ, driver_name, the_controller) != 0) {
		WARN(dev, "Unable to allocate AHB_DMA IRQ %d, maybe already allocated.\n", VIRQ_USBDEV_AHB_DMA);
	}
#endif
#endif

	//<4>.Init USB DEV ISR  
	if (request_irq(IRQ_USBDEV, FTC_irq, SA_INTERRUPT /*|SA_SAMPLE_RANDOM */ ,
			driver_name, the_controller) != 0) {
		WARN(dev, "request interrupt failed\n");
		retval = -EBUSY;
		goto done;
	}
	the_controller->got_irq = 1;

	/* done */
	return 0;

      done:
	DBG_TEMP("FTC_usb_probe() failed\n");
#if 0				//KC
	if (the_controller->ahb_dma)
		kfree(the_controller->ahb_dma);
#endif
	if (the_controller)
		FTC_usb_remove();
	return retval;
}

/*-------------------------------------------------------------------------*/
//Trace ok 12212004
static int __init
init(void)
{
#if(USE_DMA == TRUE)
	INFO(dev, "init USB device Lower driver (DMA mode)\n");
#else
	INFO(dev, "init USB device Lower driver (PIO mode)\n");
#endif
	INFO(dev, "FUSB220_BASE_ADDRESS = 0x%x\n", FUSB220_BASE_ADDRESS);
	return FTC_usb_probe();
}

module_init(init);

//Trace ok 12212004
static void __exit
cleanup(void)
{
	INFO(dev, "remove USB device Lower driver\n");
#if 0				//KC
	if (the_controller->ahb_dma)
		kfree(the_controller->ahb_dma);
#endif
	return FTC_usb_remove();
}

module_exit(cleanup);
