/* cnXXxxWdt.c - Cavium CN30XX/CN50XX watchdog driver */

/* Copyright (c) 2009, TeamF1, Inc. */

/*
modification history
--------------------
01d,09Dec09,sg   adding this file for 336g and srxn3205
01c,04aug09,anp  added support for resetting the timer from kernel space.
01b,24jul09,anp  added support for CN50XX CPU.
01a,21jul09,anp  written.
*/

/*
DESCRIPTION
Cavium cn30xx has a watchdog timer which can be programmed by following 
registers:
  o CN30XX_CIU_WDOG0    - this register starts and initializes the watchdog 
                          timer.
  o CN30XX_CIU_PP_POKE0 - this is the register, which our driver should poll to.
                          This will tell the watchdog timer that our kernel is
                          still alive.

Cavium cn50xx is a two core, processor, so it has two watchdog timers. 
The timers are very much similar to CN30XX.

This driver supports two implementations:
  o Driver will be an interface between an user land daemon and the watchdog 
    timer. The user land daemon will send a HEART_BEAT to the driver at regular 
    intervals and the driver will in turn reset the watchdog timer.
  o Driver will itself be responsible for generating the HEART_BEAT. In this
    implementation user space daemon is not required.

Driver should be working in _only_ one mode. Working mode can be configured by
the macro 'CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE'.

In the first implementation, once the user land daemon opens the device node, 
the driver will start the watchdog timer. The user land daemon will then ioctl 
the driver for HEART_BEAT, at regular intervals. On receiving these ioctls, 
driver will reset the watchdog timer. Once there is some problem in user land 
or even in kernel, driver will not be able to reset the watchdog timer and 
eventually watchdog timer will reset the CPU.

 ____________                       __________         __________________
|            |                     |          |       |                  |
| watchDogd  |--->---|ioctl|--->---|  driver  |--->---|  watchdog timer  |
|____________|                     |__________|       |__________________|

                  Fig: watchdog interfacing with userspace.

In the second implementation, driver configures the watchdod timer and starts a 
kernel timer. This kernel timer is responsible to reset the hardware watchdog 
timer. Say once the kernel is dead, there will be no resetting of hardware 
watchdog timer, and it will reset the CPU.
         __________         ______         __________________
        |          |       |      |       |                  |
        |  driver  |--->---| Poke |--->---|  watchdog timer  |
        |__________|       |______|       |__________________|

               Fig: watchdog interfacing with driver.


INCLUDE FILES: linux/module.h linux/fs.h linux/watchdog.h
*/

/* includes */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/watchdog.h>
#include <linux/timer.h>

/* defines */

#define OK                                   0
#define ERROR                                -1

#define CVM_CNXXXX_WDT_DEV                   "watchdog"

#define CVM_CNXXXX_CIU_WDOG0                 0x8001070000000500ull
#define CVM_CNXXXX_CIU_WDOG1                 0x8001070000000508ull
#define CVM_CNXXXX_CIU_PP_POKE0              0x8001070000000580ull
#define CVM_CNXXXX_CIU_PP_POKE1              0x8001070000000588ull

#define CVM_CNXXXX_WDT_TIMEOUT               0xFFFF3 /* ~6 secs/timeout */
#define CVM_CNXXXX_POKE_INTERVAL             HZ*1    /* 1 second */

#undef CVM_CNXXXX_WATHDOG_DEBUG

/* undef it for CN50XX */
#define CVM_ARCH_CN30XX

/* define to handle the timer from user space daemon watchDogd */
#undef CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE

MODULE_AUTHOR ("TeamF1, Inc. <support@TeamF1.com>");
MODULE_DESCRIPTION ("Cavium cnXXxx watchdog timer driver");

/* typedefs */

/* globals */

/* locals */

static        int        majDevNum = 0; /* major device number   */        
static        int        devAccess = 0; /* device access control */
static struct timer_list wdTimer;

/* forward declarations */

static int  cnXXxxWdtIoctl       (struct inode *, struct file *,
                                  unsigned int , unsigned long);
static int  cnXXxxWdtOpen        (struct inode *, struct file *);
static int  cnXXxxWdtRelease     (struct inode *, struct file *);
static long cnXXxxWdtCompatIoctl (struct file  *, unsigned int, unsigned long);

struct file_operations fOps =
    {
    .owner        = THIS_MODULE,
    .open         = cnXXxxWdtOpen,
    .release      = cnXXxxWdtRelease, 
    .ioctl        = cnXXxxWdtIoctl,      
#ifdef CONFIG_COMPAT
    .compat_ioctl = cnXXxxWdtCompatIoctl,
#endif /* CONFIG_COMPAT */       
    };

/*******************************************************************************
*
* cnXXxxWdtCsrWrite - writes to a Central Interrupt Unit register.
*
* This routine writes 64 bit data to a Central Interrupt Unit register of 
* Cavium Octeon CNXXXX CPU.
*
* RETURNS: N/A
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtCsrRead()
*/

static inline void cnXXxxWdtCsrWrite
    (
    uint64_t csrAddr, /* IN: CIU register address */ 
    uint64_t val      /* IN: value                */
    )
    {
    *(volatile uint64_t *) csrAddr = val;
    }

/*******************************************************************************
*
* cnXXxxWdtCsrRead - reads from a Central Interrupt Unit register.
*
* This routine reads 64 bit data from a Central Interrupt Unit register of 
* Cavium Octeon CNXXXX CPU.
*
* RETURNS: uint64_t, value present in CIU register (csrAddr). 
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtCsrWrite()
*/

static inline uint64_t cnXXxxWdtCsrRead
    (
    uint64_t csrAddr  /* IN: CIU register address */
    )
    {
    return *(volatile uint64_t *) csrAddr;
    }

/*******************************************************************************
*
* cnXXxxWdtPoll - polls the watchdog timer.
*
* This routine polls the cnXXxx watchdog timer telling that kernel is alive.
*
* RETURNS: N/A
*
* ERRNO: N/A
*
* SEE_ALSO: N/A
*/

static void cnXXxxWdtPoll (void)
    {
    /* we are inside the poll routine, so lets poll to watchdog timer */
    /* set the CNXXXX_CIU_PP_POKE0 to any value. A write operation will tell the
     * watchdog timer that kernel is alive.
     */
#ifdef CVM_CNXXXX_WATHDOG_DEBUG
    printk (KERN_DEBUG"cnXXxxWdtPoll: resetting the watchdog timer\n");
#endif
    cnXXxxWdtCsrWrite (CVM_CNXXXX_CIU_PP_POKE0, 0x1);
#ifndef CVM_ARCH_CN30XX
    cnXXxxWdtCsrWrite (CVM_CNXXXX_CIU_PP_POKE1, 0x1);
#endif /* CVM_ARCH_CN30XX */

#ifndef CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE
    /* restart the kernel timer */
    mod_timer (&wdTimer, jiffies + CVM_CNXXXX_POKE_INTERVAL);
#endif /* CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE */
    }

/******************************************************************************
* 
* cnXXxxWdtIoctl - cnsXXxxWdt ioctl implementation.
* 
* This routine is ioctl implementation for cnXXxx watchdog timer driver.
*
* RETURNS: OK on success, -EOPNOTSUPP otherwise.
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtCompatIoctl()
*/

int cnXXxxWdtIoctl 
    (
    struct   inode * pInode, 
    struct   file  * pFlip, 
    unsigned int     cmd, 
    unsigned long    arg
    )
    {
#ifdef CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE   
    switch (cmd)
        {
        case WDIOC_KEEPALIVE:
            {
            cnXXxxWdtPoll();

            break;
            }

        default:
            {
            printk (KERN_INFO"cnXXxxWdtIoctl: unsupported ioctl\n");

            return -EOPNOTSUPP;
            }
        }
#endif /* CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE */

    /* ioctl successful */
    return OK;
    }

#ifdef CONFIG_COMPAT
/*******************************************************************************
*
* cnXXxxWdtCompatIoctl - cnXXxxWdt ioctl implementation.
*
* This routine implements 32 bit user space compatible IOCTLs for 64 bit kernel
* for cnXXxx watchdog timer driver.
*
* RETURNS: OK on success, ERROR otherwise.
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtIoctl()
*/

long cnXXxxWdtCompatIoctl
    (
    struct   file * pFile,
    unsigned int    cmd,
    unsigned long   arg
    )
    {
    return cnXXxxWdtIoctl (pFile->f_dentry->d_inode, pFile, cmd, arg);
    }
#endif /* CONFIG_COMPAT */

/*******************************************************************************
*
* cnXXxxWdtOpen - device open call
*
* This routine is called when device file is opened.
*
* RETURNS: OK on success, -EBUSY otherwise.
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtRelease()
*/

int cnXXxxWdtOpen 
    (
    struct inode * inode, /* IN: inode pointer */
    struct file  * file   /* IN: file pointer  */
    )
    {

    if (devAccess) 
        return -EBUSY; /* device is already busy */

    devAccess++; /* we won't allow anyone else */

    /* start the watchdog timer
     * initialize the cnXXxx watchdog timer. Set the register CNXXXX_CIU_WDOGX 
     * with proper values.
     */
#ifdef CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE
    cnXXxxWdtCsrWrite (CVM_CNXXXX_CIU_WDOG0, CVM_CNXXXX_WDT_TIMEOUT);
#ifndef CVM_ARCH_CN30XX
    cnXXxxWdtCsrWrite (CVM_CNXXXX_CIU_WDOG1, CVM_CNXXXX_WDT_TIMEOUT);
#endif /* CVM_ARCH_CN30XX */
    printk (KERN_INFO"watchdog timer initialized\n");
#endif /* CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE */

    /* watchdog timer started */
    return OK;
    }
    
/*******************************************************************************
*
* cnXXxxWdtRelease - device close call
*
* This routine is called when the device file is closed.
*
* RETURNS: OK
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtOpen()
*/

static int cnXXxxWdtRelease 
    (
    struct inode * inode, /* IN: inode pointer  */
    struct file  * file   /* IN: device pointer */
    )
    { 
    devAccess--; /* we are ready for next access */
    
    return OK;
    }

/*******************************************************************************
*
* cnXXxxWdtInit - module initialization
*
* This routine does all the initialization when this module is inserted.
*
* RETURNS: OK on success, majDevNum otherwise
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtExit()
*/

static int __init cnXXxxWdtInit (void)
    {

    /* register our driver */
    majDevNum = register_chrdev (majDevNum, CVM_CNXXXX_WDT_DEV, &fOps);
    if (majDevNum < 0)
        {
        printk (KERN_INFO"cnXXxxWdtInit: device registration failed\n");
        return majDevNum;
        }
    printk (KERN_DEBUG "cnXXxxWdtInit: device registration successful, Major No"
                       ": %d\n", majDevNum);

#ifndef CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE
    cnXXxxWdtCsrWrite (CVM_CNXXXX_CIU_WDOG0, CVM_CNXXXX_WDT_TIMEOUT);

     /* initialize the kernel timer */
    init_timer (&wdTimer);
    wdTimer.function = cnXXxxWdtPoll;
    wdTimer.data = 0;
    wdTimer.expires = jiffies + CVM_CNXXXX_POKE_INTERVAL;
    add_timer (&wdTimer);
#endif /* CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE */

    return OK;
    }

/*******************************************************************************
*
* cnXXxxWdtExit - module de-initialization.
*
* This routine does all the cleanup when this module is unloaded.
*
* RETURNS: N/A
*
* ERRNO: N/A
*
* SEE_ALSO: cnXXxxWdtInit()
*/

static void __exit cnXXxxWdtExit (void)
    {

    /* unregister the device */
    unregister_chrdev (majDevNum, CVM_CNXXXX_WDT_DEV);
#ifndef CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE
    del_timer (&wdTimer);
    cnXXxxWdtCsrWrite (CVM_CNXXXX_CIU_WDOG0, 0x00);
#ifndef CVM_ARCH_CN30XX
    cnXXxxWdtCsrWrite (CVM_CNXXXX_CIU_WDOG1, 0x00);
#endif /* CVM_ARCH_CN30XX */
    printk (KERN_INFO"watchdog timer disabled\n");
#endif /* CVM_CNXXX_WDT_HNDL_FROM_USER_SPACE */
    return;
    }

module_init (cnXXxxWdtInit);
module_exit (cnXXxxWdtExit);
