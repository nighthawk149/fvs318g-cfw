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


#ifndef	_STAR_NIC_H_
#define	_STAR_NIC_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define	NIC_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSPA_NIC_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	NIC_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSVA_NIC_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * define access macros
 */
#define	NIC_PHY_CONTROL_REG0			NIC_MEM_MAP_VALUE(0x000)
#define	NIC_PHY_CONTROL_REG1			NIC_MEM_MAP_VALUE(0x004)

#define	NIC_MAC_CONTROL_REG			NIC_MEM_MAP_VALUE(0x008)
#define	NIC_FLOW_CONTROL_CONFIG_REG		NIC_MEM_MAP_VALUE(0x00C)

#define	NIC_ARL_CONFIG_REG			NIC_MEM_MAP_VALUE(0x010)

#define	NIC_MY_MAC_HIGH_BYTE_REG		NIC_MEM_MAP_VALUE(0x014)
#define	NIC_MY_MAC_LOW_BYTE_REG			NIC_MEM_MAP_VALUE(0x018)

#define	NIC_HASH_TABLE_CONTROL_REG		NIC_MEM_MAP_VALUE(0x01C)

#define	NIC_MY_VLANID_CONTROL_REG		NIC_MEM_MAP_VALUE(0x020)

#define	NIC_MY_VLANID_0_1			NIC_MEM_MAP_VALUE(0x024)
#define	NIC_MY_VLANID_2_3			NIC_MEM_MAP_VALUE(0x028)

#define	NIC_DMA_CONFIG_REG			NIC_MEM_MAP_VALUE(0x030)
#define	NIC_TX_DMA_CONTROL_REG			NIC_MEM_MAP_VALUE(0x034)
#define	NIC_RX_DMA_CONTROL_REG			NIC_MEM_MAP_VALUE(0x038)
#define	NIC_TX_DESC_PTR_REG			NIC_MEM_MAP_VALUE(0x03C)
#define	NIC_RX_DESC_PTR_REG			NIC_MEM_MAP_VALUE(0x040)

#define	NIC_TX_DESC_BASE_ADDR_REG		NIC_MEM_MAP_VALUE(0x044)
#define	NIC_RX_DESC_BASE_ADDR_REG		NIC_MEM_MAP_VALUE(0x048)
#define	NIC_DELAYED_INT_CONFIG_REG		NIC_MEM_MAP_VALUE(0x04C)

#define	NIC_INT_STATUS_REG			NIC_MEM_MAP_VALUE(0x050)
#define	NIC_INT_MASK_REG			NIC_MEM_MAP_VALUE(0x054)

#define	NIC_TEST_0_REG				NIC_MEM_MAP_VALUE(0x058)
#define	NIC_TEST_1_REG				NIC_MEM_MAP_VALUE(0x05C)

#define	NIC_MIB_RX_OK_PKT_CNTR			NIC_MEM_MAP_VALUE(0x100)
#define	NIC_MIB_RX_OK_BYTE_CNTR			NIC_MEM_MAP_VALUE(0x104)
#define	NIC_MIB_RX_RUNT_BYTE_CNTR		NIC_MEM_MAP_VALUE(0x108)
#define	NIC_MIB_RX_OSIZE_DROP_PKT_CNTR		NIC_MEM_MAP_VALUE(0x10C)

#define	NIC_MIB_RX_NO_BUF_DROP_PKT_CNTR		NIC_MEM_MAP_VALUE(0x110)

#define	NIC_MIB_RX_CRC_ERR_PKT_CNTR		NIC_MEM_MAP_VALUE(0x114)

#define	NIC_MIB_RX_ARL_DROP_PKT_CNTR		NIC_MEM_MAP_VALUE(0x118)

#define	NIC_MIB_MYVLANID_MISMATCH_DROP_PKT_CNTR	NIC_MEM_MAP_VALUE(0x11C)

#define	NIC_MIB_RX_CHKSUM_ERR_PKT_CNTR		NIC_MEM_MAP_VALUE(0x120)

#define	NIC_MIB_RX_PAUSE_FRAME_PKT_CNTR		NIC_MEM_MAP_VALUE(0x124)

#define	NIC_MIB_TX_OK_PKT_CNTR			NIC_MEM_MAP_VALUE(0x128)
#define	NIC_MIB_TX_OK_BYTE_CNTR			NIC_MEM_MAP_VALUE(0x12C)

#define	NIC_MIB_TX_COLLISION_CNTR		NIC_MEM_MAP_VALUE(0x130)
#define	NIC_MIB_TX_PAUSE_FRAME_CNTR		NIC_MEM_MAP_VALUE(0x130)

#define	NIC_MIB_TX_FIFO_UNDERRUN_RETX_CNTR	NIC_MEM_MAP_VALUE(0x134)




/*
 * define constants macros
 */

#define	NIC_PHY_ADDRESS		1 //the phy addr const	value
#define	NIC_PHY_ID		0x0243	//the phy id

#define	GW_NIC_MAX_TFD_NUM	(32)
#define	GW_NIC_MAX_RFD_NUM	(32)
#define	MAX_BUFFERS		(64)



#define	MMU_OFF			(0)
#define	MMU_ON			(1)
#define	OS_NULL			(0)


#define	NET_BUFFER_PACKET_SIZE		(512)
#define	NET_BUFFER_SHIFT_BIT_NUM	(9)	// 2*n9=512

#define	MAX_PACKET_LEN		(1536)

#define	INTERNAL_LOOPBACK_MODE	(1)
#define	SOFTWARE_REPEATER_MODE	(2)

#define	TXTC_INT_BIT		(0x08000000)
#define	TX_INSV_BIT		(0x04000000)

#define	LS_BIT			(0x10000000)
#define	FS_BIT			(0x20000000)
#define	EOR_BIT			(0x40000000)
#define	FS_LS_BIT		(0x30000000)
#define	C_BIT			(0x80000000)
#define	FS_LS_C_BIT		(0xB0000000)
#define	FS_LS_INT_BIT		(0x38000000)



// HASH	TABLE CONTROL REGISTER
#define	NIC_HASH_TABLE_BIST_DONE_BIT	(0x1 <<	17)
#define	NIC_HASH_TABLE_BIST_OK_BIT	(0x1 <<	16)
#define	NIC_HASH_COMMAND_START_BIT	(0x1 <<	14)
#define	NIC_HASH_COMMAND_BIT		(0x1 <<	13)
#define	NIC_HASH_BIT_DATA		(0x1 <<	12)
#define	NIC_HASH_BIT_ADDRESS_BIT	(0x1ff)


#define	NIC_REG_CNT			((0x48 << 2) + 1)

/*
 * macro access
 */

#define	GW_NIC_TX_TFD_NEXT(work_tfd_ptr) \
    work_tfd_ptr = NIC_TX_TFD_Ring.head	+ (((u32)(work_tfd_ptr - NIC_TX_TFD_Ring.head) + 1) % GW_NIC_MAX_TFD_NUM)


#define	GW_NIC_TX_TFD_PREVIOUS(work_tfd_ptr) \
    work_tfd_ptr = NIC_TX_TFD_Ring.head	+ ((GW_NIC_MAX_TFD_NUM + (u32)(work_tfd_ptr - NIC_TX_TFD_Ring.head) - 1) % GW_NIC_MAX_TFD_NUM)


#define	GW_NIC_RX_RFD_NEXT(work_rfd_ptr) \
    work_rfd_ptr = NIC_RX_RFD_Ring.head	+ (((u32)(work_rfd_ptr - NIC_RX_RFD_Ring.head) + 1) % GW_NIC_MAX_RFD_NUM)


#define	GW_NIC_RX_RFD_PREVIOUS(work_rfd_ptr) \
    work_rfd_ptr = NIC_RX_RFD_Ring.head	+ ((GW_NIC_MAX_RFD_NUM + (u32)(work_rfd_ptr - NIC_RX_RFD_Ring.head) - 1) % GW_NIC_MAX_RFD_NUM)


/*
 * PHY register	defines
 */
#define	PHY_MII_CONTROL_REG_ADDR		0x00
#define	PHY_MII_STATUS_REG_ADDR			0x01
#define	PHY_ID1_REG_ADDR			0x02
#define	PHY_ID2_REG_ADDR			0x03
#define	PHY_AN_ADVERTISEMENT_REG_ADDR		0x04
#define	PHY_AN_REAMOTE_CAP_REG_ADDR		0x05


#define	PHY_RESERVED1_REG_ADDR			0x10
#define	PHY_RESERVED2_REG_ADDR			0x11
#define	PHY_CH_STATUS_OUTPUT_REG_ADDR		0x12
#define	PHY_RESERVED3_REG_ADDR			0x13
#define	PHY_RESERVED4_REG_ADDR			0x14


#define	PHY_SPEC_CONTROL_REG_ADDR		0x16
#define	PHY_INTC_CONTROL_STATUS_REG_ADDR	0x17

/*
 * NIC registers access	macros defines
 */

//0x004
#define	HAL_NIC_WRITE_PHY_CONTROL1(config_value) \
    ((NIC_PHY_CONTROL_REG1) = (config_value))

#define	HAL_NIC_READ_PHY_CONTROL1(config_value)	\
    ((config_value) = (NIC_PHY_CONTROL_REG1))

//0x008
#define	HAL_NIC_WRITE_MAC_CONFIGURATION(config_value) \
    ((NIC_MAC_CONTROL_REG) = (config_value))

#define	HAL_NIC_READ_MAC_CONFIGURATION(config_value) \
    ((config_value) = (NIC_MAC_CONTROL_REG))

//0x00C
#define	HAL_NIC_WRITE_FLOW_CONTROL_CONFIG(fc_cfg) \
    ((NIC_FLOW_CONTROL_CONFIG_REG) = (fc_cfg))

#define	HAL_NIC_READ_FLOW_CONTROL_CONFIG(fc_cfg) \
    ((fc_cfg) =	(NIC_FLOW_CONTROL_CONFIG_REG))

//0x010
#define	HAL_NIC_WRITE_ARL_CONFIGURATION(cfg) \
    ((NIC_ARL_CONFIG_REG) = (cfg))

#define	HAL_NIC_READ_ARL_CONFIGURATION(cfg) \
    ((cfg) = (NIC_ARL_CONFIG_REG))

//0x014,
#define	HAL_NIC_WRITE_MY_MAC_HIGH_BYTE(cfg) \
    ((NIC_MY_MAC_HIGH_BYTE_REG)	= (cfg & 0x0000FFFF ) )

#define	HAL_NIC_READ_MY_MAC_HIGH_BYTE(cfg) \
    ((cfg) = (NIC_MY_MAC_HIGH_BYTE_REG & 0x0000FFFF ))

//0x018
#define	HAL_NIC_WRITE_MY_MAC_LOW_BYTE(cfg) \
    ((NIC_MY_MAC_LOW_BYTE_REG) = (cfg))

#define	HAL_NIC_READ_MY_MAC_LOW_BYTE(cfg) \
    ((cfg) = (NIC_MY_MAC_LOW_BYTE_REG))

//0x03C
#define	HAL_NIC_READ_INTERRUPT_STATUS(int_status) \
    ((int_status) = (NIC_INT_STATUS_REG))

#define	HAL_NIC_CLEAR_ALL_INTERRUPT_STATUS_SOURCES()\
    ((NIC_INT_STATUS_REG) = (0xFFFFFFFF))

#define	HAL_NIC_CLEAR_INTERRUPT_STATUS_SOURCES(source) \
    ((NIC_INT_STATUS_REG) |= (source))

#define	HAL_NIC_CLEAR_INTERRUPT_STATUS_SOURCE_BIT(source_bit_index) \
    ((NIC_INT_STATUS_REG) |= (1	<< (source_bit_index)))

//0x040
#define	HAL_NIC_DISABLE_ALL_INTERRUPT_STATUS_SOURCES() \
    ((NIC_INT_MASK_REG)	= (0xFFFFFFFF))

#define	HAL_NIC_ENABLE_ALL_INTERRUPT_STATUS_SOURCES() \
    ((NIC_INT_MASK_REG)	= (0x00000000))

#define	HAL_NIC_DISABLE_INTERRUPT_STATUS_SOURCE_BIT(source_bit_index) \
    ((NIC_INT_MASK_REG)	|= (1 << (source_bit_index)))

#define	HAL_NIC_ENABLE_INTERRUPT_STATUS_SOURCE_BIT(source_bit_index) \
    ((NIC_INT_MASK_REG)	&= ~(1 << (source_bit_index)))

//0x44
#define	HAL_NIC_WRITE_TEST0_REG(cfg) \
    ((NIC_TEST_0_REG) =	(cfg))

#define	HAL_NIC_READ_TEST0_REG(cfg) \
    ((cfg) = (NIC_TEST_0_REG))

//0x48
#define	HAL_NIC_WRITE_TEST1_REG(cfg) \
    ((NIC_TEST_1_REG) =	(cfg))

#define	HAL_NIC_READ_TEST1_REG(cfg) \
    ((cfg) = (NIC_TEST_1_REG))



/*
 * NIC's DMA macros defines
 */
#define	HAL_NIC_TX_DMA_START() \
    ((NIC_TX_DMA_CONTROL_REG) =	(1))


#define	HAL_NIC_TX_DMA_STOP() \
    ((NIC_TX_DMA_CONTROL_REG) =	(0))


#define	HAL_NIC_READ_TX_DMA_STATE(state) \
    ((state) = (NIC_TX_DMA_CONTROL_REG))


#define	HAL_NIC_RX_DMA_START() \
    ((NIC_RX_DMA_CONTROL_REG) =	(1))


#define	HAL_NIC_RX_DMA_STOP() \
    ((NIC_RX_DMA_CONTROL_REG) =	(0))


#define	HAL_NIC_WRITE_TXSD(tssd_value) \
    ((NIC_TX_DESC_PTR_REG) = (tssd_value))


#define	HAL_NIC_READ_TXSD(tssd_value) \
    ((tssd_value) = (NIC_TX_DESC_PTR_REG))


#define	HAL_NIC_WRITE_RXSD(fssd_value) \
    ((NIC_RX_DESC_PTR_REG) = (fssd_value))


#define	HAL_NIC_READ_RXSD(fssd_value) \
    ((fssd_value) = (NIC_RX_DESC_PTR_REG))


#define	HAL_NIC_WRITE_TX_BASE(ts_base_value) \
    ((NIC_TX_DESC_BASE_ADDR_REG) = (ts_base_value))


#define	HAL_NIC_READ_TX_BASE(ts_base_value) \
    ((ts_base_value) = (NIC_TX_DESC_BASE_ADDR_REG))


#define	HAL_NIC_WRITE_RX_BASE(fs_base_value) \
    ((NIC_RX_DESC_BASE_ADDR_REG) = (fs_base_value))


#define	HAL_NIC_READ_RX_BASE(fs_base_value) \
    ((fs_base_value) = (NIC_RX_DESC_BASE_ADDR_REG))


#define	HAL_NIC_WRITE_DELAYED_INTERRUPT_CONFIG(delayed_interrupt_config) \
    ((NIC_DELAYED_INT_CONFIG_REG) = (delayed_interrupt_config))


#define	HAL_NIC_READ_DELAYED_INTERRUPT_CONFIG(delayed_interrupt_config)	\
    ((delayed_interrupt_config)	= (NIC_DELAYED_INT_CONFIG_REG))

#endif	// end of #ifndef _STAR_NIC_H_
