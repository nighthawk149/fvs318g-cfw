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

#ifndef	_STAR_PCMCIA_DRIDGE_H_
#define	_STAR_PCMCIA_DRIDGE_H_

/******************************************************************************
 * MODULE NAME:	   star_pcmcia_bridge.h
 * PROJECT CODE:   Equuleus
 * DESCRIPTION:
 * MAINTAINER:	   Eric	Yang
 * DATE:	   15 September	2005
 *
 * SOURCE CONTROL:
 *
 * LICENSE:
 *     This source code	is copyright (c) 2005 Star Semi	Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *     15 September 2005  -  Eric Yang	- Initial Version v1.0
 *
 *
 * SOURCE:
 * ISSUES:
 * NOTES TO USERS:
 ******************************************************************************/

#include "star_sys_memory_map.h"


#if defined(__UBOOT__)
#define	PCMCIA_BRIDGE_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSPA_PCMCIA_CONTROL_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	PCMCIA_BRIDGE_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSVA_PCMCIA_CONTROL_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * define access macros
 */
#define	PCMCIA_CONFIGURATION_REG			PCMCIA_BRIDGE_MEM_MAP_VALUE(0x20)
#define	PCMCIA_MEMORY_ACCESS_TIMING_PARAM_REG		PCMCIA_BRIDGE_MEM_MAP_VALUE(0x24)
#define	PCMCIA_IO_ACCESS_TIMING_PARAM_REG		PCMCIA_BRIDGE_MEM_MAP_VALUE(0x28)


#define	PCMCIA_ATTRIBUTE_MEMORY_SPACE_BASE_ADDR		(SYSPA_PCMCIA_ATTRIBUTE_MEMORY_BASE_ADDR)
#define	PCMCIA_COMMOM_MEMORY_SPACE_BASE_ADDR		(SYSPA_PCMCIA_COMMON_MEMORY_BASE_ADDR)
#define	PCMCIA_IO_SPACE_BASE_ADDR			(SYSPA_PCMCIA_IO_SPACE_BASE_ADDR)



/*
 * define constants macros
 */
#define	PCMCIA_DATA_BUS_WIDTH_8		(0)

#define	PCMCIA_DATA_BUS_WIDTH_16	(1)


/*
 * Flags for PCMCIA_STATUS
 */
#define	FLAG_STATUS_BVD1		0x01
#define	FLAG_STATUS_STSCHG		0x01
#define	FLAG_STATUS_BVD2		0x02
#define	FLAG_STATUS_SPKR		0x02
#define	FLAG_STATUS_DETECT		0xf3	  /* bit 2=0,3=0 ,0x0c bit 2=1,3=1 */
#define	FLAG_STATUS_WRPROT		0x10
#define	FLAG_STATUS_READY		0x20
#define	FLAG_STATUS_INPACK		0x40


/*
 * Flags for PCMCIA_CSC
 */
#define	FLAG_CSC_BVD1			0x01
#define	FLAG_CSC_BVD2			0x02
#define	FLAG_CSC_READY			0x04
#define	FLAG_CSC_INPACK			0x08
#define	FLAG_CSC_STSCHG			0x10
#define	FLAG_CSC_CARDINT		0x20
#define	FLAG_CSC_DETECT			0x40
#define	FLAG_CSC_SWCDC			0x80


/*
 * Flags for PCMCIA_POWER
 */
#define	FLAG_POWER_OFF			0x00	  /* Turn off the socket */
#define	FLAG_POWER_3V			0x01	  /* 1:Vcc = 3.3v 0:Vcc	= 5.0v */
#define	FLAG_POWER_SWH			0x02	  /* Direct 5V/3V switch enable	*/
#define	FLAG_POWER_CTL			0x10	  /* Socket power control */
#define	FLAG_POWER_AUTO			0x20	  /* Auto power	switch enable */
#define	FLAG_POWER_OUTENA		0x40	  /* Output enable */


/*
 * Flags for PCMCIA_GBLCTL
 */
#define	FLAG_GBLCTL_PWRDOWN		0x01
#define	FLAG_GBLCTL_WBACK		0x02
#define	FLAG_GBLCTL_16BITS		0x04
#define	FLAG_GBLCTL_IOCARD		0x08
#define	FLAG_GBLCTL_SWCDINT		0x10
#define	FLAG_GBLCTL_RESET		0x20


/*
 * Flags for PCMCIA_INTCFG
 */
#define	FLAG_INTCFG_BDEAD		0x01
#define	FLAG_INTCFG_BWARN		0x02
#define	FLAG_INTCFG_READY		0x04
#define	FLAG_INTCFG_INPACK		0x08
#define	FLAG_INTCFG_LEVEL		0x10
#define	FLAG_INTCFG_FEDGE		0x20
#define	FLAG_INTCFG_REDGE		0x30
#define	FLAG_INTCFG_DETECT		0x40
#define	FLAG_INTCFG_STSCHG		0x80


/*
 * Definitions for Card	Status flags for GetStatus
 */
#define	STATUS_BATDEAD			0x0001
#define	STATUS_BATWARN			0x0002
#define	STATUS_DETECT			0x0004
#define	STATUS_WRPROT			0x0008
#define	STATUS_READY			0x0010
#define	STATUS_INPACK			0x0020
#define	STATUS_STSCHG			0x0040    /* just for	CSC */
#define	SOFTWARE_STATUS_DETECT		0x0040    /* just for	CSC */


/*
 * Set Socket configuration flags
 */
#define	SS_PWR_AUTO			0x0001
#define	SS_PWR_SWH			0x0002
#define	SS_PWR_SEL			0x0004
#define	SS_POWER_ON			0x0008
#define	SS_OUTPUT_ENA			0x0010
#define	SS_IOCARD			0x0020
#define	SS_RESET			0x0040
#define	SS_WBACK			0x0080
#define	SS_16BITS			0x0100
#define	SS_PWR_DOWN_MODE		0x0200
#define	SS_SWCDINT			0x0400


/*
 * Set Interrupt Configuration flags
 */
#define	INTR_BATDEAD			0x0001
#define	INTR_BATWARN			0x0002
#define	INTR_READY			0x0004
#define	INTR_INPACK			0x0008
#define	INTR_CARDINT			0x0010
#define	INTR_DETECT			0x0020
#define	INTR_STSCHG			0x0040


/*
 * tuple code
 */
#define	CISTPL_NULL			0x00
#define	CISTPL_DEVICE			0x01
#define	CISTPL_NO_LINK			0x14
#define	CISTPL_VERS_1			0x15
#define	CISTPL_CONFIG			0x1a
#define	CISTPL_CFTABLE_ENTRY		0x1b
#define	CISTPL_MANFID			0x20
#define	CISTPL_END			0xff


/*
 * Return codes
 */
#define	CS_SUCCESS			0x00
#define	CS_UNSUPPORTED_FUNCTION		0x15
#define	CS_NO_MORE_ITEMS		0x1f
#define	CS_BAD_TUPLE			0x40


/*
 * Attributes for tuple	calls
 */
#define	TUPLE_RETURN_LINK		0x01
#define	TUPLE_RETURN_COMMON		0x02

#define	RETURN_FIRST_TUPLE		0xff


/*
 * macro declarations
 */
#define	HAL_PCMCIA_ENABLE_PCMCIA_CONTROLLER() \
{ \
    (PCMCIA_CONFIGURATION_REG) |= (0x1 << 1); \
}

#define	HAL_PCMCIA_DISABLE_PCMCIA_CONTROLLER() \
{ \
    (PCMCIA_CONFIGURATION_REG) &= ~(0x1	<< 1); \
}


#endif	// end of #ifndef _STAR_PCMCIA_DRIDGE_H_
