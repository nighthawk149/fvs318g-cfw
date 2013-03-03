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

//*******************
//Options
#define DEBUG
#define I2S_FULL_DUPLEX
#define I2S_WM8772_DMAC_LLP_TEST
#define I2S_WM8772_DMAC_LLP_NUM      3
#define I2S_WM8772_BUFFER_SIZE       (48 * 1 * 2 * 100)
#define I2S_TIMING 	I2S_I2S_MODE
//#define I2S_TIMING 	I2S_RJF_MODE
//#define I2S_TIMING 	I2S_LJF_MODE
#define DATA_WIDTH 	I2S_DATA_32_BIT
//#define DATA_WIDTH 	I2S_DATA_16_BIT
#define I2S_MODE 	I2S_MASTER_MODE
//#define I2S_MODE 	I2S_SLAVE_MODE
#define I2S_SCLK_MODE	I2S_CLOCK_256S_MODE
//#define I2S_SCLK_MODE	I2S_CLOCK_CONTINUOUS_MODE
//*******************

#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/pci.h>

#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_i2s.h>
#include <asm/arch/star_demo_dma.h>

MODULE_AUTHOR("Mac Lin");
MODULE_LICENSE("Dual BSD/GPL");


//Debug = left only

static u32 sampling_rate=44100;

module_param(sampling_rate, int, 0);
MODULE_PARM_DESC(gpio, "sampling_rate");

//#define DMA_TRANSFER_MAX_BYTE    (0xfff<<(sample_size>>4)) 
//#define BUFSIZE (0xfff<<4) //twice the 32-bit DMA transfer byte size

static u8* buffer=NULL;
static u8* buffer_p=NULL;
static DMAC_LLP_T* desc=NULL;
static DMAC_LLP_T* desc_p=NULL;

static u8* lbuffer=NULL;
static u8* lbuffer_p=NULL;
static DMAC_LLP_T* lb_txd=NULL;
static DMAC_LLP_T* lb_txd_p=NULL;
static DMAC_LLP_T* lb_rxd=NULL;
static DMAC_LLP_T* lb_rxd_p=NULL;

static u8* rbuffer=NULL;
static u8* rbuffer_p=NULL;
static DMAC_LLP_T* rb_txd=NULL;
static DMAC_LLP_T* rb_txd_p=NULL;
static DMAC_LLP_T* rb_rxd=NULL;
static DMAC_LLP_T* rb_rxd_p=NULL;

static DMAC_LLP_T* i2s_left_tx_channel_dma_llp_desc=NULL;
static DMAC_LLP_T* i2s_left_rx_channel_dma_llp_desc=NULL;
static DMAC_LLP_T* i2s_right_tx_channel_dma_llp_desc=NULL;
static DMAC_LLP_T* i2s_right_rx_channel_dma_llp_desc=NULL;

static u8* i2s_left_channel_llp_buf[I2S_WM8772_DMAC_LLP_NUM];
static u8* i2s_right_channel_llp_buf[I2S_WM8772_DMAC_LLP_NUM];


static u32 i2s_err_lur=0;

static u32 debug=0;
#define DEBUG_PRINT(arg...) if(debug) printk(arg);

static u32 i2s_wm8772_dma_left_tx_channel 	= 0;
static u32 i2s_wm8772_dma_right_tx_channel 	= 1;
static u32 i2s_wm8772_dma_left_rx_channel 	= 2;
static u32 i2s_wm8772_dma_right_rx_channel 	= 3;


#if (I2S_WM8772_DMAC_LLP_NUM >= 3)
#define I2S_WM8772_DMAC_LLP_RING_TEST
#endif
#define BUFSIZE (I2S_WM8772_DMAC_LLP_NUM*I2S_WM8772_BUFFER_SIZE*4)

#define I2S_BASE_ADDR SYSPA_I2S_BASE_ADDR
#define i2s_dmac_llp_num I2S_WM8772_DMAC_LLP_NUM
//=================================================================================
//

/******************************************************************************
 *
 * FUNCTION:  I2s_Configure_WM8772
 * PURPOSE:
 *
 ******************************************************************************/
void I2s_Configure_WM8772(u32 interface_format, u32 word_length, u32 data_interface_mode)
{
    u16    reg_data;


    /*
     * The current configuration of W571C161 DAC are:
     * 1. MODE : External hardware setting control mode
     * 2. COM1 = 1, COM2 = 0 : 16-bit I2S
     * 3. COM3 = 0 : De-emphasis filter off
     * 4. I2S Level 2 (Slave Mode) : How to configure it
     */

    /*
     * Configure register R31 for reset
     */
#if 1
    reg_data = (0x1F << 9) |  /* Register address value : 0x1F */
               (0x1FF);
#else
    reg_data = (0xF << 9) |  /* Register address value : 0xF */
               (0x1FF);
#endif

    I2s_Gpio_SSP_Write(reg_data);
    
    /*
     * Configure register R2
     */ 
    reg_data = (0x2 << 9) |  /* Register address value : 0x2 */
#if 1
//               (0x5 << 5) |  /* Right Channel Output Mixer Status : right channel data */
               (0x9 << 5) |  /* Right Channel Output Mixer Status : right channel data */
                             /* Left Channel Output Mixer Status : left channel data */
#else
               (0x2 << 7) |  /* Right Channel Output Mixer Status : right channel data */
               (0x1 << 5) |  /* Left Channel Output Mixer Status : left channel data */
#endif
               (0x0 << 4) |  /* IZM : disable */
               (0x0 << 3) |  /* ATC : right channels use right attenuations */
               (0x0 << 2) |  /* PDWNALL : disable */
               (0x0 << 1) |  /* DEEMPALL : normal */
               (0x0 << 0);   /* Soft Mute Select : normal */

    I2s_Gpio_SSP_Write(reg_data);
 
    /*
     * Configure register R12
     */
    reg_data = (0xC << 9) |  /* Register address value : 0xC */
               (0x1 << 6) |  /* Disable the MUTE decoded circuit used by DZFM */
//               (0x0 << 6) |  /* Enable the MUTE decoded circuit used by DZFM */
               (0x0 << 2) |  /* Normal operation */
               (0x0 << 1) |  /* Normal operation */
               (0x0 << 0);   /* Normal operation */

    I2s_Gpio_SSP_Write(reg_data);

    
    /*
     * Configure register R9
     */
    reg_data = (0x9 << 9) |  /* Register address value : 0x9 */
               (0x0 << 8) |  /* Channel 3L/R De-emphasis Status : No de-emphasis */
               (0x0 << 7) |  /* Channel 2L/R De-emphasis Status : No de-emphasis */
               (0x0 << 6) |  /* Channel 1L/R De-emphasis Status : No de-emphasis */
               (0x1 << 5) |  /* Channel 3L/R Play Status : Mute */
               (0x1 << 4) |  /* Channel 2L/R Play Status : Mute */
               (0x0 << 3) |  /* Channel 1L/R Play Status : play */
               (0x3 << 1) |  /* DZFM[1:0] : Channel 3 */
               (0x1 << 0);   /* ZCD : disabled */

    I2s_Gpio_SSP_Write(reg_data);


    /*
     * Configure register R3 for audio data interface format
     */
    reg_data = (0x3 << 9) |  /* Register address value : 0x3 */
               (0x0 << 6) |  /* Phase : normal */
               (0x0 << 4) |  /* Input Word Length : 16-bit(0x0 << 4) ; 32-bit(0x3 <<4) */
#if 0
               (0x1 << 3) |  /* BCK Polarity : invert : for DSP mode only ????? */
#else
               (0x0 << 3) |  /* BCK Polarity : normal */
#endif

#if 0
               (0x1 << 2) |  /* LRC Polarity : invert */
#else
               (0x0 << 2) |  /* LRC Polarity : normal */
#endif          
               (0x0 << 0);   /* Interface Format : right justified mode */

    if (interface_format == I2S_RJF_MODE)
    {
        reg_data |= 0;
    }
    else if (interface_format == I2S_LJF_MODE)
    {
        reg_data |= 1;
    }
    else if (interface_format == I2S_I2S_MODE)
    {
        reg_data |= 2;
    }
    else
    {
        printk("\nI2S: Incorrect selected interface mode: DSP mode!\n");
        
        reg_data |= 3;
    }

    /*
     *  16-bit data for I2S_RJF_MODE
     */ 
    if (interface_format == I2S_RJF_MODE)
    {
        // 16-bit data
        reg_data |= (0 << 4);
    }
    else
    {
        if (word_length == I2S_DATA_16_BIT)
        {
            reg_data |= (0 << 4);
        }
        else if (word_length == I2S_DATA_32_BIT)
        {
            reg_data |= (3 << 4);
        }
    }

    I2s_Gpio_SSP_Write(reg_data);

    
    /*
     * Configure register R10
     */
    reg_data = (0xA << 9) |  /* Register address value : 0xA */
               (0x2 << 6) |  /* Frequency Ratio = MCLK/LRC : 256 */
               //(0x3 << 6) |  /* Frequency Ratio = MCLK/LRC : 384 */
               //(0x0 << 6) |  /* Frequency Ratio = MCLK/LRC : 128 */
               (0x0 << 5) |  /* Data Interface Mode : Level 2 (Slave mode) */
               (0x0 << 4) |  /* All Channel Power Down : play */
               (0x1 << 3) |  /* Channel 3 L/R Play Status : power down */
               (0x1 << 2) |  /* Channel 2 L/R Play Status : power down */
               (0x0 << 1) |  /* Channel 1 L/R Play Status : play */
#if 0
               (0x1 << 0);   /* ADC deactive */
#else
               (0x0 << 0);   /* ADC active */
#endif

    if (data_interface_mode == I2S_SLAVE_MODE)
    {
        /*
         * Equuleus's I2S is in Slave mode, so WM8772 will be in Master mode
         */
        reg_data |= (0x1 << 5);
    }

    I2s_Gpio_SSP_Write(reg_data);


    /*
     * Configure register R11?????
     */
    reg_data = (0xB << 9) |  /* Register address value : 0xB */
               (0x0 << 8);   /* 128x oversampling */
               
    I2s_Gpio_SSP_Write(reg_data);


    /*
     * Configure register R12
     */
    reg_data = (0xC << 9) |  /* Register address value : 0xC */
               (0x0 << 3);   /* Highpass filter enabled */
               
    I2s_Gpio_SSP_Write(reg_data);
    
    return;
}

//=================================================================================
//

static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8772_dma_right_tx;
static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8772_dma_right_rx;
static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8772_dma_left_tx;
static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8772_dma_left_rx;

void I2s_Configure_WM8772_DMA_Hardware_Handshake(void)
{
	u32 ii,tot_size;
    /*
     * Initialize DMAC's LLP descriptors
     */
	i2s_left_tx_channel_dma_llp_desc=lb_txd;
	i2s_left_rx_channel_dma_llp_desc=lb_rxd;
	i2s_right_tx_channel_dma_llp_desc=rb_txd;
	i2s_right_rx_channel_dma_llp_desc=rb_rxd;
	
    for (ii = 0; ii < I2S_WM8772_DMAC_LLP_NUM; ii++)
    {
        memset ((void *)&i2s_left_tx_channel_dma_llp_desc[ii], 0x0, sizeof (i2s_left_tx_channel_dma_llp_desc[ii]));
        memset ((void *)&i2s_left_rx_channel_dma_llp_desc[ii], 0x0, sizeof (i2s_left_rx_channel_dma_llp_desc[ii]));
        
        memset ((void *)&i2s_right_tx_channel_dma_llp_desc[ii], 0x0, sizeof (i2s_right_tx_channel_dma_llp_desc[ii]));
        memset ((void *)&i2s_right_rx_channel_dma_llp_desc[ii], 0x0, sizeof (i2s_right_rx_channel_dma_llp_desc[ii]));
    }

//1. *****************************************************************************
    /*
     * Configure DMA's channel setting for I2S's Left Channel Tx
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */
    for (ii = 0; ii < I2S_WM8772_DMAC_LLP_NUM; ii++)
    {
        i2s_left_tx_channel_dma_llp_desc[ii].SrcAddr = i2s_left_channel_llp_buf[ii];
        
        i2s_left_tx_channel_dma_llp_desc[ii].DstAddr = (I2S_BASE_ADDR + 0xC8);

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_left_tx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&lb_txd_p[ii+1] : (DMAC_LLP_T *)&lb_txd_p[1];
#else
        i2s_left_tx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&lb_txd_p[ii+1] : (DMAC_LLP_T *)(0);
#endif

        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);
        
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.tot_size = tot_size;  /* TOT_SIZE */
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_sel = DMAC_CH_DST_SEL_M0;  /* DST_SEL */
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_sel = DMAC_CH_SRC_SEL_M1;  /* SRC_SEL */
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;  /* DSTAD_CTL */
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;  /* SRCAD_CTL */
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_width = DMAC_CH_DST_WIDTH_32_BITS;  /* DST_WIDTH */
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_width = DMAC_CH_SRC_WIDTH_32_BITS;  /* SRC_WIDTH */       

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = (DMAC_CH_TC_MASK_ENABLE);  /* TC_MASK */
#else
        i2s_left_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = ((ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_CH_TC_MASK_ENABLE) : (DMAC_CH_TC_MASK_DISABLE));  /* TC_MASK */
#endif
    }

    i2s_wm8772_dma_left_tx.channel_id = DMAC_CH_ID(i2s_wm8772_dma_left_tx_channel);
    i2s_wm8772_dma_left_tx.target_select = DMAC_HW_HAND_SHAKE_I2S_TX_LEFT_ID;

    i2s_wm8772_dma_left_tx.src_addr = i2s_left_tx_channel_dma_llp_desc[0].SrcAddr;
    i2s_wm8772_dma_left_tx.dst_addr = i2s_left_tx_channel_dma_llp_desc[0].DstAddr;

    i2s_wm8772_dma_left_tx.src_master = DMAC_CH_SRC_SEL_M1;
    i2s_wm8772_dma_left_tx.dst_master = DMAC_CH_DST_SEL_M0;

    i2s_wm8772_dma_left_tx.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;
    i2s_wm8772_dma_left_tx.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;

    i2s_wm8772_dma_left_tx.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    i2s_wm8772_dma_left_tx.dst_width = DMAC_CH_DST_WIDTH_32_BITS;


    // Note here the total number of bytes is specified!!
    i2s_wm8772_dma_left_tx.transfer_bytes = I2S_WM8772_BUFFER_SIZE * 4;

    i2s_wm8772_dma_left_tx.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;   

    i2s_wm8772_dma_left_tx.llp_addr = (i2s_dmac_llp_num == 1) ? 0 : (u32)i2s_left_tx_channel_dma_llp_desc[0].LLP;

    Hal_Dmac_Configure_DMA_Handshake(&i2s_wm8772_dma_left_tx);
//2. *****************************************************************************
    /*
     * Configure DMA's channel setting for I2S's Right Channel Tx
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */
    for (ii = 0; ii < I2S_WM8772_DMAC_LLP_NUM; ii++)
    {
        i2s_right_tx_channel_dma_llp_desc[ii].SrcAddr = i2s_right_channel_llp_buf[ii];
        
        i2s_right_tx_channel_dma_llp_desc[ii].DstAddr = (I2S_BASE_ADDR + 0xC4);

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_right_tx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&rb_txd_p[ii+1] : (DMAC_LLP_T *)&rb_txd_p[1];
#else
        i2s_right_tx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&rb_txd_p[ii + 1] : (DMAC_LLP_T *)(0);
#endif

        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);

        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.tot_size = tot_size;  /* TOT_SIZE */
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_sel = DMAC_CH_DST_SEL_M0;  /* DST_SEL */
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_sel = DMAC_CH_SRC_SEL_M1;  /* SRC_SEL */
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;  /* DSTAD_CTL */
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;  /* SRCAD_CTL */
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_width = DMAC_CH_DST_WIDTH_32_BITS;  /* DST_WIDTH */
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_width = DMAC_CH_SRC_WIDTH_32_BITS;  /* SRC_WIDTH */

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = (DMAC_CH_TC_MASK_ENABLE);  /* TC_MASK */
#else
        i2s_right_tx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = ((ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_CH_TC_MASK_ENABLE) : (DMAC_CH_TC_MASK_DISABLE));  /* TC_MASK */
#endif
    }


    i2s_wm8772_dma_right_tx.channel_id = DMAC_CH_ID(i2s_wm8772_dma_right_tx_channel);
    i2s_wm8772_dma_right_tx.target_select = DMAC_HW_HAND_SHAKE_I2S_TX_RIGHT_ID;

    i2s_wm8772_dma_right_tx.src_addr = i2s_right_tx_channel_dma_llp_desc[0].SrcAddr;
    i2s_wm8772_dma_right_tx.dst_addr = i2s_right_tx_channel_dma_llp_desc[0].DstAddr;

    i2s_wm8772_dma_right_tx.src_master = DMAC_CH_SRC_SEL_M1;
    i2s_wm8772_dma_right_tx.dst_master = DMAC_CH_DST_SEL_M0;

    i2s_wm8772_dma_right_tx.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;
    i2s_wm8772_dma_right_tx.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;

    i2s_wm8772_dma_right_tx.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    i2s_wm8772_dma_right_tx.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    // Note here the total number of bytes is specified!!
    i2s_wm8772_dma_right_tx.transfer_bytes = I2S_WM8772_BUFFER_SIZE * 4;

    i2s_wm8772_dma_right_tx.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;

    i2s_wm8772_dma_right_tx.llp_addr = (i2s_dmac_llp_num == 1) ? 0 : (u32)i2s_right_tx_channel_dma_llp_desc[0].LLP;

    Hal_Dmac_Configure_DMA_Handshake(&i2s_wm8772_dma_right_tx);


//3. *****************************************************************************
    /*
     * Configure DMA's channel setting for I2S's Left Channel Rx
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */
    for (ii = 0; ii < I2S_WM8772_DMAC_LLP_NUM; ii++)
    {
        i2s_left_rx_channel_dma_llp_desc[ii].SrcAddr = (I2S_BASE_ADDR + 0xD0);
        
        i2s_left_rx_channel_dma_llp_desc[ii].DstAddr = i2s_left_channel_llp_buf[ii];

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_left_rx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&lb_rxd_p[ii + 1] : (DMAC_LLP_T *)&lb_rxd_p[1];
#else
        i2s_left_rx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&lb_rxd_p[ii + 1] : (DMAC_LLP_T *)(0);
#endif

        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);

        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.tot_size = tot_size;  /* TOT_SIZE */
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_sel = DMAC_CH_DST_SEL_M1;  /* DST_SEL */
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_sel = DMAC_CH_SRC_SEL_M0;  /* SRC_SEL */
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.dstad_ctl = DMAC_CH_DSTAD_CTL_INC;  /* DSTAD_CTL */
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.srcad_ctl = DMAC_CH_SRCAD_CTL_FIX;  /* SRCAD_CTL */
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_width = DMAC_CH_DST_WIDTH_32_BITS;  /* DST_WIDTH */
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_width = DMAC_CH_SRC_WIDTH_32_BITS;  /* SRC_WIDTH */

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = (DMAC_CH_TC_MASK_ENABLE);  /* TC_MASK */
#else
        i2s_left_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = ((ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_CH_TC_MASK_ENABLE) : (DMAC_CH_TC_MASK_DISABLE));  /* TC_MASK */
#endif
    }

    i2s_wm8772_dma_left_rx.channel_id = DMAC_CH_ID(i2s_wm8772_dma_left_rx_channel);
    i2s_wm8772_dma_left_rx.target_select = DMAC_HW_HAND_SHAKE_I2S_RX_LEFT_ID;

    i2s_wm8772_dma_left_rx.src_addr = i2s_left_rx_channel_dma_llp_desc[0].SrcAddr;
    i2s_wm8772_dma_left_rx.dst_addr = i2s_left_rx_channel_dma_llp_desc[0].DstAddr;

    i2s_wm8772_dma_left_rx.src_master = DMAC_CH_SRC_SEL_M0;
    i2s_wm8772_dma_left_rx.dst_master = DMAC_CH_DST_SEL_M1;

    i2s_wm8772_dma_left_rx.srcad_ctl = DMAC_CH_SRCAD_CTL_FIX;
    i2s_wm8772_dma_left_rx.dstad_ctl = DMAC_CH_DSTAD_CTL_INC;

    i2s_wm8772_dma_left_rx.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    i2s_wm8772_dma_left_rx.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    // Note here the total number of bytes is specified!!
    i2s_wm8772_dma_left_rx.transfer_bytes = I2S_WM8772_BUFFER_SIZE * 4;

    i2s_wm8772_dma_left_rx.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;   

    i2s_wm8772_dma_left_rx.llp_addr = (i2s_dmac_llp_num == 1) ? 0 : (u32)i2s_left_rx_channel_dma_llp_desc[0].LLP;

    Hal_Dmac_Configure_DMA_Handshake(&i2s_wm8772_dma_left_rx);

//4. *****************************************************************************
    /*
     * Configure DMA's channel setting for I2S's Right Channel Rx
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */
    for (ii = 0; ii < I2S_WM8772_DMAC_LLP_NUM; ii++)
    {
        i2s_right_rx_channel_dma_llp_desc[ii].SrcAddr = (I2S_BASE_ADDR + 0xCC);
        
        i2s_right_rx_channel_dma_llp_desc[ii].DstAddr = i2s_right_channel_llp_buf[ii];

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_right_rx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&rb_rxd_p[ii + 1] : (DMAC_LLP_T *)&rb_rxd_p[1];
#else
        i2s_right_rx_channel_dma_llp_desc[ii].LLP = (ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_LLP_T *)&rb_rxd_p[ii + 1] : (DMAC_LLP_T *)(0);
#endif

        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);

        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.tot_size = tot_size;  /* TOT_SIZE */
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_sel = DMAC_CH_DST_SEL_M1;  /* DST_SEL */
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_sel = DMAC_CH_SRC_SEL_M0;  /* SRC_SEL */
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.dstad_ctl = DMAC_CH_DSTAD_CTL_INC;  /* DSTAD_CTL */
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.srcad_ctl = DMAC_CH_SRCAD_CTL_FIX;  /* SRCAD_CTL */
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.dst_width = DMAC_CH_DST_WIDTH_32_BITS;  /* DST_WIDTH */
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.src_width = DMAC_CH_SRC_WIDTH_32_BITS;  /* SRC_WIDTH */

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = (DMAC_CH_TC_MASK_ENABLE);  /* TC_MASK */
#else
        i2s_right_rx_channel_dma_llp_desc[ii].Ctrl_TotSize.tc_status_mask = ((ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_CH_TC_MASK_ENABLE) : (DMAC_CH_TC_MASK_DISABLE));  /* TC_MASK */
#endif
    }

    i2s_wm8772_dma_right_rx.channel_id = DMAC_CH_ID(i2s_wm8772_dma_right_rx_channel);
    i2s_wm8772_dma_right_rx.target_select = DMAC_HW_HAND_SHAKE_I2S_RX_RIGHT_ID;

    i2s_wm8772_dma_right_rx.src_addr = i2s_right_rx_channel_dma_llp_desc[0].SrcAddr;
    i2s_wm8772_dma_right_rx.dst_addr = i2s_right_rx_channel_dma_llp_desc[0].DstAddr;

    i2s_wm8772_dma_right_rx.src_master = DMAC_CH_SRC_SEL_M0;
    i2s_wm8772_dma_right_rx.dst_master = DMAC_CH_DST_SEL_M1;

    i2s_wm8772_dma_right_rx.srcad_ctl = DMAC_CH_SRCAD_CTL_FIX;
    i2s_wm8772_dma_right_rx.dstad_ctl = DMAC_CH_DSTAD_CTL_INC;

    i2s_wm8772_dma_right_rx.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    i2s_wm8772_dma_right_rx.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    // Note here the total number of bytes is specified!!
    i2s_wm8772_dma_right_rx.transfer_bytes = I2S_WM8772_BUFFER_SIZE * 4;

    i2s_wm8772_dma_right_rx.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;

    i2s_wm8772_dma_right_rx.llp_addr = (i2s_dmac_llp_num == 1) ? 0 : (u32)i2s_right_rx_channel_dma_llp_desc[0].LLP;

    Hal_Dmac_Configure_DMA_Handshake(&i2s_wm8772_dma_right_rx);
//*****************************************************************************
    return;
}


//=================================================================================
//

extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);

static irqreturn_t str8100_dma_tc_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	u32 dma_tc_status,tot_size;
	u32 len;
//printk("%s: this_irq=%d\n",__FUNCTION__,this_irq);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	//todo:

    HAL_DMAC_READ_TERMINAL_COUNT_INTERRUPT_STATUS(dma_tc_status);
printk("%s: this_irq=%d, dma_tc_status=%.8x\n",__FUNCTION__,this_irq,dma_tc_status);

#ifdef I2S_WM8772_DMAC_LLP_RING_TEST
	u32 i;
    /*
     * For LLP ring test, the TC interrupt shoule not happen!!
     */
	for(i=0;i<8;i++)
	if (dma_tc_status & DMAC_CH_ID(i)){                      
		HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i));
		printk("%s: channel %d: Error!! there should be no tc irq happened!!\n",__FUNCTION__,i);
	}

#else
    /*
     * For this case, it's recommended to set I2S_WM8772_DMAC_LLP_NUM = 1
     */
    /*
     * For DMA's Tx for I2S Left Channel
     */
    if (dma_tc_status & DMAC_CH_ID(i2s_wm8772_dma_left_tx_channel))
    {                      
        HAL_DMAC_DISABLE_CHANNEL(i2s_wm8772_dma_left_tx_channel);
        
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i2s_wm8772_dma_left_tx_channel));

        /*
         * Re-initialize DMA's channel for Left_Tx
         */
        DMAC_CH_SRC_ADDR(i2s_wm8772_dma_left_tx_channel) = i2s_left_tx_channel_dma_llp_desc[0].SrcAddr;

        DMAC_CH_DST_ADDR(i2s_wm8772_dma_left_tx_channel) = i2s_left_tx_channel_dma_llp_desc[0].DstAddr;

        /*
         * Note this macro DMAC_CH_SIZE is to configure TOT_SIZE field which is the total transfer
         * number of source transfer width!
         */        
        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);

        DMAC_CH_SIZE(i2s_wm8772_dma_left_tx_channel) = tot_size & 0x0FFF;

        HAL_DMAC_ENABLE_CHANNEL(i2s_wm8772_dma_left_tx_channel);
    }


    /*
     * For DMA's Tx for I2S Right Channel
     */
    if (dma_tc_status & DMAC_CH_ID(i2s_wm8772_dma_right_tx_channel))
    {                      
        HAL_DMAC_DISABLE_CHANNEL(i2s_wm8772_dma_right_tx_channel);
        
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i2s_wm8772_dma_right_tx_channel));

        /*
         * Re-initialize DMA's channel for Right_Tx
         */
        DMAC_CH_SRC_ADDR(i2s_wm8772_dma_right_tx_channel) = i2s_right_tx_channel_dma_llp_desc[0].SrcAddr;

        DMAC_CH_DST_ADDR(i2s_wm8772_dma_right_tx_channel) = i2s_right_tx_channel_dma_llp_desc[0].DstAddr;

        /*
         * Note this macro DMAC_CH_SIZE is to configure TOT_SIZE field which is the total transfer
         * number of source transfer width!
         */        
        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);

        DMAC_CH_SIZE(i2s_wm8772_dma_right_tx_channel) = tot_size & 0x0FFF;

        HAL_DMAC_ENABLE_CHANNEL(i2s_wm8772_dma_right_tx_channel);
    }


    /*
     * For DMA's Rx for I2S Left Channel
     */
    if (dma_tc_status & DMAC_CH_ID(i2s_wm8772_dma_left_rx_channel))
    {                      
        HAL_DMAC_DISABLE_CHANNEL(i2s_wm8772_dma_left_rx_channel);
        
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i2s_wm8772_dma_left_rx_channel));

        /*
         * Re-initialize DMA's channel for Left_Rx
         */
        DMAC_CH_SRC_ADDR(i2s_wm8772_dma_left_rx_channel) = i2s_left_rx_channel_dma_llp_desc[0].SrcAddr;

        DMAC_CH_DST_ADDR(i2s_wm8772_dma_left_rx_channel) = i2s_left_rx_channel_dma_llp_desc[0].DstAddr;

        /*
         * Note this macro DMAC_CH_SIZE is to configure TOT_SIZE field which is the total transfer
         * number of source transfer width!
         */        
        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);

        DMAC_CH_SIZE(i2s_wm8772_dma_left_rx_channel) = tot_size & 0x0FFF;

        HAL_DMAC_ENABLE_CHANNEL(i2s_wm8772_dma_left_rx_channel);
    }


    /*
     * For DMA's Rx for I2S Right Channel
     */
    if (dma_tc_status & DMAC_CH_ID(i2s_wm8772_dma_right_rx_channel))
    {                      
        HAL_DMAC_DISABLE_CHANNEL(i2s_wm8772_dma_right_rx_channel);
        
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i2s_wm8772_dma_right_rx_channel));

        /*
         * Re-initialize DMA's channel for Right_Rx
         */
        DMAC_CH_SRC_ADDR(i2s_wm8772_dma_right_rx_channel) = i2s_right_rx_channel_dma_llp_desc[0].SrcAddr;

        DMAC_CH_DST_ADDR(i2s_wm8772_dma_right_rx_channel) = i2s_right_rx_channel_dma_llp_desc[0].DstAddr;

        /*
         * Note this macro DMAC_CH_SIZE is to configure TOT_SIZE field which is the total transfer
         * number of source transfer width!
         */        
        tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(I2S_WM8772_BUFFER_SIZE * 4, DMAC_CH_SRC_WIDTH_32_BITS);

        DMAC_CH_SIZE(i2s_wm8772_dma_right_rx_channel) = tot_size & 0x0FFF;

        HAL_DMAC_ENABLE_CHANNEL(i2s_wm8772_dma_right_rx_channel);
    }
#endif



	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}


static irqreturn_t str8100_i2s_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	u32 interrupt_status;
	DEBUG_PRINT("%s: this_irq=%d, I2S_INTERRUPT_STATUS_REG=0x%.8x\n",__FUNCTION__,this_irq,I2S_INTERRUPT_STATUS_REG);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	//todo:
	interrupt_status = I2S_INTERRUPT_STATUS_REG;   

		I2S_RIGHT_TRANSMIT_DATA_REG=0;


	if ((interrupt_status & (I2S_RXBF_R_FULL_FLAG | I2S_RXBF_L_FULL_FLAG | I2S_TXBF_R_EMPTY_FLAG | I2S_TXBF_L_EMPTY_FLAG))){
		printk("%s: Error! wrong i2s empty/full flag\n",__FUNCTION__);
	
	}

	if ((interrupt_status & (I2S_RXBF_R_OR_FLAG | I2S_RXBF_L_OR_FLAG |I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG))){
		// Clear I2S interrupt status
		i2s_err_lur++;
		if(i2s_err_lur>10)
			I2S_INTERRUPT_ENABLE_REG &= ~(I2S_TXBF_L_UR_FLAG);
//			HAL_I2S_DISABLE_I2S();
		
		printk("%s: Left Channel Tx Underrun!, interrupt_status=%.8x,i2s_err_lur=%d\n",__FUNCTION__,interrupt_status,i2s_err_lur);
	}
	I2S_INTERRUPT_STATUS_REG &= 0xf0;

	
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}



//=================================================================================

static void i2s_exit_module(void){
	printk("%s:\n",__FUNCTION__);
	remove_proc_entry("str8100/i2s", NULL);
	free_irq(INTC_I2S_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_TC_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_ERROR_BIT_INDEX, NULL);
	if(buffer) {
		pci_free_consistent(NULL, BUFSIZE*2, buffer, buffer_p);
		buffer=buffer_p=NULL;
		lbuffer=lbuffer_p=rbuffer=rbuffer_p=NULL;
	}
	if(desc) {
		pci_free_consistent(NULL, I2S_WM8772_DMAC_LLP_NUM*sizeof(DMAC_LLP_T)*4 , desc, desc_p);
		desc=desc_p=NULL;
		lb_txd=lb_txd_p=lb_rxd=lb_rxd_p=NULL;
		rb_txd=rb_txd_p=rb_rxd=rb_rxd_p=NULL;
	}
}

static int __init i2s_init_module(void)
{
	
	u32 ret;
	u32 ii;
	
	printk("%s:\n",__FUNCTION__);

	buffer = pci_alloc_consistent(NULL, BUFSIZE*2, &buffer_p);
	if(!buffer){
		printk("%s: alloc buffer failed.\n",__FUNCTION__);
		goto exit1;
	}
	lbuffer=buffer;
	lbuffer_p=buffer_p;
	rbuffer=lbuffer+BUFSIZE;
	rbuffer_p=lbuffer_p+BUFSIZE;
	
	desc = pci_alloc_consistent(NULL, I2S_WM8772_DMAC_LLP_NUM*sizeof(DMAC_LLP_T)*4 , &desc_p);
	if(!desc){
		printk("%s: alloc buffer for desc failed.\n",__FUNCTION__);
		goto exit1;
	}
	lb_txd=desc;
	lb_txd_p=desc_p;
	lb_rxd=desc+I2S_WM8772_DMAC_LLP_NUM;
	lb_rxd_p=desc_p+I2S_WM8772_DMAC_LLP_NUM;

	rb_txd=desc+I2S_WM8772_DMAC_LLP_NUM*2;
	rb_txd_p=desc_p+I2S_WM8772_DMAC_LLP_NUM*2;
	rb_rxd=desc+I2S_WM8772_DMAC_LLP_NUM*3;
	rb_rxd_p=desc_p+I2S_WM8772_DMAC_LLP_NUM*3;
	

	str8100_set_interrupt_trigger (INTC_I2S_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);
	if ((ret=request_irq(INTC_I2S_BIT_INDEX, str8100_i2s_irq_handler, 0, "i2s", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_I2S_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}

	str8100_set_interrupt_trigger (INTC_GDMAC_TC_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_GDMAC_TC_BIT_INDEX, str8100_dma_tc_irq_handler, 0, "dma tc", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_GDMAC_TC_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}
	str8100_set_interrupt_trigger (INTC_GDMAC_ERROR_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_GDMAC_ERROR_BIT_INDEX, str8100_dma_err_irq_handler, 0, "dma error", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_GDMAC_ERROR_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}

	/*
	 * Initialize GPIO pins for COM1/COM2/COM3
	 */
	I2s_Gpio_SSP_Initialise();

	str8100_i2s_init(sampling_rate,DATA_WIDTH, I2S_MODE, I2S_TIMING, I2S_SCLK_MODE);

	/*
	 * Configure Wolfson's WM8772 ADC/DAC
	 */
	I2s_Configure_WM8772(I2S_TIMING, DATA_WIDTH, I2S_MODE); 
#ifdef I2S_FULL_DUPLEX
	/*
	* To support I2S full duplex mode, GPIOA[3] will be used as I2S_DR pin.
	* It means GPIOA[3] should be as input pin.
	*/
	MISC_GPIOA_PIN_ENABLE_REG &= ~(0x1 << 3);
    
	GPIOA_DIRECTION_REG &= ~(0x1 << 3);
#endif

	I2S_RIGHT_TRANSMIT_DATA_REG=0;
	I2S_LEFT_TRANSMIT_DATA_REG=0;
	ii=I2S_RIGHT_RECEIVE_DATA_REG;
	ii=I2S_LEFT_RECEIVE_DATA_REG;

	/*
	 * Initialize DMAC related configuration
	 */    

	i2s_left_channel_llp_buf[0] = lbuffer_p;
	i2s_right_channel_llp_buf[0] = rbuffer_p;
	memset ((void *)&lbuffer[0], 0x0, I2S_WM8772_BUFFER_SIZE * 4);
	memset ((void *)&rbuffer[0], 0x0, I2S_WM8772_BUFFER_SIZE * 4);
	for (ii = 1; ii < I2S_WM8772_DMAC_LLP_NUM; ii++)
	{
		i2s_left_channel_llp_buf[ii] = i2s_left_channel_llp_buf[ii - 1] + (I2S_WM8772_BUFFER_SIZE * 4);
		i2s_right_channel_llp_buf[ii] = i2s_right_channel_llp_buf[ii - 1] + (I2S_WM8772_BUFFER_SIZE * 4);
		memset ((void *)&lbuffer[ii*I2S_WM8772_BUFFER_SIZE*4], 0x0, I2S_WM8772_BUFFER_SIZE * 4);
		memset ((void *)&rbuffer[ii*I2S_WM8772_BUFFER_SIZE*4], 0x0, I2S_WM8772_BUFFER_SIZE * 4);
	}

	i2s_wm8772_dma_left_tx_channel = 0;
    
	i2s_wm8772_dma_right_tx_channel = 1;
    
	i2s_wm8772_dma_left_rx_channel = 2;
    
	i2s_wm8772_dma_right_rx_channel = 3;


	I2s_Configure_WM8772_DMA_Hardware_Handshake();


	/*
	 * Enable I2S's interrupt sources
	 * Note for DMA hardware handshake, we only need to enable Left/Right Channel's 
	 * Transmit Buffer Underrun.
	 */
	I2S_INTERRUPT_ENABLE_REG |= (I2S_RXBF_R_OR_FLAG | I2S_RXBF_L_OR_FLAG | I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG);

	// Enable CPU interrupt
	local_irq_enable();
	
	/*
	 * Note DMA must be enabled first before I2S is enabled
	 */
	HAL_DMAC_ENABLE();
	HAL_I2S_ENABLE_I2S();
	printk("%s: exit successfully\n",__FUNCTION__);
	return 0;

exit1:
	HAL_I2S_DISABLE_I2S();
	i2s_exit_module();
	printk("%s: exit errornous\n",__FUNCTION__);
	return -EBUSY;
}

module_init(i2s_init_module);
module_exit(i2s_exit_module);
