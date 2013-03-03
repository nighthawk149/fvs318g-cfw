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

#ifndef __DEMO_DMA_H__
#define __DEMO_DMA_H__
#include <linux/types.h>	/* size_t */
#include <linux/interrupt.h>
#include <linux/module.h>

#include <asm/arch/star_dmac.h>

/*
 * defines for each channel
 */
#define DMAC_CH_DISABLE                0
#define DMAC_CH_ENABLE                 1

#define DMAC_CH_DST_SEL_M0             0
#define DMAC_CH_DST_SEL_M1             1

#define DMAC_CH_SRC_SEL_M0             0
#define DMAC_CH_SRC_SEL_M1             1

#define DMAC_CH_DSTAD_CTL_INC          0
#define DMAC_CH_DSTAD_CTL_DEC          1
#define DMAC_CH_DSTAD_CTL_FIX          2

#define DMAC_CH_SRCAD_CTL_INC          0
#define DMAC_CH_SRCAD_CTL_DEC          1
#define DMAC_CH_SRCAD_CTL_FIX          2

#define DMAC_CH_MODE_HW_HANDSHAKE      1

#define DMAC_CH_SRC_WIDTH_8_BITS       0
#define DMAC_CH_SRC_WIDTH_16_BITS      1
#define DMAC_CH_SRC_WIDTH_32_BITS      2

#define DMAC_CH_DST_WIDTH_8_BITS       0
#define DMAC_CH_DST_WIDTH_16_BITS      1
#define DMAC_CH_DST_WIDTH_32_BITS      2

#define DMAC_CH_ABT_TRANSFER           1

#define DMAC_CH_PROT1_PRIVILEGED_MODE  1
#define DMAC_CH_PROT1_USER_MODE        0

#define DMAC_CH_PROT2_BUFFERABLE       1
#define DMAC_CH_PROT2_NON_BUFFERABLE   0

#define DMAC_CH_PROT3_CACHEABLE        1
#define DMAC_CH_PROT3_NON_CACHEABLE    0

#define DMAC_CH_PRI_LEVEL_0            0
#define DMAC_CH_PRI_LEVEL_1            1
#define DMAC_CH_PRI_LEVEL_2            2
#define DMAC_CH_PRI_LEVEL_3            3

#define DMAC_CH_TC_MASK_DISABLE        0
#define DMAC_CH_TC_MASK_ENABLE         1

#define DMAC_MAX_CHANNEL_NUM           (8)


#define DMAC_CH0_ID                    (1 << 0)
#define DMAC_CH1_ID                    (1 << 1)
#define DMAC_CH2_ID                    (1 << 2)
#define DMAC_CH3_ID                    (1 << 3)
#define DMAC_CH4_ID                    (1 << 4)
#define DMAC_CH5_ID                    (1 << 5)
#define DMAC_CH6_ID                    (1 << 6)
#define DMAC_CH7_ID                    (1 << 7)
#define DMAC_CH_ID(idx)                (1 << idx) 

#define DMAC_LITTLE_ENDIAN             (0)
#define DMAC_BIG_ENDIAN                (1)

/* 
 * DMAC LLP Descriptor
 */
typedef struct _DMAC_LLP_CONTROL_    DMAC_LLP_CONTROL_T;

struct _DMAC_LLP_CONTROL_
{
//#if (ENDIAN_MODE == LITTLE_ENDIAN)
#if 1
    unsigned int    tot_size              : 12;//b11-0
    unsigned int    reserved_1            : 4; //b15-12    
    unsigned int    dst_sel               : 1; //b16
    unsigned int    src_sel               : 1; //b17    
    unsigned int    dstad_ctl             : 2; //b19-18        
    unsigned int    srcad_ctl             : 2; //b21-20        
    unsigned int    dst_width             : 3; //b24-22
    unsigned int    src_width             : 3; //b27-25
    unsigned int    tc_status_mask        : 1; //b28
    unsigned int    reserved_0            : 3; //b31-29

#else


    unsigned int    reserved_0            : 3; //b31-29
    unsigned int    tc_status_mask        : 1; //b28
    unsigned int    src_width             : 3; //b27-25
    unsigned int    dst_width             : 3; //b24-22
    unsigned int    srcad_ctl             : 2; //b21-20
    unsigned int    dstad_ctl             : 2; //b19-18    
    unsigned int    src_sel               : 1; //b17
    unsigned int    dst_sel               : 1; //b16
    unsigned int    reserved_1            : 4; //b15-12
    unsigned int    tot_size              : 12;//b11-0


#endif
}; 


/* 
 * DMAC LLP Descriptor object
 */
typedef struct _DMAC_LLP_    DMAC_LLP_T;
struct _DMAC_LLP_
{
    unsigned int    SrcAddr;
    
    unsigned int    DstAddr;
    
    DMAC_LLP_T     *LLP;

    DMAC_LLP_CONTROL_T    Ctrl_TotSize;    
};

typedef struct _DMAC_HARDWARE_HANDSHAKE_OBJ_    DMAC_HARDWARE_HANDSHAKE_OBJ_T;
struct _DMAC_HARDWARE_HANDSHAKE_OBJ_
{
    unsigned int    src_addr;                     //Src address
    unsigned int    dst_addr;                     //Dst address
    unsigned int    src_master;                   //0:AHB0, 1:AHB1
    unsigned int    dst_master;                   //0:AHB0, 1:AHB1
    unsigned int    dstad_ctl;                    //0:Incr, 1:Decr, 2:Fix
    unsigned int    srcad_ctl;                    //0:Incr, 1:Decr, 2:Fix
    unsigned int    src_width;                    //0:8bits, 1:16bits, 2:32bits
    unsigned int    dst_width;                    //0:8bits, 1:16bits, 2:32bits
    unsigned int    transfer_bytes;               //Byte Count to be transferred
    unsigned int    channel_id;                   //0~7 for Channel0-7 selection
    unsigned int    channel_num;                   //0~7
    unsigned int    target_select;                //target ID
    unsigned int    src_burst_size;               //number of transfer 
    unsigned int    llp_addr;                     //LLP address

    void * private_data;
    DMAC_LLP_T* llp_head;
}; 


extern void Hal_Dmac_Configure_DMA_Handshake(DMAC_HARDWARE_HANDSHAKE_OBJ_T *dmac_obj);
extern irqreturn_t str8100_dma_err_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs);
//extern int str8100_i2s_init(int sampling_rate);
extern int str8100_i2s_init(int sampling_rate,u32 i2s_sdata_width, u32 i2s_mode,
			u32 i2s_tranfer_timing_ctrl, u32 i2s_sclk_mode);
extern u32 Hal_Dmac_Get_Channel_Transfer_Unit_Number(u32 byte_size, u32 src_width);

extern u32 I2s_Gpio_SSP_Initialise(void);
extern void I2s_Gpio_SSP_Write(u16);
extern void I2s_Gpio_SSP_Switch_To_Record_Data(void);
extern void I2s_Gpio_SSP_Switcg_To_Playback_Data(void);


#endif //#ifndef __DEMO_DMA_H__


