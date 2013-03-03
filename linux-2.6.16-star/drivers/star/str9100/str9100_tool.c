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


#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,32)
#define LINUX24 1
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26 1
#endif

#ifdef LINUX24
#include <asm/arch/str9100/star_tool.h>
#endif

#ifdef LINUX26
#include <linux/str9100/str9100_tool.h>
#endif

#include <linux/proc_fs.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/delay.h>



#ifdef LINUX24
// for star_tool.h create_proc_str9100()
//int create_proc_dir=0;


struct proc_dir_entry *create_proc_str9100(const char* proc_str)
{
	struct proc_dir_entry *de;
	struct proc_dir_entry *str9100_gsw_procdir=0;


        de = &proc_root;
        for (de = de->subdir; de ; de = de->next) {
		if (strcmp(de->name, "str9100")==0) // find it
		{
			return de;
		}
        }

	// not found /proc/str9100, so create it.

	
       	str9100_gsw_procdir=proc_mkdir(proc_str, NULL);
       	if (str9100_gsw_procdir==0)
		return 0;


	return str9100_gsw_procdir;
}
#endif



#define CONFIG_GET_FLASH_VAR
#ifdef CONFIG_GET_FLASH_VAR
// add by descent 2006/07/20

/* Add for read MAC from FLASH. */
#ifndef __ARM_BE__
#define B0(h)   ((h) & 0xFF)
#define B1(h)   (((h) >> 8) & 0xFF)
#else
#define B0(h)   (((h) >> 8) & 0xFF)
#define B1(h)   ((h) & 0xFF) 
#endif

void copy_from_flash(unsigned long from, void *to,ssize_t len)
{   
	int i;  
	u8 *dest = (u8*)to;
	u16 data;
	unsigned long remap = (unsigned long)ioremap(FLASH_BASE_ADDR, FLASH_SIZE);
	u16 *src = (u16 *)(remap + from);

	for (i=0; i < (len / 2); i++) {
		data = src[i];
		dest[i * 2] = B0(data);
		dest[i * 2 + 1] = B1(data);
	}

	if (len & 1) {
		dest[len - 1] = B0(src[i]);
	}
#ifdef LINUX24
	iounmap((void *)remap);
#else
	iounmap((void __iomem *)remap);
#endif
}

#if 0
char *get_flash_env(const char *env_name)
{
	const int ENV_SIZE=0x8000;
	//const char *ENV_BEGIN=FLASH_ADDRESS(0x10000000)+0x20000;
	//const char *ENV_BEGIN=FLASH_ADDRESS(0x10020000);
	unsigned long from=0x20000;

        volatile unsigned long remap = (unsigned long)ioremap(FLASH_BASE_ADDR, FLASH_SIZE);
        volatile u8 *src = (u16 *)(remap + from);

	int i=0;
	char *str_p=env_name;
	char *beg_p=src;
	char *p;
	 

	while(1) {
		p=strstr(beg_p, env_name);
		if (p) { // found
			char *asign_p=strchr(p, '=');
			if (asign_p)
			{
        			iounmap(remap);
				return asign_p+1;
			}
			else
				break; // should not this case
		}
		else
		{
			++beg_p;
		}

		if (p > src+ENV_SIZE) {
			break;
		}
	}
        iounmap(remap);
	return 0; // not found
}
#else
// copy from 2.6.16
char *get_flash_env(const char *env_name)
{
	const int ENV_SIZE = 0x8000;
	unsigned long from = 0x20000;

	u8 *remap = ioremap(FLASH_BASE_ADDR, FLASH_SIZE);
	volatile u8 *src = (volatile u8 *)(remap + from);

	char *str_p = (char *)env_name;
	char *beg_p = (char *)src;
	char *p;

	while (1) {
		p = strstr(beg_p, str_p);
		if (p) { // found
			char *asign_p = strchr(p, '=');
			if (asign_p) {
				iounmap(remap);
				return asign_p + 1;
			} else
				break; // should not this case
		} else {
			++beg_p;
		}
		if (p > ((char *)src + ENV_SIZE)) {
			break;
		}
	}
        iounmap(remap);
	return 0; // not found
}

#endif
#endif // ifdef CONFIG_GET_FLASH_VAR


/*
 *  * MXIC's flash manufacture ID
 *   */
#define MX_MANUFACT     0x00C200C2      /* MXIC    manuf. ID in D23..D16, D7..D0 */


#define MXIC_MANUFACTURE_ID             0x00C20000

/*
 *  * MXIC's flash device ID
 *   */
#define MXIC_DEVICE_ID_MX29LV320B       0x000000A8
#define MX_ID_LV640BB       0x22CB22CB      /* 29LV640BB by Macronix, AMD compatible */
#define MX_ID_LV640BT       0x22C922C9      /* 29LV640BT by Macronix, AMD compatible */

#ifdef LINUX24
const char *get_flash_type(volatile u16 *saddr)
{
	short i;
	u16 mid;
	u16 did;
	int name_index=0;
	const char *flash_name[]={
	                           0,
	                           "EON_EN29LV640HL(8MB)",
	                           "MXIC_MX29LV640BT(8MB)"
	                         };
	//u32  base = (u32)addr;

        //volatile unsigned long remap = (unsigned long)ioremap(FLASH_BASE_ADDR, FLASH_SIZE);
        //volatile u16 *saddr = (u16 *)(remap + 0);

        //volatile u8 *src = (u16 *)(remap + 0);
	//volatile u8 *saddr = src;

	/* Write auto select command: read Manufacturer ID */
	saddr[0x555] = 0xAA;
	saddr[0x2AA] = 0x55;
	saddr[0x555] = 0x90;

	mid = saddr[0];
	did = saddr[1];

	if ( ((mid & 0xffff) == 0x007f) && ((did & 0xFFFF) == 0x227e) ) // "EON_EN29LV640HL(8MB)"
		name_index=1;
	if ( ((mid & 0xffff) == 0x00c2) && ((did & 0xFFFF) == 0x22c9) ) // "MXIC_MX29LV640BT(8MB)"
		name_index=2;



	/* reset to read mode */
	saddr[0] = 0xF0; /* reset the bank */
	udelay(10000);

	return flash_name[name_index];
}
#endif

#ifdef LINUX26
const char *get_flash_type(void)
{
	u8 *remap = ioremap(FLASH_BASE_ADDR, FLASH_SIZE);
	volatile u16 *saddr = (volatile u16 *)remap;
	u16 mid;
	u16 did;
	int name_index = 0;
	const char *flash_name[] = {
		0,
		"EON_EN29LV640HL(8MB)",
		"MXIC_MX29LV640BT(8MB)"
	};

	/* Write auto select command: read Manufacturer ID */
	saddr[0x555] = 0xAA;
	saddr[0x2AA] = 0x55;
	saddr[0x555] = 0x90;

	mid = saddr[0];
	did = saddr[1];

	if (((mid & 0xffff) == 0x007f) && ((did & 0xFFFF) == 0x227e) ) // "EON_EN29LV640HL(8MB)"
		name_index=1;
	if (((mid & 0xffff) == 0x00c2) && ((did & 0xFFFF) == 0x22c9) ) // "MXIC_MX29LV640BT(8MB)"
		name_index=2;

	/* reset to read mode */
	saddr[0] = 0xF0; /* reset the bank */
	udelay(1000);

	iounmap(remap);

	return flash_name[name_index];
}
#endif

