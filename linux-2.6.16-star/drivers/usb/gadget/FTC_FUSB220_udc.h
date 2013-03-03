/*
 * Faraday USB220 ("FTUSB220") USB Device Controller driver
 *
 * Copyright (C) 2004-2005 John
 *	by John	Chiang
 * Copyright (C) 2004 Faraday corp.
 *
 * This	file is	licensed under the terms of the	GNU General Public
 * License version 2.  This program is licensed	"as is"	without	any
 * warranty of any kind, whether express or implied.
 */
//*************************************************************************
//****************************** 1.Name	Define ****************************
//*************************************************************************

//////////////////////////////////////////////

//#define MESS_ERROR	(0x01 << 0)
//#define MESS_WARNING	(0x01 << 1)
//#define MESS_INFO	(0x01 << 2)
//#define MESS_DEBUG	(0x01 << 7)
////////////////////////////////////////////

#define	TRUE			1
#define	FALSE			0

#define	MASK_F0			0xF0

// Block Size define
#define	BLK512BYTE		1
#define	BLK1024BYTE		2

#define	BLK64BYTE		1
#define	BLK128BYTE		2

// Block toggle	number define
#define	SINGLE_BLK		1
#define	DOUBLE_BLK		2
#define	TRIBLE_BLK		3

// Endpoint transfer type
#define	TF_TYPE_CONTROL		0
#define	TF_TYPE_ISOCHRONOUS	1
#define	TF_TYPE_BULK		2
#define	TF_TYPE_INTERRUPT	3

// ***********Caution***************
// Endpoint or FIFO direction define
// Please don't	change this define, this will cause very serious problems.
// Because someone change this define form my original code(FUSB220 firmware on	cpe),
// and didn't check where this register	use. And let i waste 2 days to debug. Damn!!
// IN =	0, OUT = 1....please don't change this define anymore. (YPING left his message.)
#define	DIRECTION_IN	1
#define	DIRECTION_OUT	0

// FIFO	number define
#define	FIFO0		0x0
#define	FIFO1		0x1
#define	FIFO2		0x2
#define	FIFO3		0x3
#define	FIFO4		0x4
#define	FIFO5		0x5
#define	FIFO6		0x6
#define	FIFO7		0x7
#define	FIFO8		0x8
#define	FIFO9		0x9

// Endpoint number define
#define	EP0		0x00
#define	EP1		0x01
#define	EP2		0x02
#define	EP3		0x03
#define	EP4		0x04
#define	EP5		0x05
#define	EP6		0x06
#define	EP7		0x07
#define	EP8		0x08
#define	EP9		0x09
#define	EP10		0x10
#define	EP11		0x11
#define	EP12		0x12
#define	EP13		0x13
#define	EP14		0x14
#define	EP15		0x15

#define	BIT0		0x00000001
#define	BIT1		0x00000002
#define	BIT2		0x00000004
#define	BIT3		0x00000008
#define	BIT4		0x00000010
#define	BIT5		0x00000020
#define	BIT6		0x00000040
#define	BIT7		0x00000080

#define	BIT8		0x00000100
#define	BIT9		0x00000200
#define	BIT10		0x00000400
#define	BIT11		0x00000800
#define	BIT12		0x00001000
#define	BIT13		0x00002000
#define	BIT14		0x00004000
#define	BIT15		0x00008000

#define	BIT16		0x00010000
#define	BIT17		0x00020000
#define	BIT18		0x00040000
#define	BIT19		0x00080000
#define	BIT20		0x00100000
#define	BIT21		0x00200000
#define	BIT22		0x00400000
#define	BIT23		0x00800000

#define	BIT24		0x01000000
#define	BIT25		0x02000000
#define	BIT26		0x04000000
#define	BIT27		0x08000000
#define	BIT28		0x10000000
#define	BIT29		0x20000000
#define	BIT30		0x40000000
#define	BIT31		0x80000000

#define	mLowByte(u16)	((u8)(u16))
#define	mHighByte(u16)	((u8)(u16 >> 8))

#define	DMA_ADDR_INVALID	(~(dma_addr_t)0)
#define	 AHB_DMA_MAX_LEN	8192
//#define  USE_DMA		TRUE
#define	 USE_DMA		FALSE

// Buffer allocation for each EP
#define	USB_EPX_BUFSIZ	4096

#define	MAX_FIFO_SIZE	64	// reset value for EP
#define	MAX_EP0_SIZE	0x40	// ep0 fifo size

#define	USB_EP0_BUFSIZ	256

// Used	for test pattern traffic
#define	TEST_J		0x02
#define	TEST_K		0x04
#define	TEST_SE0_NAK	0x08
#define	TEST_PKY	0x10

#define	FUSB220_CURRENT_SUPPORT_EP	5	//ep0~ep4

#define	FUSB220_MAX_EP			10	// 1..10
#define	FUSB220_MAX_FIFO		10	// 0.. 9
#define	FUSB220_DMA_IS_IDLE_NOW		(FUSB220_MAX_EP+1)	// 1..10
//#define USB_AHBDAC 3

/*-------------------------------------------------------------------------*/
//*************************************************************************
//****************************** 2.Structure Define************************
//*************************************************************************

/* DRIVER DATA STRUCTURES and UTILITIES	*/

struct FTC_ep {
	struct usb_ep ep;
	struct FTC_udc *dev;
	unsigned long irqs;

	unsigned num:8,	dma:1, is_in:1,	stopped:1, dma_running:1;

	/* analogous to	a host-side qh */
	struct list_head queue;
	const struct usb_endpoint_descriptor *desc;
};

struct FTC_request {
	struct usb_request req;
	struct list_head queue;
	u32 u32DMACurrTxLen;
	dma_addr_t CurDmaStartAddr;

	unsigned mapped:1;
};

enum ep0state {
	EP0_DISCONNECT,		/* no host */
	EP0_IDLE,		/* between STATUS ack and SETUP	report */
	EP0_IN,	EP0_OUT,	/* data	stage */
	EP0_STATUS,		/* status stage	*/
	EP0_STALL,		/* data	or status stages */
	EP0_SUSPEND,		/* usb suspend */
};

typedef	enum {
	CMD_VOID,		// No command
	CMD_GET_DESCRIPTOR,	// Get_Descriptor command
	CMD_SET_DESCRIPTOR,	// Set_Descriptor command
	CMD_TEST_MODE		// Test_Mode command
} CommandType;

typedef	enum {
	PIO_Mode,
	AHB_DMA,
	APB_DMA
} DMA_mode;

struct FTC_udc {
	/* each	pci device provides one	gadget,	several	endpoints */
	struct usb_gadget gadget;
	spinlock_t lock;
	struct FTC_ep ep[FUSB220_MAX_EP];
	struct usb_gadget_driver *driver;

	struct usb_request *EP0req;

	enum ep0state ep0state;
	unsigned got_irq:1, got_region:1, req_config:1,	configured:1, enabled:1;

	struct usb_ctrlrequest ControlCmd;

	u8 u8UsbConfigValue;
	u8 u8UsbInterfaceValue;
	u8 u8UsbInterfaceAlternateSetting;

	CommandType eUsbCxCommand;
	DMA_mode Dma_Status;

	u8 bUsbBufferEmpty;

	u8 u16TxRxCounter;

	u8 usb_interrupt_level1;
	u8 usb_interrupt_level1_Save;
	u8 usb_interrupt_level1_Mask;

	u8 *pu8DescriptorEX;

	//11/2/05' AHB_DMA
	//ahb_dma_data_t  *ahb_dma;
	u8 EPUseDMA;		//EP for DMA
	struct FTC_request *ReqForDMA;

	/* statistics... */
	unsigned long irqs;
	u8 u8LineCount;
};

/*-------------------------------------------------------------------------*/
//*************************************************************************
//****************************** 3.Debug Info and hardware feature Define************
//*************************************************************************
#define	NoFixPort		0
#define	FixPort			1
#define	USB_DataPort		NoFixPort

#define	DBG_OFF		0x00
#define	DBG_CTRL	0x01
#define	DBG_BULK	0x02
#define	DBG_ISO		0x04
#define	DBG_INT		0x08
#define	DBG_FUNC	0x10
#define	DBG_TMP		0x20
#define	USB_DBG		(DBG_OFF)	//|DBG_TMP|DBG_FUNC)//|DBG_CTRL)

#define	xprintk(dev,level,fmt,args...) printk(level "%s	: " fmt	, driver_name ,	## args)
#define	wprintk(level,fmt,args...) printk(level	"%s : "	fmt , driver_name , ## args)

#ifdef DEBUG
#define	DBG(dev,fmt,args...) xprintk(dev , KERN_DEBUG ,	fmt , ## args)
#else
#define	DBG(dev,fmt,args...) do	{ } while (0)
#endif				/* DEBUG */

#define	VERBOSE

#ifdef VERBOSE
#define	VDBG DBG
#else
#define	VDBG(dev,fmt,args...) do { } while (0)
#endif				/* VERBOSE */

#define	ERROR(dev,fmt,args...) xprintk(dev , KERN_ERR ,	fmt , ## args)
#define	WARN(dev,fmt,args...) xprintk(dev , KERN_WARNING , fmt , ## args)
#define	INFO(dev,fmt,args...) xprintk(dev , KERN_INFO ,	fmt , ## args)

#if (USB_DBG & DBG_TMP)
#define	DBG_TEMP(fmt,args...) wprintk(KERN_INFO	, fmt ,	## args)
#else
#define	DBG_TEMP(fmt,args...)
#endif

#if (USB_DBG & DBG_FUNC)
#define	DBG_FUNCC(fmt,args...) wprintk(KERN_INFO , fmt , ## args)
#else
#define	DBG_FUNCC(fmt,args...)
#endif

#if (USB_DBG & DBG_CTRL)
#define	DBG_CTRLL(fmt,args...) wprintk(KERN_INFO , fmt , ## args)
#else
#define	DBG_CTRLL(fmt,args...)
#endif

#if (USB_DBG & DBG_BULK)
#define	DBG_BULKK(fmt,args...) wprintk(KERN_INFO , fmt , ## args)
#else
#define	DBG_BULKK(fmt,args...)
#endif

/*-------------------------------------------------------------------------*/
//*************************************************************************
//****************************** 4.Others Define************************
//*************************************************************************

/* 2.5 stuff that's sometimes missing in 2.4 */

#ifndef	container_of
#define	container_of	list_entry
#endif

#ifndef	likely
#define	likely(x)	(x)
#define	unlikely(x)	(x)
#endif

#ifndef	BUG_ON
#define	BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)
#endif

#ifndef	WARN_ON
#define	WARN_ON(x)	do { } while (0)
#endif

#ifndef	IRQ_NONE
typedef	void irqreturn_t;
#define	IRQ_NONE
#define	IRQ_HANDLED
#define	IRQ_RETVAL(x)
#endif

#define	Cmd_Service_Fail		0
#define	Cmd_Already_Service		1
#define	Cmd_Let_Gadget_To_Service	2

//*************************************************************************
//****************************** 5.Function Export Define************************
//*************************************************************************
static void vUsbFIFO_EPxCfg_FS(void);
static void vUsbFIFO_EPxCfg_HS(void);
static int FTC_fifo_status(struct usb_ep *_ep);

//static u8 bClear_feature(struct FTC_udc *dev);
//static u8 bGet_status(struct FTC_udc *dev);
//static u8 bSet_address(struct	FTC_udc	*dev);
//static u8 bSet_descriptor(struct FTC_udc *dev);
//static u8 bSet_feature(struct	FTC_udc	*dev);
//static u8 bGet_descriptor(struct FTC_udc *dev);
//static void vGet_configuration(struct	FTC_udc	*dev);
//static u8 bSet_configuration(struct FTC_udc *dev);
//static u8 bGet_interface(struct FTC_udc *dev);
//static u8 bSet_interface(struct FTC_udc *dev);
//static u8 bSynch_frame(struct	FTC_udc	*dev);
//static u8 bStandardCommand(struct FTC_udc *dev);

//*************************************************************************
//****************************** 6.ED/FIFO Config Define************************
//*************************************************************************

//**************************************
//*** Full Speed HW ED/FIFO Configuration Area ***
#define	FULL_ED1_bBLKSIZE	BLK64BYTE
#define	FULL_ED1_bBLKNO		DOUBLE_BLK
#define	FULL_ED1_bDIRECTION	DIRECTION_IN
#define	FULL_ED1_bTYPE		TF_TYPE_BULK
#define	FULL_ED1_MAXPACKET	64

#define	FULL_ED2_bBLKSIZE	BLK64BYTE
#define	FULL_ED2_bBLKNO		DOUBLE_BLK
#define	FULL_ED2_bDIRECTION	DIRECTION_OUT
#define	FULL_ED2_bTYPE		TF_TYPE_BULK
#define	FULL_ED2_MAXPACKET	64

#define	FULL_ED3_bBLKSIZE	BLK64BYTE
#define	FULL_ED3_bBLKNO		SINGLE_BLK
#define	FULL_ED3_bDIRECTION	DIRECTION_IN
#define	FULL_ED3_bTYPE		TF_TYPE_INTERRUPT
#define	FULL_ED3_MAXPACKET	64

#define	FULL_ED4_bBLKSIZE	BLK64BYTE
#define	FULL_ED4_bBLKNO		SINGLE_BLK
#define	FULL_ED4_bDIRECTION	DIRECTION_OUT
#define	FULL_ED4_bTYPE		TF_TYPE_INTERRUPT
#define	FULL_ED4_MAXPACKET	64
//**************************************************

//**************************************
//*** High Speed HW ED/FIFO Configuration Area ***
#define	HIGH_ED1_bBLKSIZE	BLK512BYTE
#define	HIGH_ED1_bBLKNO		DOUBLE_BLK
#define	HIGH_ED1_bDIRECTION	DIRECTION_IN
#define	HIGH_ED1_bTYPE		TF_TYPE_BULK
#define	HIGH_ED1_MAXPACKET	512

#define	HIGH_ED2_bBLKSIZE	BLK512BYTE
#define	HIGH_ED2_bBLKNO		DOUBLE_BLK
#define	HIGH_ED2_bDIRECTION	DIRECTION_OUT
#define	HIGH_ED2_bTYPE		TF_TYPE_BULK
#define	HIGH_ED2_MAXPACKET	512

#define	HIGH_ED3_bBLKSIZE	BLK64BYTE
#define	HIGH_ED3_bBLKNO		SINGLE_BLK
#define	HIGH_ED3_bDIRECTION	DIRECTION_IN
#define	HIGH_ED3_bTYPE		TF_TYPE_INTERRUPT
#define	HIGH_ED3_MAXPACKET	64

#define	HIGH_ED4_bBLKSIZE	BLK64BYTE
#define	HIGH_ED4_bBLKNO		SINGLE_BLK
#define	HIGH_ED4_bDIRECTION	DIRECTION_OUT
#define	HIGH_ED4_bTYPE		TF_TYPE_INTERRUPT
#define	HIGH_ED4_MAXPACKET	64
//**************************************************

#define	FULL_ED1_FIFO_START	FIFO0
#define	FULL_ED2_FIFO_START	(FULL_ED1_FIFO_START+(FULL_ED1_bBLKNO *FULL_ED1_bBLKSIZE))
#define	FULL_ED3_FIFO_START	(FULL_ED2_FIFO_START+(FULL_ED2_bBLKNO *FULL_ED2_bBLKSIZE))
#define	FULL_ED4_FIFO_START	(FULL_ED3_FIFO_START+(FULL_ED3_bBLKNO *FULL_ED3_bBLKSIZE))

#define	FULL_EP1_Map		(FULL_ED1_FIFO_START |(FULL_ED1_FIFO_START << 4)|(0xF0 >> (4*(1-FULL_ED1_bDIRECTION))))
#define	FULL_EP1_FIFO_Map	((FULL_ED1_bDIRECTION << 4) | EP1)
#define	FULL_EP1_FIFO_Config	(0x80 |	((FULL_ED1_bBLKSIZE - 1) << 4) | ((FULL_ED1_bBLKNO - 1)	<< 2) |	FULL_ED1_bTYPE)
#define	FULL_EP2_Map		(FULL_ED2_FIFO_START |(FULL_ED2_FIFO_START << 4)|(0xF0 >> (4*(1-FULL_ED2_bDIRECTION))))
#define	FULL_EP2_FIFO_Map	((FULL_ED2_bDIRECTION << 4) | EP2)
#define	FULL_EP2_FIFO_Config	(0x80 |	((FULL_ED2_bBLKSIZE - 1) << 4) | ((FULL_ED2_bBLKNO - 1)	<< 2) |	FULL_ED2_bTYPE)
#define	FULL_EP3_Map		(FULL_ED3_FIFO_START |(FULL_ED3_FIFO_START << 4)|(0xF0 >> (4*(1-FULL_ED3_bDIRECTION))))
#define	FULL_EP3_FIFO_Map	((FULL_ED3_bDIRECTION << 4) | EP3)
#define	FULL_EP3_FIFO_Config	(0x80 |	((FULL_ED3_bBLKSIZE - 1) << 4) | ((FULL_ED3_bBLKNO - 1)	<< 2) |	FULL_ED3_bTYPE)
#define	FULL_EP4_Map		(FULL_ED4_FIFO_START |(FULL_ED4_FIFO_START << 4)|(0xF0 >> (4*(1-FULL_ED4_bDIRECTION))))
#define	FULL_EP4_FIFO_Map	((FULL_ED4_bDIRECTION << 4) | EP4)
#define	FULL_EP4_FIFO_Config	(0x80 |	((FULL_ED4_bBLKSIZE - 1) << 4) | ((FULL_ED4_bBLKNO - 1)	<< 2) |	FULL_ED4_bTYPE)

#define	HIGH_ED1_FIFO_START	FIFO0
#define	HIGH_ED2_FIFO_START	(HIGH_ED1_FIFO_START+(HIGH_ED1_bBLKNO *HIGH_ED1_bBLKSIZE))
#define	HIGH_ED3_FIFO_START	(HIGH_ED2_FIFO_START+(HIGH_ED2_bBLKNO *HIGH_ED2_bBLKSIZE))
#define	HIGH_ED4_FIFO_START	(HIGH_ED3_FIFO_START+(HIGH_ED3_bBLKNO *HIGH_ED3_bBLKSIZE))

#define	HIGH_EP1_Map		(HIGH_ED1_FIFO_START |(HIGH_ED1_FIFO_START << 4)|(0xF0 >> (4*(1-HIGH_ED1_bDIRECTION))))
#define	HIGH_EP1_FIFO_Map	((HIGH_ED1_bDIRECTION << 4) | EP1)
#define	HIGH_EP1_FIFO_Config	(0x80 |	((HIGH_ED1_bBLKSIZE - 1) << 4) | ((HIGH_ED1_bBLKNO - 1)	<< 2) |	HIGH_ED1_bTYPE)
#define	HIGH_EP2_Map		(HIGH_ED2_FIFO_START |(HIGH_ED2_FIFO_START << 4)|(0xF0 >> (4*(1-HIGH_ED2_bDIRECTION))))
#define	HIGH_EP2_FIFO_Map	((HIGH_ED2_bDIRECTION << 4) | EP2)
#define	HIGH_EP2_FIFO_Config	(0x80 |	((HIGH_ED2_bBLKSIZE - 1) << 4) | ((HIGH_ED2_bBLKNO - 1)	<< 2) |	HIGH_ED2_bTYPE)
#define	HIGH_EP3_Map		(HIGH_ED3_FIFO_START |(HIGH_ED3_FIFO_START << 4)|(0xF0 >> (4*(1-HIGH_ED3_bDIRECTION))))
#define	HIGH_EP3_FIFO_Map	((HIGH_ED3_bDIRECTION << 4) | EP3)
#define	HIGH_EP3_FIFO_Config	(0x80 |	((HIGH_ED3_bBLKSIZE - 1) << 4) | ((HIGH_ED3_bBLKNO - 1)	<< 2) |	HIGH_ED3_bTYPE)
#define	HIGH_EP4_Map		(HIGH_ED4_FIFO_START |(HIGH_ED4_FIFO_START << 4)|(0xF0 >> (4*(1-HIGH_ED4_bDIRECTION))))
#define	HIGH_EP4_FIFO_Map	((HIGH_ED4_bDIRECTION << 4) | EP4)
#define	HIGH_EP4_FIFO_Config	(0x80 |	((HIGH_ED4_bBLKSIZE - 1) << 4) | ((HIGH_ED4_bBLKNO - 1)	<< 2) |	HIGH_ED4_bTYPE)

//*************************************************************************
//****************************** 7.HW Macro Define************************
//*************************************************************************

#ifndef	__FUSB220_M_H
#define	__FUSB220_M_H

#ifdef CONFIG_ARCH_STR8100
#define	CPE_USBDEV_BASE		SYSVA_USB20_DEVICE_BASE_ADDR
#define	IO_ADDRESS(a)		(a)
#define	FTC_MIN(a,b)		((a)<(b) ? (a):(b))
#define	FTC_MAX(a,b)		((a)>(b) ? (a):(b))
#define	IRQ_USBDEV		INTC_USB20_DEVICE_BIT_INDEX
#endif

	// Macro
#define	FUSB220_BASE_ADDRESS			(IO_ADDRESS(CPE_USBDEV_BASE))	//0x96700000//	0x90600000
#define	FUSB220_FIFO_BASE(bOffset)		(IO_ADDRESS(CPE_USBDEV_BASE)+0xC0+(bOffset<<2))

#define	bFUSBPort(bOffset)			*((volatile u8 *) ( FUSB220_BASE_ADDRESS | (u32)(bOffset)))
#define	wFUSBPort(bOffset)			*((volatile u16	*) ( FUSB220_BASE_ADDRESS | (u32)(bOffset)))
#define	dwFUSBPort(bOffset)			*((volatile u32	*) ( FUSB220_BASE_ADDRESS | (u32)(bOffset)))

	//john
#define	DMAbFUSBPort(bOffset)			*((volatile u8 *) ( CPE_USBDEV_BASE | (u32)(bOffset)))
#define	DMAwFUSBPort(bOffset)			*((volatile u16	*) ( CPE_USBDEV_BASE | (u32)(bOffset)))
#define	DMAdwFUSBPort(bOffset)			*((volatile u32	*) ( CPE_USBDEV_BASE | (u32)(bOffset)))

	// Macro
#define	mUsbGoSuspend()				(bFUSBPort(0x00) |=  (u8)BIT3)

#define	mUsbSoftRstSet()			(bFUSBPort(0x00) |=  (u8)BIT4)
#define	mUsbSoftRstClr()			(bFUSBPort(0x00) &= ~(u8)BIT4)

#define	mUsbHighSpeedST()			(bFUSBPort(0x00) & (u8)BIT6)
#define	mUsbRmWkupST()				(bFUSBPort(0x00) & (u8)BIT0)
#define	mUsbRmWkupClr()				(bFUSBPort(0x00) &= ~(u8)BIT0)
#define	mUsbRmWkupSet()				(bFUSBPort(0x00) |= (u8)BIT0)
#define	mUsbGlobIntEnSet()			(bFUSBPort(0x00) |= (u8)BIT2)
#define	mUsbChipEnSet()				(bFUSBPort(0x00) |= (u8)BIT5)

#define	mUsbHbfFlush()				(bFUSBPort(0x00) |= (u8)BIT1)
#define	mUsbHbfClr()				(bFUSBPort(0x00) |= (u8)BIT7)

#define	mUsbDevAddrSet(Value)			(bFUSBPort(0x01) = Value)
#define	mUsbCfgST()				(bFUSBPort(0x01) & (u8)BIT7)
#define	mUsbCfgSet()				(bFUSBPort(0x01) |= (u8)BIT7)
#define	mUsbCfgClr()				(bFUSBPort(0x01) &= ~(u8)BIT7)
#define	mUsbClrAllFIFOSet()			(bFUSBPort(0x02) |= (u8)BIT0)
#define	mUsbClrAllFIFOClr()			(bFUSBPort(0x02) &= ~(u8)BIT0)

#define	mUsbTstHalfSpeedEn()			(bFUSBPort(0x02) |= (u8)BIT7)
#define	mUsbTstHalfSpeedDis()			(bFUSBPort(0x02) &= ~(u8)BIT7)

#define	mUsbFrameNoLow()			(bFUSBPort(0x04))
#define	mUsbFrameNoHigh()			(bFUSBPort(0x05))

#define	mUsbSOFMaskHS()				(bFUSBPort(0x06) = 0x4c); (bFUSBPort(0x07) = 0x4)
#define	mUsbSOFMaskFS()				(bFUSBPort(0x06) = 0x10); (bFUSBPort(0x07) = 0x27)

#define	mUsbTsMdWr(item)			(bFUSBPort(0x08) = item)
#define	mUsbUnPLGClr()				(bFUSBPort(0x08) &= ~(u8)BIT0)

#define	mUsbEP0DoneSet()			(bFUSBPort(0x0B) |= (u8)BIT0)
#define	mUsbTsPkDoneSet()			(bFUSBPort(0x0B) |= (u8)BIT1)
#define	mUsbEP0StallSet()			(bFUSBPort(0x0B) |= (u8)BIT2)
#define	mUsbEP0ClearFIFO()			(bFUSBPort(0x0B) |= (u8)BIT3)	//john
#define	mUsbEP0EMPFIFO()			(bFUSBPort(0x0B) &= (u8)BIT5)	//john

	///////	Read CxF data ////////
#define	mUsbEP0DataRdByte0()			(bFUSBPort(0x0C))
#define	mUsbEP0DataRdByte2()			(bFUSBPort(0x0E))
#define	mUsbEP0DataRdWord()			(wFUSBPort(0x0C))
#define	mUsbEP0DataRdDWord()			(dwFUSBPort(0x0C))

	///////	Write CxF data ////////
#define	mUsbEP0DataWrByte0(data)		(bFUSBPort(0x0C) = data)
#define	mUsbEP0DataWrByte2(data)		(bFUSBPort(0x0E) = data)
#define	mUsbEP0DataWrWord(data)			(wFUSBPort(0x0C) = data)
#define	mUsbEP0DataWrDWord(data)		(dwFUSBPort(0x0C) = (u32)data)

#define	mUsbIntGrp1Dis()			(bFUSBPort(0x10) |= (u8)BIT1)
#define	mUsbIntGrp2Dis()			(bFUSBPort(0x10) |= (u8)BIT2)
#define	mUsbIntGrp3Dis()			(bFUSBPort(0x10) |= (u8)BIT3)
#define	mUsbIntGrp4Dis()			(bFUSBPort(0x10) |= (u8)BIT4)
#define	mUsbIntGrp5Dis()			(bFUSBPort(0x10) |= (u8)BIT5)
#define	mUsbIntGrp6Dis()			(bFUSBPort(0x10) |= (u8)BIT6)
#define	mUsbIntGrp7Dis()			(bFUSBPort(0x10) |= (u8)BIT7)

#define	mUsbIntEP0SetupDis()			(bFUSBPort(0x11) |= (u8)BIT0)
#define	mUsbIntEP0InDis()			(bFUSBPort(0x11) |= (u8)BIT1)
#define	mUsbIntEP0OutDis()			(bFUSBPort(0x11) |= (u8)BIT2)
#define	mUsbIntEP0EndDis()			(bFUSBPort(0x11) |= (u8)BIT3)
#define	mUsbIntEP0FailDis()			(bFUSBPort(0x11) |= (u8)BIT4)
#define	mUsbIntRdBufErrDis()			(bFUSBPort(0x11) |= (u8)BIT5)

#define	mUsbIntEP0SetupEn()			(bFUSBPort(0x11) &= ~((u8)BIT0))
#define	mUsbIntEP0InEn()			(bFUSBPort(0x11) &= ~((u8)BIT1))
#define	mUsbIntEP0OutEn()			(bFUSBPort(0x11) &= ~((u8)BIT2))
#define	mUsbIntEP0EndEn()			(bFUSBPort(0x11) &= ~((u8)BIT3))
#define	mUsbIntEP0FailEn()			(bFUSBPort(0x11) &= ~((u8)BIT4))
#define	mUsbIntRdBufErrEn()			(bFUSBPort(0x11) &= ~((u8)BIT5))

#define	mUsbIntFIFO0_3OUTDis()			(bFUSBPort(0x12) = 0xFF)
#define	mUsbIntFIFO4_7OUTDis()			(bFUSBPort(0x13) = 0xFF)
#define	mUsbIntFIFO8_9OUTDis()			(bFUSBPort(0x14) = 0xFF)
#define	mUsbIntFIFO0_7INDis()			(bFUSBPort(0x16) = 0xFF)
#define	mUsbIntFIFO8_9INDis()			(bFUSBPort(0x17) = 0xFF)

#define	mUsbIntF0OUTEn()			(bFUSBPort(0x12) &= ~((u8)BIT1 | (u8)BIT0))
#define	mUsbIntF2OUTEn()			(bFUSBPort(0x12) &= ~((u8)BIT5 | (u8)BIT4))
#define	mUsbIntF5OUTEn()			(bFUSBPort(0x13) &= ~((u8)BIT3 | (u8)BIT2))
#define	mUsbIntF0INEn()				(bFUSBPort(0x16) &= ~((u8)BIT0))
#define	mUsbIntF1INEn()				(bFUSBPort(0x16) &= ~((u8)BIT1))
#define	mUsbIntF4INEn()				(bFUSBPort(0x16) &= ~((u8)BIT4))
#define	mUsbIntF8INEn()				(bFUSBPort(0x17) &= ~((u8)BIT0))

#define	mUsbIntF0OUTDis()			(bFUSBPort(0x12) |= ((u8)BIT1 |	(u8)BIT0))
#define	mUsbIntF2OUTDis()			(bFUSBPort(0x12) |= ((u8)BIT5 |	(u8)BIT4))
#define	mUsbIntF5OUTDis()			(bFUSBPort(0x13) |= ((u8)BIT3 |	(u8)BIT2))
#define	mUsbIntF0INDis()			(bFUSBPort(0x16) |= (u8)BIT0)
#define	mUsbIntF4INDis()			(bFUSBPort(0x16) |= (u8)BIT4)
#define	mUsbIntF8INDis()			(bFUSBPort(0x17) |= (u8)BIT0)

#define	mUsbIntFIFOEn(off,val)			(bFUSBPort(off)	&= ~(val))	//john
#define	mUsbIntFIFODis(off,val)			(bFUSBPort(off)	|=  (val))	//john

#define	mUsbIntBufEmptyDis()			(bFUSBPort(0x18) |= 0x01)	//|= BIT0)

#define	mUsbIntRX0Byte0Rd()			(bFUSBPort(0x19))
#define	mUsbIntRX0Byte1Rd()			(bFUSBPort(0x1A))
#define	mUsbIntRX0Byte0Clr()			(bFUSBPort(0x19) = 0)
#define	mUsbIntRX0Byte1Clr()			(bFUSBPort(0x1A) = 0)

#define	mUsbIntGroupMaskRd()			(bFUSBPort(0x10))
#define	mUsbIntByte0MaskRd()			(bFUSBPort(0x11))
#define	mUsbIntByte1MaskRd()			(bFUSBPort(0x12))
#define	mUsbIntByte2MaskRd()			(bFUSBPort(0x13))
#define	mUsbIntByte3MaskRd()			(bFUSBPort(0x14))
#define	mUsbIntByte4MaskRd()			(bFUSBPort(0x15))
#define	mUsbIntByte5MaskRd()			(bFUSBPort(0x16))
#define	mUsbIntByte6MaskRd()			(bFUSBPort(0x17))
#define	mUsbIntByte7MaskRd()			(bFUSBPort(0x18))

#define	mUsbIntRdBufErrClr()			(bFUSBPort(0x21) &= ~((u8)BIT5))
#define	mUsbIntGroupRegRd()			(bFUSBPort(0x20))
#define	mUsbIntByte0Rd()			(bFUSBPort(0x21))
#define	mUsbIntByte1Rd()			(bFUSBPort(0x22))
#define	mUsbIntByte2Rd()			(bFUSBPort(0x23))
#define	mUsbIntByte3Rd()			(bFUSBPort(0x24))
#define	mUsbIntByte4Rd()			(bFUSBPort(0x25))
#define	mUsbIntByte5Rd()			(bFUSBPort(0x26))
#define	mUsbIntByte6Rd()			(bFUSBPort(0x27))
#define	mUsbIntByte7Rd()			(bFUSBPort(0x28))

#define	mUsbIntBusRstClr()			(bFUSBPort(0x28) = 0x00)	// so weird?
#define	mUsbIntSuspClr()			(bFUSBPort(0x28) = 0x00)	// so weird?
#define	mUsbIntResmClr()			(bFUSBPort(0x28) = 0x00)	// so weird?
#define	mUsbIntIsoSeqErrClr()			(bFUSBPort(0x28) &= ~BIT4)	//
#define	mUsbIntIsoSeqAbortClr()			(bFUSBPort(0x28) &= ~BIT5)	//
#define	mUsbIntTX0ByteClr()			(bFUSBPort(0x28) &= ~BIT6)	//
#define	mUsbIntRX0ByteClr()			(bFUSBPort(0x28) &= ~BIT7)	//
#define	mUsbIntBufEmptyRd()			(bFUSBPort(0x28) & BIT0)	//

#define	mUsbIntIsoSeqErr0Rd()			(bFUSBPort(0x29))
#define	mUsbIntIsoSeqErr1Rd()			(bFUSBPort(0x2A))
#define	mUsbIntIsoSeqErr0Clr()			(bFUSBPort(0x29) = 0)
#define	mUsbIntIsoSeqErr1Clr()			(bFUSBPort(0x2A) = 0)

#define	mUsbIntIsoSeqAbort0Rd()			(bFUSBPort(0x2B))
#define	mUsbIntIsoSeqAbort1Rd()			(bFUSBPort(0x2C))
#define	mUsbIntIsoSeqAbort0Clr()		(bFUSBPort(0x2B) = 0)
#define	mUsbIntIsoSeqAbort1Clr()		(bFUSBPort(0x2C) = 0)

#define	mUsbIntTX0Byte0Rd()			(bFUSBPort(0x2D))
#define	mUsbIntTX0Byte1Rd()			(bFUSBPort(0x2E))
#define	mUsbIntTX0Byte0Clr()			(bFUSBPort(0x2D) = 0)
#define	mUsbIntTX0Byte1Clr()			(bFUSBPort(0x2E) = 0)

#define	mUsbIdleCnt(time)			(bFUSBPort(0x2F) = time)
#define	mUsbHbfCountRd()			(bFUSBPort(0x3F) & (0x1F))

	// Endpoint & FIFO Configuration
#define	mUsbEPinHighBandSet(EPn, dir , size)	(bFUSBPort(0x41	+ ((EPn	- 1) <<	1)) &= ~(BIT6 |BIT5));	(bFUSBPort(0x41	+ ((EPn	- 1) <<	1)) |= ((((u8)(size >> 11)+1) << 5)*(dir)) )

#define	mUsbEPMxPtSzLow(EPn, dir, size)		(bFUSBPort(0x40	+ ((1-dir) * 0x20) + ((EPn - 1)	<< 1)) = (u8)(size))
#define	mUsbEPMxPtSzHigh(EPn, dir, size)	(bFUSBPort(0x41	+ ((1-dir) * 0x20) + ((EPn - 1)	<< 1)) = (u8)(size >> 8))
#define	mUsbEPinMxPtSz(EPn)			((((bFUSBPort(0x41 + ((EPn - 1)	<< 1)))	& 0x07)	<< 8) |	(bFUSBPort(0x40	+ ((EPn	- 1) <<	1))))
#define	mUsbEPinStallST(EPn)			((bFUSBPort(0x41 + ((EPn - 1) << 1)) & BIT3) >>	3)
#define	mUsbEPinStallClr(EPn)			(bFUSBPort(0x41	+ ((EPn	- 1) <<	1)) &= ~BIT3)
#define	mUsbEPinStallSet(EPn)			(bFUSBPort(0x41	+ ((EPn	- 1) <<	1)) |=	BIT3)
#define	mUsbEPinRsTgClr(EPn)			(bFUSBPort(0x41	+ ((EPn	- 1) <<	1)) &= ~BIT4)
#define	mUsbEPinRsTgSet(EPn)			(bFUSBPort(0x41	+ ((EPn	- 1) <<	1)) |=	BIT4)

#define	mUsbEPoutMxPtSz(EPn)			((((bFUSBPort(0x61 + ((EPn - 1)	<< 1)))	& 0x07)	<< 8) |	(bFUSBPort(0x60	+ ((EPn	- 1) <<	1))))
#define	mUsbEPoutStallST(EPn)			((bFUSBPort(0x61 + ((EPn - 1) << 1)) & BIT3) >>	3)
#define	mUsbEPoutStallClr(EPn)			(bFUSBPort(0x61	+ ((EPn	- 1) <<	1)) &= ~BIT3)
#define	mUsbEPoutStallSet(EPn)			(bFUSBPort(0x61	+ ((EPn	- 1) <<	1)) |=	BIT3)
#define	mUsbEPoutRsTgClr(EPn)			(bFUSBPort(0x61	+ ((EPn	- 1) <<	1)) &= ~BIT4)
#define	mUsbEPoutRsTgSet(EPn)			(bFUSBPort(0x61	+ ((EPn	- 1) <<	1)) |=	BIT4)

#define	mUsbFIFO0DMAEn()			(bFUSBPort(0x7e) |=  BIT0)
#define	mUsbFIFO1DMAEn()			(bFUSBPort(0x7e) |=  BIT1)
#define	mUsbFIFO2DMAEn()			(bFUSBPort(0x7e) |=  BIT2)
#define	mUsbFIFO3DMAEn()			(bFUSBPort(0x7e) |=  BIT3)
#define	mUsbFIFO4DMAEn()			(bFUSBPort(0x7e) |=  BIT4)
#define	mUsbFIFO5DMAEn()			(bFUSBPort(0x7e) |=  BIT5)
#define	mUsbFIFO6DMAEn()			(bFUSBPort(0x7e	) |=  BIT6)
#define	mUsbFIFO7DMAEn()			(bFUSBPort(0x7e	) |=  BIT7)
#define	mUsbFIFO8DMAEn()			(bFUSBPort(0x7f	) |=  BIT0)
#define	mUsbFIFO9DMAEn()			(bFUSBPort(0x7f	) |=  BIT1)
#define	mUsbFIFO10DMAEn()			(bFUSBPort(0x7f	) |=  BIT2)
#define	mUsbFIFO11DMAEn()			(bFUSBPort(0x7f	) |=  BIT3)
#define	mUsbFIFO12DMAEn()			(bFUSBPort(0x7f	) |=  BIT4)
#define	mUsbFIFO13DMAEn()			(bFUSBPort(0x7f	) |=  BIT5)
#define	mUsbFIFO14DMAEn()			(bFUSBPort(0x7f	) |=  BIT6)
#define	mUsbFIFO15DMAEn()			(bFUSBPort(0x7f	) |=  BIT7)

#define	mUsbFIFO0DMADis()			(bFUSBPort(0x7e	) &=  ~BIT0)
#define	mUsbFIFO1DMADis()			(bFUSBPort(0x7e	) &=  ~BIT1)
#define	mUsbFIFO2DMADis()			(bFUSBPort(0x7e	) &=  ~BIT2)
#define	mUsbFIFO3DMADis()			(bFUSBPort(0x7e	) &=  ~BIT3)
#define	mUsbFIFO4DMADis()			(bFUSBPort(0x7e	) &=  ~BIT4)
#define	mUsbFIFO5DMADis()			(bFUSBPort(0x7e	) &=  ~BIT5)
#define	mUsbFIFO6DMADis()			(bFUSBPort(0x7e	) &=  ~BIT6)
#define	mUsbFIFO7DMADis()			(bFUSBPort(0x7e	) &=  ~BIT7)

#define	mUsbFIFODMAEn(off,val)			(bFUSBPort(off)	|= (val))	//john
#define	mUsbFIFODMADis(off,val)			(bFUSBPort(off)	&= ~(val))	//john

#define	mUsbEPMap(EPn, MAP)			(bFUSBPort(0x30	+ (EPn-1)) = MAP)
#define	mUsbEPMapRd(EPn)			(bFUSBPort(0x30	+ (EPn-1)))

#define	mUsbFIFOMap(FIFOn, MAP)			(bFUSBPort(0x80 + FIFOn) = MAP)
#define	mUsbFIFOMapRd(FIFOn)			(bFUSBPort(0x80 + FIFOn))

#define	mUsbFIFOConfig(FIFOn, CONFIG)		(bFUSBPort(0x90 + FIFOn) = CONFIG)
#define	mUsbFIFOConfigRd(FIFOn)			(bFUSBPort(0x90	+ FIFOn))

#define	mUsbFIFOOutByteCount(fifo_num)		(u16)(((((u16)bFUSBPort(0xA0+fifo_num))&((u16)0x07))<<8) | ((u16)bFUSBPort(0xB0+fifo_num)))
#define	mUsbFIFODone(fifo_num)			(bFUSBPort(0xA0+fifo_num) |= ((u8)BIT3))
#define	mUsbFIFOReset(fifo_num)			(bFUSBPort(0xA0+fifo_num) |=  BIT4)	//john
#define	mUsbFIFOResetOK(fifo_num)		(bFUSBPort(0xA0+fifo_num) &= ~BIT4)	//john

	///////	Read Data FIFO ////////
#define	mUsbRdByte0(FIFOn)			(bFUSBPort(0xC0	+ (FIFOn<<2)))
#define	mUsbRdByte2(FIFOn)			(bFUSBPort(0xC2	+ (FIFOn<<2)))
#define	mUsbRdWord(FIFOn)			(wFUSBPort(0xC0	+ (FIFOn<<2)))
#define	mUsbRdDWord(FIFOn)			(dwFUSBPort(0xC0 + (FIFOn<<2)))

	//john
#define	mDMAUsbRdDWord(FIFOn)			(DMAdwFUSBPort(0xC0 + (FIFOn<<2)))

	///////	Write Data FIFO	////////
#define	mUsbWrByte0(FIFOn, value)		(bFUSBPort(0xC0	+ (FIFOn<<2)) =	value)
#define	mUsbWrByte2(FIFOn, value)		(bFUSBPort(0xC2	+ (FIFOn<<2)) =	value)
#define	mUsbWrWord(FIFOn, value)		(wFUSBPort(0xC0	+ (FIFOn<<2)) =	value)
#define	mUsbWrDWord(FIFOn, value)		(dwFUSBPort(0xC0 + (FIFOn<<2)) = value)

#endif				/* __FUSB220_M_H  */
