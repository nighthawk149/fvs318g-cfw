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

//Options 
//#define DEBUG_PRINT	
//=================================================================================
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

#include <asm/arch/star_spi.h>
#include <asm/arch/star_pcm.h>
#include <asm/arch/star_dmac.h>
#include <asm/arch/star_demo_dma.h>

#define u_int32 u32
#define u_int16 u16
#define u_int8 u8

#define Sys_Interrupt_Disable_Save_Flags(_p) local_irq_save(*(_p))
//#define Sys_Interrupt_Disable_Save_Flags local_irq_save
#define Sys_Interrupt_Restore_Flags local_irq_restore

#define Hal_Timer_Timer3_Delay(_p) mdelay(_p/1000)

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
//#define DEBUG_PRINT(arg...) if(debug) printk(arg);
#define DEBUG_PRINT printk
#else
#define DEBUG_PRINT
#endif

/*
 * public variable declarations
 */
PCM_OBJECT_T                          pcm_object;
PCM_CHANNEL_OBJECT_T                  pcm_channel_object;

static DMAC_HARDWARE_HANDSHAKE_OBJ_T  pcm_tx_data0_dma_legerity;
static DMAC_HARDWARE_HANDSHAKE_OBJ_T  pcm_rx_data0_dma_legerity;
static DMAC_HARDWARE_HANDSHAKE_OBJ_T  pcm_tx_data1_dma_legerity;
static DMAC_HARDWARE_HANDSHAKE_OBJ_T  pcm_rx_data1_dma_legerity;

static u_int32                        pcm_tx_data0_dma_channel_num = 0;
static u_int32                        pcm_tx_data1_dma_channel_num = 1;
static u_int32                        pcm_rx_data0_dma_channel_num = 2;
static u_int32                        pcm_rx_data1_dma_channel_num = 3;

//#define           BUFFER_SIZE    (1 * 256)
//#define           BUFFER_SIZE    (1 * 128)
//#define           BUFFER_SIZE    (1 * PCM_LOG_PERIOD_SEC * 8000)
#define           BUFFER_SIZE    0xfff

u_int32    *buffer0;
u_int32    *buffer1;
u_int32    *buffer2;
u_int32    *buffer3;

u_int32    *buffer_p0;
u_int32    *buffer_p1;
u_int32    *buffer_p2;
u_int32    *buffer_p3;

u_int32    *pcm_rx0_buffer_le88221;
u_int32    *pcm_tx0_buffer_le88221;
u_int32    *pcm_rx1_buffer_le88221;
u_int32    *pcm_tx1_buffer_le88221;

u_int32    *pcm_rx0_buffer_le88221_p;
u_int32    *pcm_tx0_buffer_le88221_p;
u_int32    *pcm_rx1_buffer_le88221_p;
u_int32    *pcm_tx1_buffer_le88221_p;


#define HAL_DMAC_WRITE_CHANNEL0_DESTINATION_ADDRESS(_p) DMAC_CH_DST_ADDR_REG(0) = (_p)
#define HAL_DMAC_WRITE_CHANNEL1_DESTINATION_ADDRESS(_p) DMAC_CH_DST_ADDR_REG(1) = (_p)
#define HAL_DMAC_WRITE_CHANNEL2_DESTINATION_ADDRESS(_p) DMAC_CH_DST_ADDR_REG(2) = (_p)
#define HAL_DMAC_WRITE_CHANNEL3_DESTINATION_ADDRESS(_p) DMAC_CH_DST_ADDR_REG(3) = (_p)

#define HAL_DMAC_WRITE_CHANNEL0_SOURCE_ADDRESS(_p) DMAC_CH_SRC_ADDR_REG(0) = (_p)
#define HAL_DMAC_WRITE_CHANNEL1_SOURCE_ADDRESS(_p) DMAC_CH_SRC_ADDR_REG(1) = (_p)
#define HAL_DMAC_WRITE_CHANNEL2_SOURCE_ADDRESS(_p) DMAC_CH_SRC_ADDR_REG(2) = (_p)
#define HAL_DMAC_WRITE_CHANNEL3_SOURCE_ADDRESS(_p) DMAC_CH_SRC_ADDR_REG(3) = (_p)

#define HAL_DMAC_WRITE_CHANNEL0_TRANSFER_SIZE(_p) DMAC_CH_SIZE_REG(0) = (_p)
#define HAL_DMAC_WRITE_CHANNEL1_TRANSFER_SIZE(_p) DMAC_CH_SIZE_REG(1) = (_p)
#define HAL_DMAC_WRITE_CHANNEL2_TRANSFER_SIZE(_p) DMAC_CH_SIZE_REG(2) = (_p)
#define HAL_DMAC_WRITE_CHANNEL3_TRANSFER_SIZE(_p) DMAC_CH_SIZE_REG(3) = (_p)
		
#define HAL_DMAC_ENABLE_CHANNEL0(_p) HAL_DMAC_ENABLE_CHANNEL(0)
#define HAL_DMAC_ENABLE_CHANNEL1(_p) HAL_DMAC_ENABLE_CHANNEL(1)
#define HAL_DMAC_ENABLE_CHANNEL2(_p) HAL_DMAC_ENABLE_CHANNEL(2)
#define HAL_DMAC_ENABLE_CHANNEL3(_p) HAL_DMAC_ENABLE_CHANNEL(3)
        
u_int32 rxbuf_full=0;
u_int32 txbuf_empty=0;
u_int32 rxbuf_overrun=0;
u_int32 txbuf_underrun=0;

u_int32 pcm_dma_ch0_tc_count = 0;
u_int32 pcm_dma_ch2_tc_count = 0;
u_int32 pcm_dma_ch1_tc_count = 0;
u_int32 pcm_dma_ch3_tc_count = 0;

/*
 * For Legerity's Le88221
 */
#define        CH0_TX_Le88221_DELAY       (0)
#define        CH0_RX_Le88221_DELAY       (0)

#define        CH1_TX_Le88221_DELAY       (8)
#define        CH1_RX_Le88221_DELAY       (8)

#define        CH2_TX_Le88221_DELAY       (32)
#define        CH2_RX_Le88221_DELAY       (32)

#define        CH3_TX_Le88221_DELAY       (48)
#define        CH3_RX_Le88221_DELAY       (48)




static struct proc_dir_entry *star_pcm_proc_entry=NULL;
static u8* txbuffer;
static u8* txbuffer_p;
static u32 txlen=0;
static u32 txpos=0;

/******************************************************************************
 *
 * FUNCTION:  Hal_Pcm_Initialize
 * PURPOSE:
 *
 ******************************************************************************/
void Hal_Pcm_Initialize(PCM_OBJECT_T *pPcm_Object)
{
    u_int32 volatile    tx_data0 = 0;
    u_int32 volatile    rx_data0 = 0;


    // Enable PCM pins
    HAL_MISC_ENABLE_PCM_PINS();

    // Enable PCM clock
    HAL_PWRMGT_ENABLE_PCM_CLOCK(); 

/*    if (p2s_reset_flag == 0)
    {
        Hal_Pwrmgt_Software_Reset(PWRMGT_P2S_SOFTWARE_RESET_BIT_INDEX);
        
        p2s_reset_flag = 1;
    }
*/

    /*
     * Note PCM is NOT enabled after this function is invoked!!
     */
    PCM_CONFIGURATION_0_REG = pPcm_Object->config_0;
    PCM_CONFIGURATION_1_REG = pPcm_Object->config_1;
    PCM_CHANNEL_0_CONFIG_REG = pPcm_Object->channel_0_config;
    PCM_CHANNEL_1_CONFIG_REG = pPcm_Object->channel_1_config;
    PCM_CHANNEL_2_CONFIG_REG = pPcm_Object->channel_2_config;
    PCM_CHANNEL_3_CONFIG_REG = pPcm_Object->channel_3_config;
    PCM_INTERRUPT_ENABLE_REG = pPcm_Object->interrupt_config;

/*
    if (pPcm_Object->interrupt_config)
    {
        Hal_Intc_Register_Interrupt(&pPcm_Object->intc_obj);	
    }
*/
    rx_data0 = PCM_RX_DATA_63_32_REG;
    rx_data0 = PCM_RX_DATA_31_0_REG;

    // Clear spurious interrupt sources
    PCM_INTERRUPT_STATUS_REG = PCM_RXBUF_OVERRUN_FG | PCM_TXBUF_UNDERRUN_FG;

    // Disable PCM
    HAL_PCM_DISABLE_PCM();
}


/******************************************************************************
 *
 * FUNCTION:  Hal_Pcm_Is_Transmit_Buffer_Empty
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 Hal_Pcm_Is_Transmit_Buffer_Empty(void)
{
    /*
     * Return value :
     *    1 : PCM Tx Transmit Buffer Empty
     *    0 : PCM Tx Transmit Buffer Not Empty
     */    
    return ((PCM_INTERRUPT_STATUS_REG & PCM_TXBUF_EMPTY_FG) ? 1 : 0);
}


/******************************************************************************
 *
 * FUNCTION:  Hal_Pcm_Is_Receive_Buffer_Full
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 Hal_Pcm_Is_Receive_Buffer_Full(void)
{
    /*
     * Return value :
     *    1 : PCM Rx Receive Buffer Full
     *    0 : PCM Rx Receive Buffer Not Full
     */    
    return ((PCM_INTERRUPT_STATUS_REG & PCM_RXBUF_FULL_FG) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Pcm_Configure_DMA_Hardware_Handshake_For_Legerity
 * PURPOSE:
 *
 ******************************************************************************/
void Pcm_Configure_DMA_Hardware_Handshake_For_Legerity(void)
{

	DEBUG_PRINT("%s: ==>\n",__FUNCTION__);
#if 1
    /*
     * Configure DMA's channel setting for PCM Transmit Data[31:0] Register
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */

    pcm_tx_data0_dma_legerity.channel_id = DMAC_CH_ID(pcm_tx_data0_dma_channel_num);//0    
    pcm_tx_data0_dma_legerity.target_select = DMAC_HW_HAND_SHAKE_PCM_TX0_ID;//0

    pcm_tx_data0_dma_legerity.src_addr = (u_int32)pcm_tx0_buffer_le88221_p;//SCR        
    pcm_tx_data0_dma_legerity.dst_addr = (SYSPA_PCM_BASE_ADDR + 0x98);//DST

    pcm_tx_data0_dma_legerity.src_master = DMAC_CH_SRC_SEL_M1;//1
    pcm_tx_data0_dma_legerity.dst_master = DMAC_CH_DST_SEL_M0;//0

    pcm_tx_data0_dma_legerity.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;//0
    pcm_tx_data0_dma_legerity.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;//2

    pcm_tx_data0_dma_legerity.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    pcm_tx_data0_dma_legerity.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    // Note here the total number of bytes is specified!!
    pcm_tx_data0_dma_legerity.transfer_bytes = BUFFER_SIZE * 4;

    pcm_tx_data0_dma_legerity.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;//0   

    // Note this DMA's channel will be enabled when the following function is invoked!!
    Hal_Dmac_Configure_DMA_Handshake(&pcm_tx_data0_dma_legerity);


    /*
     * Configure DMA's channel setting for PCM Receive Data[31:0] Register
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */
    pcm_rx_data0_dma_legerity.channel_id = DMAC_CH_ID(pcm_rx_data0_dma_channel_num);//2    
    pcm_rx_data0_dma_legerity.target_select = DMAC_HW_HAND_SHAKE_PCM_RX0_ID;//1

    pcm_rx_data0_dma_legerity.src_addr = (SYSPA_PCM_BASE_ADDR + 0xA0);//SCR
    pcm_rx_data0_dma_legerity.dst_addr = (u_int32)pcm_rx0_buffer_le88221_p;//DST        

    pcm_rx_data0_dma_legerity.src_master = DMAC_CH_SRC_SEL_M0;//0
    pcm_rx_data0_dma_legerity.dst_master = DMAC_CH_DST_SEL_M1;//1

    pcm_rx_data0_dma_legerity.srcad_ctl = DMAC_CH_SRCAD_CTL_FIX;//2
    pcm_rx_data0_dma_legerity.dstad_ctl = DMAC_CH_DSTAD_CTL_INC;//0

    pcm_rx_data0_dma_legerity.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    pcm_rx_data0_dma_legerity.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    // Note here the total number of bytes is specified!!
    pcm_rx_data0_dma_legerity.transfer_bytes = BUFFER_SIZE * 4;

    pcm_rx_data0_dma_legerity.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;

    // Note this DMA's channel will be enabled when the following function is invoked!!
    Hal_Dmac_Configure_DMA_Handshake(&pcm_rx_data0_dma_legerity);
#endif


#if 0
    /*
     * Configure DMA's channel setting for PCM Transmit Data[31:0] Register
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */    
    pcm_tx_data1_dma_legerity.channel_id = DMAC_CH_ID(pcm_tx_data1_dma_channel_num);//1
    pcm_tx_data1_dma_legerity.target_select = DMAC_HW_HAND_SHAKE_PCM_TX1_ID;

    pcm_tx_data1_dma_legerity.src_addr = (u_int32)pcm_tx1_buffer_le88221_p;
    pcm_tx_data1_dma_legerity.dst_addr = (SYSPA_PCM_BASE_ADDR + 0x9C);    

    pcm_tx_data1_dma_legerity.src_master = DMAC_CH_SRC_SEL_M1;
    pcm_tx_data1_dma_legerity.dst_master = DMAC_CH_DST_SEL_M0;

    pcm_tx_data1_dma_legerity.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;
    pcm_tx_data1_dma_legerity.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;

    pcm_tx_data1_dma_legerity.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    pcm_tx_data1_dma_legerity.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    // Note here the total number of bytes is specified!!
    pcm_tx_data1_dma_legerity.transfer_bytes = BUFFER_SIZE * 4;

    pcm_tx_data1_dma_legerity.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;

    Hal_Dmac_Configure_DMA_Handshake(&pcm_tx_data1_dma_legerity);



    /*
     * Configure DMA's channel setting for PCM Receive Data[31:0] Register
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */    
    pcm_rx_data1_dma_legerity.channel_id = DMAC_CH_ID(pcm_rx_data1_dma_channel_num);//3
    pcm_rx_data1_dma_legerity.target_select = DMAC_HW_HAND_SHAKE_PCM_RX1_ID;
    pcm_rx_data1_dma_legerity.src_addr = (SYSPA_PCM_BASE_ADDR + 0xA4);    
    pcm_rx_data1_dma_legerity.dst_addr = (u_int32)pcm_rx1_buffer_le88221_p;    

    pcm_rx_data1_dma_legerity.src_master = DMAC_CH_SRC_SEL_M0;
    pcm_rx_data1_dma_legerity.dst_master = DMAC_CH_DST_SEL_M1;

    pcm_rx_data1_dma_legerity.srcad_ctl = DMAC_CH_SRCAD_CTL_FIX;
    pcm_rx_data1_dma_legerity.dstad_ctl = DMAC_CH_DSTAD_CTL_INC;

    pcm_rx_data1_dma_legerity.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    pcm_rx_data1_dma_legerity.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    // Note here the total number of bytes is specified!!
    pcm_rx_data1_dma_legerity.transfer_bytes = BUFFER_SIZE * 4;

    pcm_rx_data1_dma_legerity.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;

    Hal_Dmac_Configure_DMA_Handshake(&pcm_rx_data1_dma_legerity);    
#endif            
  
    return;
}

static irqreturn_t str8100_pcm_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
    u_int32 volatile    interrupt_status;    

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
#if 1
    PCM_INTERRUPT_STATUS_REG&=0xc0;

	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
#endif
	DEBUG_PRINT("%s: this_irq=%d, PCM_INTERRUPT_STATUS_REG=0x%.8x\n",__FUNCTION__,this_irq,PCM_INTERRUPT_STATUS_REG);

    // Get PCM interrupt status
    HAL_PCM_READ_INTERRUPT_STATUS(interrupt_status);   
    if (interrupt_status & PCM_RXBUF_FULL_FG){
    	rxbuf_full++;
		DEBUG_PRINT("%s: rx buf full\n",__FUNCTION__);
    }

    if (interrupt_status & PCM_TXBUF_EMPTY_FG){
    	txbuf_empty++;
        DEBUG_PRINT("%s: tx buf empty\n",__FUNCTION__);
    }
    if (interrupt_status & PCM_RXBUF_OVERRUN_FG){
        // Clear PCM interrupt status
        HAL_PCM_CLEAR_INTERRUPT_STATUS(PCM_RXBUF_OVERRUN_FG);
    
        rxbuf_overrun++;
        DEBUG_PRINT("%s: rx buf overrun\n",__FUNCTION__);
    }

    if (interrupt_status & PCM_TXBUF_UNDERRUN_FG){
        // Clear PCM interrupt status
        HAL_PCM_CLEAR_INTERRUPT_STATUS(PCM_TXBUF_UNDERRUN_FG);

        txbuf_underrun++;
        DEBUG_PRINT("%s: tx buf underrun\n",__FUNCTION__);
    }
    PCM_INTERRUPT_STATUS_REG&=0xc0;

	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}


static irqreturn_t str8100_dma_tc_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
    u_int32 volatile    dma_tc_status;
	u32 len,i;
	DEBUG_PRINT("%s: this_irq=%d,tc_status=0x%.8x\n",__FUNCTION__,this_irq,((DMAC_INT_TC_STATUS_REG) & 0xFF) );

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);

    HAL_DMAC_READ_TERMINAL_COUNT_INTERRUPT_STATUS(dma_tc_status);


    HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_GDMAC_TC_BIT_INDEX);
    HAL_DMAC_READ_TERMINAL_COUNT_INTERRUPT_STATUS(dma_tc_status);

    //DMA Rx reg 1
    if (dma_tc_status & DMAC_CH_ID(pcm_rx_data1_dma_channel_num))
    {
        HAL_DMAC_DISABLE_CHANNEL(pcm_rx_data1_dma_channel_num);
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(pcm_rx_data1_dma_channel_num));
        dma_tc_status &= ~(DMAC_CH_ID(pcm_rx_data1_dma_channel_num));
    
        //make pcm ch0 to ch1, and vice versa
        for(i=0;i<BUFFER_SIZE;i++)  pcm_rx1_buffer_le88221[i] = ((pcm_rx1_buffer_le88221[i] >> 8) & 0x000000FF) | ((pcm_rx1_buffer_le88221[i] << 8) & 0x0000FF00);
        
        //swap buffer
        if(pcm_rx1_buffer_le88221==buffer3) {pcm_rx1_buffer_le88221=buffer2;pcm_rx1_buffer_le88221_p=buffer_p2;}
        else {pcm_rx1_buffer_le88221=buffer3;pcm_rx1_buffer_le88221_p=buffer_p3;}
   
	

        // Re-initialize DMA's channel for Rx
        HAL_DMAC_WRITE_CHANNEL3_DESTINATION_ADDRESS((u_int32)pcm_rx1_buffer_le88221_p);
        HAL_DMAC_WRITE_CHANNEL3_SOURCE_ADDRESS(SYSPA_PCM_BASE_ADDR + 0xa4);
        HAL_DMAC_WRITE_CHANNEL3_TRANSFER_SIZE(BUFFER_SIZE);
        HAL_DMAC_ENABLE_CHANNEL3();
                        
    	pcm_dma_ch3_tc_count++;
    }       
    //DMA Tx reg 1
    if (dma_tc_status & DMAC_CH_ID(pcm_tx_data1_dma_channel_num))
    {
        HAL_DMAC_DISABLE_CHANNEL(pcm_tx_data1_dma_channel_num);
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(pcm_tx_data1_dma_channel_num));
        dma_tc_status &= ~(DMAC_CH_ID(pcm_tx_data1_dma_channel_num));
    
        //swap buffer
        if(pcm_tx1_buffer_le88221==buffer3) {pcm_tx1_buffer_le88221=buffer2;pcm_tx1_buffer_le88221_p=buffer_p2;}
        else {pcm_tx1_buffer_le88221=buffer3;pcm_tx1_buffer_le88221_p=buffer_p3;}
   
	
        // Re-initialize DMA's channel for Tx
        HAL_DMAC_WRITE_CHANNEL1_SOURCE_ADDRESS((u_int32)pcm_tx1_buffer_le88221_p);
        HAL_DMAC_WRITE_CHANNEL1_DESTINATION_ADDRESS(SYSPA_PCM_BASE_ADDR + 0x9c);
        HAL_DMAC_WRITE_CHANNEL1_TRANSFER_SIZE(BUFFER_SIZE);
        HAL_DMAC_ENABLE_CHANNEL1();
                        
    	pcm_dma_ch1_tc_count++;
    }       

    //DMA Rx reg 0
    if (dma_tc_status & DMAC_CH_ID(pcm_rx_data0_dma_channel_num))
    {
        HAL_DMAC_DISABLE_CHANNEL(pcm_rx_data0_dma_channel_num);
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(pcm_rx_data0_dma_channel_num));
        dma_tc_status &= ~(DMAC_CH_ID(pcm_rx_data0_dma_channel_num));
                
        //make pcm ch0 to ch1, and vice versa
        for(i=0;i<BUFFER_SIZE;i++)  pcm_rx0_buffer_le88221[i] = ((pcm_rx0_buffer_le88221[i] >> 8) & 0x000000FF) | ((pcm_rx0_buffer_le88221[i] << 8) & 0x0000FF00);

        if(pcm_rx0_buffer_le88221==buffer0) {pcm_rx0_buffer_le88221=buffer1;pcm_rx0_buffer_le88221_p=buffer_p1;}
        else {pcm_rx0_buffer_le88221=buffer0;pcm_rx0_buffer_le88221_p=buffer_p0;}
        
        // Re-initialize DMA's channel for Rx
        HAL_DMAC_WRITE_CHANNEL2_DESTINATION_ADDRESS((u_int32)pcm_rx0_buffer_le88221_p);
        HAL_DMAC_WRITE_CHANNEL2_SOURCE_ADDRESS(SYSPA_PCM_BASE_ADDR + 0xA0);
        HAL_DMAC_WRITE_CHANNEL2_TRANSFER_SIZE(BUFFER_SIZE);
        HAL_DMAC_ENABLE_CHANNEL2();
                        
    	pcm_dma_ch2_tc_count++;
    }       

    //DMA Tx reg 0
    if (dma_tc_status & DMAC_CH_ID(pcm_tx_data0_dma_channel_num))
    {
        HAL_DMAC_DISABLE_CHANNEL(pcm_tx_data0_dma_channel_num);
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(pcm_tx_data0_dma_channel_num));
        dma_tc_status &= ~(DMAC_CH_ID(pcm_tx_data0_dma_channel_num));

        //swap buffer
        if(pcm_tx0_buffer_le88221==buffer0) {pcm_tx0_buffer_le88221=buffer1;pcm_tx0_buffer_le88221_p=buffer_p1;}
        else {pcm_tx0_buffer_le88221=buffer0;pcm_tx0_buffer_le88221_p=buffer_p0;}
        
        // Re-initialize DMA's channel for Tx
        HAL_DMAC_WRITE_CHANNEL0_SOURCE_ADDRESS((u_int32)pcm_tx0_buffer_le88221_p);
        HAL_DMAC_WRITE_CHANNEL0_DESTINATION_ADDRESS(SYSPA_PCM_BASE_ADDR + 0x98);
        HAL_DMAC_WRITE_CHANNEL0_TRANSFER_SIZE(BUFFER_SIZE);
        HAL_DMAC_ENABLE_CHANNEL0();
                        
    	pcm_dma_ch0_tc_count++;
    }       

    /*
     * If there is any bit set, it means something wrong!!
     */
    if (dma_tc_status)
    {
        // Something wrong because spurious interrupt happens!!
        printk("Something wrong because spurious interrupt happens(%.8x)!!\n",dma_tc_status);
    }

	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}

void pcm_init(){
	u32 flags;
	u32 tmp;
	local_irq_save(flags);
    /*
     * Check CLK_OUT_SEL_Pin for 8.192 MHz
     */
	DEBUG_PRINT("%s:\n",__FUNCTION__);
//    HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS();
    HAL_PWRMGT_CONFIGURE_CLOCK_OUT_PIN(10, 0);

    /*
     * For IDL Case:
     *     UDCLK    : 4.096 MHz
     *     PCMCLK   : 2.048 MHz = 4.096/(1 + 1)
     *     FSYNCCLK : 8 KHz = 2048000/(255 + 1)
     */
    pcm_object.config_0 = ((/*clock_rate_ctrl*/1 & 0x7) << 0) |    /* Configure master clock rate */
                          (0 << 12) |                   /* Disable loopback mode */
                          (1 << 13) |                   /* Enable master mode */
                          (0 << 14) |                   /* Select IDL mode */
                          /*(1 << 24) | */                  /* Enable PCM data swap */
                          (0 << 24) |                   /* Disable PCM data swap */
                          (0 << 31);                    /* Disable PCM */

    /*
     * Note FSYNC_WIDE will be ignored when the PCM is configured as Slave or as Master
     * with GCI mode
     */    
    pcm_object.config_1 = ((0 & 0x1) << 15);    /* Select FSYNC mode , 0 : short FSYNC, 1 : long FSYNC */


    /*
     * Configure the settings of PCM's channel
     */
    pcm_object.channel_0_config = ((CH0_TX_Le88221_DELAY & 0x7F) << 0) |
                                  ((CH0_RX_Le88221_DELAY & 0x7F) << 8) |
                                  ((PCM_DATA_BIT_8 & 0x1) << 22) |
                                  (1 << 23);    /* Enable this channel */
                                  
    pcm_object.channel_1_config = ((CH1_TX_Le88221_DELAY & 0x7F) << 0) |
                                  ((CH1_RX_Le88221_DELAY & 0x7F) << 8) |
                                  ((PCM_DATA_BIT_8 & 0x1) << 22) |
                                  (1 << 23);    /* Enable this channel */

    pcm_object.channel_2_config = ((CH2_TX_Le88221_DELAY & 0x7F) << 0) |
                                  ((CH2_RX_Le88221_DELAY & 0x7F) << 8) |
                                  ((0 & 0x1) << 22) |
                                  (0 << 23);    /* Disable this channel */

    pcm_object.channel_3_config = ((CH3_TX_Le88221_DELAY & 0x7F) << 0) |
                                  ((CH3_RX_Le88221_DELAY & 0x7F) << 8) |
                                  ((0 & 0x1) << 22) |
                                  (0 << 23);    /* Disable this channel */


    // Enable PCM's interrupt sources
//    pcm_object.interrupt_config = 0;
    pcm_object.interrupt_config = PCM_RXBUF_OVERRUN_FG | PCM_TXBUF_UNDERRUN_FG;
//    pcm_object.interrupt_config |=PCM_RXBUF_FULL_FG|PCM_TXBUF_EMPTY_FG;


   
    // Initialize PCM's setting
    Hal_Pcm_Initialize(&pcm_object);
    
    
    // Disable PCM interrupt since GDMA hardware handshake interrupt will be used.
    //HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_PCM_BIT_INDEX);

    // Initialize GDMA hardware handshake for PCM
    Pcm_Configure_DMA_Hardware_Handshake_For_Legerity();


    /*
     * PCM will start to transmit and receive data once PCM is enabled. To avoid
     * PCM Transmit Buffer underrun, we have to put one transmit data into PCM
     * Transmit Buffer before PCM is enabled!!
     * Note PCM channel 0 is used.
     */
/*
	PCM_TX_DATA_31_0_REG=0;
	PCM_TX_DATA_63_32_REG=0;

	tmp=PCM_RX_DATA_31_0_REG;
	tmp=PCM_RX_DATA_63_32_REG;
*/
    HAL_PCM_CLEAR_INTERRUPT_STATUS(PCM_RXBUF_OVERRUN_FG | PCM_TXBUF_UNDERRUN_FG);//(0x4 | 0x8)=0xC

/*{
	u32 dma_ch;
	for (dma_ch = 0; dma_ch < DMAC_MAX_CHANNEL_NUM; dma_ch++)
	{
			HAL_DMAC_CLEAR_ERROR_ABORT_INTERRUPT_STATUS(DMAC_CH_ID(dma_ch));
        HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(dma_ch));
	}	
}
*/
    HAL_DMAC_ENABLE();    
    HAL_PCM_ENABLE_PCM();

    /*
     * Configure Legerity's Le88221 MPI Interface
     */
    Pcm_Initial_Legerity_Le88221();
	local_irq_restore(flags);

    
	DEBUG_PRINT("%s: end =>\n",__FUNCTION__);

}

//=================================================================================
static int proc_read_pcm(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int len=0;
	DEBUG_PRINT("%s:\n",__FUNCTION__);
	len += sprintf(buf,	"test\n"
				"test\n"); 
	*eof = 1;
	return len;
}

static proc_write_pcm(struct file *file, const char *buffer, unsigned long count, void *data){
	int len=0;
	DEBUG_PRINT("%s: count=%d\n",__FUNCTION__,count);
	pcm_init();

	return count;
	//is buffer free?
	if(txpos!=txlen){
		printk("%s: buffer not free\n");
		return -EBUSY;
	}

	//copy the raw data to local buffer	
	if(count>BUFFER_SIZE) len=BUFFER_SIZE;
	else len=count;
	
	if(copy_from_user(txbuffer,buffer,len)){
		return -EFAULT;	
	}
	txlen=len;
	txpos=0;

	//Initialization	

	// Enable CPU interrupt
	local_irq_enable();
	
	/*
	 * Note DMA must be enabled first before PCM is enabled
	 */
	HAL_DMAC_ENABLE();


	while (1){
		local_irq_disable();
		if (txpos>txlen||txlen==0){
			// Disable PCM
			
			break;
		}
		local_irq_enable();
	}
	DEBUG_PRINT("%s: exit. \n",__FUNCTION__);

	local_irq_enable();

	return len;

debug:
	
	return count;
}

static void __exit pcm_exit_module(void){
	printk("%s:\n",__FUNCTION__);
	remove_proc_entry("str8100/pcm", NULL);
	free_irq(INTC_PCM_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_TC_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_ERROR_BIT_INDEX, NULL);
/*	if(txbuffer) {
		pci_free_consistent(NULL, BUFFER_SIZE*4, txbuffer, txbuffer_p);
		txbuffer=txbuffer_p=NULL;
	}
*/
#if 1
	if(buffer0) {
		pci_free_consistent(NULL, BUFFER_SIZE*4, buffer0, buffer_p0);
		buffer0=buffer_p0=NULL;
	}

	if(buffer1) {
		pci_free_consistent(NULL, BUFFER_SIZE*4, buffer1, buffer_p1);
		buffer1=buffer_p1=NULL;
	}
	if(buffer2) {
		pci_free_consistent(NULL, BUFFER_SIZE*4, buffer2, buffer_p2);
		buffer2=buffer_p2=NULL;
	}

	if(buffer3) {
		pci_free_consistent(NULL, BUFFER_SIZE*4, buffer3, buffer_p3);
		buffer3=buffer_p3=NULL;
	}
#else
	printk("%s:buffers not freed yet!!!\n",__FUNCTION__);
#endif	
}

extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);
static int __init pcm_init_module(void)
{
	
	u32 ret;
	
	printk("%s:\n",__FUNCTION__);

	star_pcm_proc_entry = create_proc_entry("str8100/pcm", S_IFREG | S_IRUGO, NULL);
	if(!star_pcm_proc_entry){
		return -EBUSY;
	}
	star_pcm_proc_entry->read_proc=proc_read_pcm;
	star_pcm_proc_entry->write_proc=proc_write_pcm;
	
/*
	txbuffer = pci_alloc_consistent(NULL, BUFFER_SIZE, &txbuffer_p);
	if(!txbuffer){
		printk("%s: alloc txbuffer failed.\n",__FUNCTION__);
		goto exit1;
	}
*/
	buffer0 = pci_alloc_consistent(NULL, BUFFER_SIZE*4, &buffer_p0);
	if(!buffer0){
		printk("%s: alloc buffer0 failed.\n",__FUNCTION__);
		goto exit1;
	}
	buffer1 = pci_alloc_consistent(NULL, BUFFER_SIZE*4, &buffer_p1);
	if(!buffer1){
		printk("%s: alloc buffer1 failed.\n",__FUNCTION__);
		goto exit1;
	}
	buffer2 = pci_alloc_consistent(NULL, BUFFER_SIZE*4, &buffer_p2);
	if(!buffer2){
		printk("%s: alloc buffer2 failed.\n",__FUNCTION__);
		goto exit1;
	}
	buffer3 = pci_alloc_consistent(NULL, BUFFER_SIZE*4, &buffer_p3);
	if(!buffer3){
		printk("%s: alloc buffer3 failed.\n",__FUNCTION__);
		goto exit1;
	}
	DEBUG_PRINT("%s: buffers allocated... \n",__FUNCTION__);

	pcm_rx0_buffer_le88221=buffer0;
	pcm_tx0_buffer_le88221=buffer1;
	pcm_rx1_buffer_le88221=buffer2;
	pcm_tx1_buffer_le88221=buffer3;
	
	pcm_rx0_buffer_le88221_p=buffer_p0;
	pcm_tx0_buffer_le88221_p=buffer_p1;
	pcm_rx1_buffer_le88221_p=buffer_p2;
	pcm_tx1_buffer_le88221_p=buffer_p3;
/*
	str8100_set_interrupt_trigger (INTC_PCM_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);
	if((ret=request_irq(INTC_PCM_BIT_INDEX, str8100_pcm_irq_handler, 0, "pcm", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_PCM_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}
*/
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

	//pcm_init();

	return 0;
exit1:
	//pcm_exit_module();
	return -EBUSY;
}

module_init(pcm_init_module);
module_exit(pcm_exit_module);
