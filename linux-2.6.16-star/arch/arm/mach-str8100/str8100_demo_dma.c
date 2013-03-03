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

#include <linux/kernel.h>	/* printk() */
#include <linux/delay.h>
#include <asm/arch/star_demo_dma.h>
#include <asm/arch/star_i2s.h>
#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_gpio.h>

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
//#define DEBUG_PRINT(arg...) if(debug) printk(arg);
#define DEBUG_PRINT printk
#else
#define DEBUG_PRINT(arg...) 
#endif


/******************************************************************************
 *
 * FUNCTION:  Hal_Dmac_Get_Channel_Transfer_Unit_Number
 * PURPOSE:   
 *
 ******************************************************************************/
u32 Hal_Dmac_Get_Channel_Transfer_Unit_Number(u32 byte_size, u32 src_width)
{
    u32    transfer_unit_num;
    
    
    if (src_width == DMAC_CH_SRC_WIDTH_8_BITS)  // 8-bit
    {
        transfer_unit_num = byte_size;
    }
    else if (src_width == DMAC_CH_SRC_WIDTH_16_BITS)  // 16-bit
    {
        if (byte_size % 2)
        {
            transfer_unit_num = (byte_size >> 1) + 1;
        }
        else
        {
            transfer_unit_num = (byte_size >> 1);
        }
    }
    else if (src_width == DMAC_CH_SRC_WIDTH_32_BITS)  // 32-bit
    {
        if (byte_size % 4)
        {
            transfer_unit_num = (byte_size >> 2) + 1;
        }
        else
        {
            transfer_unit_num = (byte_size >> 2);
        }
    }
    else
    {
        transfer_unit_num = 0;
    }
    
    return transfer_unit_num;
}
EXPORT_SYMBOL(Hal_Dmac_Get_Channel_Transfer_Unit_Number);


void Hal_Dmac_Configure_DMA_Handshake(DMAC_HARDWARE_HANDSHAKE_OBJ_T *dmac_obj)
{
    u32    channel_control, ch;

    
    /*
     * Configure DMA controller for UART's hardware DMA handshake mode
     */    
    HAL_DMAC_DISABLE();
    
//#if (ENDIAN_MODE == BIG_ENDIAN)
#if 0
    /*Set Master0 and Master 1 endianness as Big Endian*/
    HAL_DMAC_SET_MASTER0_BIG_ENDIAN();
    HAL_DMAC_SET_MASTER1_BIG_ENDIAN();
#else
    /*Set Master0 and Master 1 endianness as Little Endian*/
    HAL_DMAC_SET_MASTER0_LITTLE_ENDIAN();
    HAL_DMAC_SET_MASTER1_LITTLE_ENDIAN();
#endif

    //Clear TC interrupt status    
    HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(0xFF);        // 8 channels

    //Clear Errot/Abort interrupt status    
    HAL_DMAC_CLEAR_ERROR_ABORT_INTERRUPT_STATUS(0x00FF00FF);    // 8 channels

    /*
     * Configure DMA's channel control
     */   
    channel_control = ((DMAC_CH_TC_MASK_DISABLE << 31) | \
                       ((dmac_obj->target_select&0xf) << 25) | \
                       (DMAC_CH_PRI_LEVEL_3 << 22) | \
                       (DMAC_CH_PROT3_NON_CACHEABLE << 21) | \
                       (DMAC_CH_PROT2_NON_BUFFERABLE << 20) | \
                       (DMAC_CH_PROT1_PRIVILEGED_MODE << 19) | \
                       ((dmac_obj->src_burst_size&0x7) << 16) | \
                       ((dmac_obj->src_width&0x7) << 11) | \
                       ((dmac_obj->dst_width&0x7) << 8) | \
                       (DMAC_CH_MODE_HW_HANDSHAKE << 7) | \
                       ((dmac_obj->srcad_ctl&0x3) << 5) | \
                       ((dmac_obj->dstad_ctl&0x3) << 3) | \
                       (dmac_obj->src_master << 2) | \
                       (dmac_obj->dst_master << 1) | \
                       (DMAC_CH_DISABLE));

#if 0
    // for testing only
    if (dmac_obj->llp_addr != 0)
    {
        /*
         * For LLP hardware handshaking with at least one descriptors, disable TC interrupt 
         * of the first descriptor.
         */
        channel_control |= (DMAC_CH_TC_MASK_ENABLE << 31);
    }
#endif
    for (ch = 0; ch < DMAC_MAX_CHANNEL_NUM; ch++)
    {
        if (dmac_obj->channel_id & DMAC_CH_ID(ch))
        {  
            /*
             * Configure channel's CSR register
             */
            DMAC_CH_CSR_REG(ch) = channel_control & 0xFFFFFFFE; //Disable CH(n) DMA
                
            /*
             * Configure channel's CFG register: disable channel abort interrupt,
             * enable channel error and terminal count interrupts.
             */
            DMAC_CH_CFG_REG(ch) |= (0x07);
            DMAC_CH_CFG_REG(ch) &= ~((0x3));
        }                               	
    }

    for (ch = 0; ch < DMAC_MAX_CHANNEL_NUM; ch++)
    {
        if (dmac_obj->channel_id & DMAC_CH_ID(ch))
        {  
            //Set Src address register
            DMAC_CH_SRC_ADDR_REG(ch)= dmac_obj->src_addr;

            //Set Dst address register
            DMAC_CH_DST_ADDR_REG(ch)= dmac_obj->dst_addr;    

            //Set Transfer Number
            if (dmac_obj->src_width == DMAC_CH_SRC_WIDTH_8_BITS)
            {
                DMAC_CH_SIZE_REG(ch) = (dmac_obj->transfer_bytes & 0x0FFF);
                DEBUG_PRINT("%s: 8-bits transfer_bytes=%d, DMAC_CH_SIZE_REG(%d)=%.8x\n",__FUNCTION__,dmac_obj->transfer_bytes,ch,DMAC_CH_SIZE_REG(ch));
            }
            else if (dmac_obj->src_width == DMAC_CH_SRC_WIDTH_16_BITS)
            {                                  
                DMAC_CH_SIZE_REG(ch) = ((dmac_obj->transfer_bytes >> 1) + (dmac_obj->transfer_bytes % 2)) & 0x0FFF;
                DEBUG_PRINT("%s: 16-bits transfer_bytes=%d, DMAC_CH_SIZE_REG(%d)=%.8x\n",__FUNCTION__,dmac_obj->transfer_bytes,ch,DMAC_CH_SIZE_REG(ch));
            }
            else if (dmac_obj->src_width == DMAC_CH_SRC_WIDTH_32_BITS)
            {
                DMAC_CH_SIZE_REG(ch) = ((dmac_obj->transfer_bytes >> 2) + ((dmac_obj->transfer_bytes % 4) ? 1 : 0)) & 0x0FFF;
                DEBUG_PRINT("%s: 32-bits transfer_bytes=%d, DMAC_CH_SIZE_REG(%d)=%.8x\n",__FUNCTION__,dmac_obj->transfer_bytes,ch,DMAC_CH_SIZE_REG(ch));
            }
            else
            {
            	DEBUG_PRINT("%s: dead\n",__FUNCTION__);
                while (1);
            }

            //Enable Channel DMA transfer 
            HAL_DMAC_ENABLE_CHANNEL(ch);
             
            //Set Channel's Sync logic
            DMAC_SYNC_REG |= (1 << ch);

            /*
             * Configure channel LLP if LLP is enabled
             */
            DMAC_CH_LLP_REG(ch) = (dmac_obj->llp_addr == 0) ? 0 : (dmac_obj->llp_addr & 0xFFFFFFFC);
        }
    }
}
EXPORT_SYMBOL(Hal_Dmac_Configure_DMA_Handshake);

irqreturn_t str8100_dma_err_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	u32 dma_error_status,dma_ch;
printk("%s: this_irq=%d\n",__FUNCTION__,this_irq);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	//todo:

	HAL_DMAC_READ_ERROR_ABORT_INTERRUPT_STATUS(dma_error_status);
	for (dma_ch = 0; dma_ch < DMAC_MAX_CHANNEL_NUM; dma_ch++)
	{
		if (dma_error_status & DMAC_CH_ID(dma_ch))
		{
			printk("%s: this_irq=%d, DMA channel error on ch %d\n",__FUNCTION__,this_irq,dma_ch);
			HAL_DMAC_DISABLE_CHANNEL(dma_ch);
			HAL_DMAC_CLEAR_ERROR_ABORT_INTERRUPT_STATUS(DMAC_CH_ID(dma_ch));
		}
	}	
	
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL(str8100_dma_err_irq_handler);

static const u32 rate_map[3][4]={	{48000,    0,    0,    0},
					{44100,22050,11025, 5512},
					{32000,16000, 8000,    0}};
    /*
     * Configure I2S related parameters
     */
/*//#define i2s_sdata_width	I2S_DATA_16_BIT
#define i2s_sdata_width		I2S_DATA_32_BIT
#define i2s_mode	 	I2S_SLAVE_MODE
//#define i2s_mode		I2S_MASTER_MODE
#define i2s_tranfer_timing_ctrl	I2S_I2S_MODE
//#define i2s_tranfer_timing_ctrl	I2S_RJF_MODE
//#define i2s_tranfer_timing_ctrl	I2S_LJF_MODE
#define i2s_sclk_mode		I2S_CLOCK_CONTINUOUS_MODE
//#define i2s_sclk_mode		I2S_CLOCK_256S_MODE
*/

//int str8100_i2s_init(sampling_rate,I2S_DATA_32_BIT, I2S_MASTER_MODE, I2S_I2S_MODE, I2S_CLOCK_256S_MODE);
//int str8100_i2s_init(sampling_rate,I2S_DATA_32_BIT, I2S_SLAVE_MODE, I2S_I2S_MODE, I2S_CLOCK_CONTINUOUS_MODE);

int str8100_i2s_init(int sampling_rate,u32 i2s_sdata_width, u32 i2s_mode,
			u32 i2s_tranfer_timing_ctrl, u32 i2s_sclk_mode){
	u32 tmp;
	u32 i/*clock*/ ,j,i2s_src_clk_div;
	DEBUG_PRINT("%s: sampling_rate=%d\n",__FUNCTION__,sampling_rate);
	
	for(i=0;i<3;i++)
		for(j=0;j<4;j++)
			if(rate_map[i][j]==sampling_rate) goto rate_match;

rate_match:
	if(i==3&&j==4) {
		printk("%s: unsupported sampling rate (%d)\n",__FUNCTION__,sampling_rate);
		return -1;
	}else{
		DEBUG_PRINT("%s: clock=%d,src_clk_div=%d\n",__FUNCTION__,i,j);
	}
	
	i2s_src_clk_div=j;

	/*
	 * Select I2S clock source; here we use 48K sampling rate for the target file.
	 */
	 
//	switch(sampling_rate){
	switch(i/*clock*/){
	case 0/*48000*/: 
		HAL_PWRMGT_I2S_CLOCK_SOURCE_12288000HZ(); 
		break; 
	case 1/*44100*/:
		HAL_PWRMGT_I2S_CLOCK_SOURCE_11289600HZ(); 
		break;
	case 2/*32000*/: 
		HAL_PWRMGT_I2S_CLOCK_SOURCE_8192000HZ(); 
		break; 
	}

	HAL_PWRMGT_CONFIGURE_CLOCK_OUT_PIN(11,0);
	
	// Enable I2S pins
	HAL_MISC_ENABLE_I2S_PINS();
	
	// Enable I2S clock
	HAL_PWRMGT_ENABLE_I2S_CLOCK(); 

	//Hal_Pwrmgt_Software_Reset(PWRMGT_P2S_SOFTWARE_RESET_BIT_INDEX);


    /*
     * Configure I2S to be Master & Transmitter
     * Note the I2S's WS Clock is derived from Clock & Power Management Functional
     * Block!!
     */
	I2S_CONFIGURATION_REG = 
                        (i2s_sdata_width << 0) |
                        (i2s_src_clk_div << 4) |
                        (0x1 << 12) |   /* Use I2SSD as data output pin and GPIOA[3] as data input pin for full-duplex mode */
                        (0x0 << 15) |   /* Disable clock phase invert */
                        (0x0 << 24) |   /* Disable I2S data swap */
                        ((i2s_sclk_mode & 0x1) << 25) |
                        ((i2s_tranfer_timing_ctrl & 0x3) << 26) |
                        (0x0 << 29) |   /* Enable I2S Transmitter */
			(i2s_mode << 30) |
			(0x0 << 31);	/* Disable I2S */

	//Enable none while initializing
	I2S_INTERRUPT_ENABLE_REG = 0x0;
//	I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG | I2S_TXBF_R_EMPTY_FLAG | I2S_TXBF_L_EMPTY_FLAG);

    // Clear spurious interrupt sources
    I2S_INTERRUPT_STATUS_REG = 0xF0;

    tmp = I2S_LEFT_RECEIVE_DATA_REG;
    tmp = I2S_RIGHT_RECEIVE_DATA_REG;

    // Disable I2S
    HAL_I2S_DISABLE_I2S();

	return 0;
}
EXPORT_SYMBOL(str8100_i2s_init);



/*
 * Define which GPIO Pins will serve as Winbond's W571C161's SSP (Serial Setup
 * Potr) Pins which consist of SSPEN, SSPCLK, and SSPTR.
 * Note these three pins are uni-directional output pins:
 * SSPEN  : mapping to COM1 pin
 * SSPCLK : mapping to COM2 pin
 * SSPTR  : mapping to COM3 pin
 */
//#define SSPEN_GPIO_INDEX        (3)   /* use GPIOA #3 as WM8772's COM1 */
#define SSPEN_GPIO_INDEX        (0)   /* use GPIOA #3 as WM8772's COM1 */

#define SSPCLK_GPIO_INDEX       (1)   /* use GPIOA #4 as WM8772's COM2 */

#define SSPTR_GPIO_INDEX        (2)   /* use GPIOA #5 as WM8772's COM3 */

#define SWITCH_GPIO_INDEX       (7)   /* use GPIOA #6 as WM8772's Transfer or Receiver */

/*
 * Macro-defines to "output" SSPEN, SSPCLK and SSPTR
 */
#define SSPEN_HIGH()            HAL_GPIOA_SET_DATA_OUT_HIGH(0x1 << SSPEN_GPIO_INDEX)

#define SSPEN_LOW()             HAL_GPIOA_SET_DATA_OUT_LOW(0x1 << SSPEN_GPIO_INDEX)


#define SSPCLK_HIGH()           HAL_GPIOA_SET_DATA_OUT_HIGH(0x1 << SSPCLK_GPIO_INDEX)

#define SSPCLK_LOW()            HAL_GPIOA_SET_DATA_OUT_LOW(0x1 << SSPCLK_GPIO_INDEX)


#define SSPTR_HIGH()            HAL_GPIOA_SET_DATA_OUT_HIGH(0x1 << SSPTR_GPIO_INDEX)

#define SSPTR_LOW()             HAL_GPIOA_SET_DATA_OUT_LOW(0x1 << SSPTR_GPIO_INDEX)


/*
 * SWITCH TX/RX For ASIC VER.
 */
#define SSPSW_HIGH()            HAL_GPIOA_SET_DATA_OUT_HIGH(0x1 << SWITCH_GPIO_INDEX)

#define SSPSW_LOW()             HAL_GPIOA_SET_DATA_OUT_LOW(0x1 << SWITCH_GPIO_INDEX)

/*
 * Macro-defines to configure the "directions" of SSPEN, SSPCLK and SSPTR
 */
#define SSPEN_DIR_OUT()         HAL_GPIOA_SET_DIRECTION_OUTPUT(0x1 << SSPEN_GPIO_INDEX)

#define SSPCLK_DIR_OUT()        HAL_GPIOA_SET_DIRECTION_OUTPUT(0x1 << SSPCLK_GPIO_INDEX)

#define SSPTR_DIR_OUT()         HAL_GPIOA_SET_DIRECTION_OUTPUT(0x1 << SSPTR_GPIO_INDEX)

//For ASIC
#define SSPSW_DIR_OUT()         HAL_GPIOA_SET_DIRECTION_OUTPUT(0x1 << SWITCH_GPIO_INDEX)


#define TIME_FACTOR               (20)

/*
 * Function prototype declaration
 */
u32            I2s_Gpio_SSP_Initialise(void);

void           I2s_Gpio_SSP_Write(u16);

void           I2s_Gpio_SSP_Switch_To_Record_Data(void);

void           I2s_Gpio_SSP_Switcg_To_Playback_Data(void);


/******************************************************************************
 *
 * FUNCTION:  I2s_Gpio_SSP_Initialise
 * PURPOSE:   
 *
 ******************************************************************************/
u32 I2s_Gpio_SSP_Initialise(void)
{
 
    // Enable GPIO clock
    HAL_PWRMGT_ENABLE_GPIO_CLOCK();

    // Perform GPIO software reset
//    Hal_Pwrmgt_Software_Reset(PWRMGT_GPIO_SOFTWARE_RESET_BIT_INDEX);


    /*
     * 1. Determine/Check which three GPIO pins will serve as SSPEN, SSPCLK and 
     *    SSPTR pins
     * 2. At initialization :
     *    SSPEN  : Output Direction, High state
     *    SSPCLK : Output Direction, Low state
     *    SSPTR  : Output Direction, Low state
     */
    SSPEN_DIR_OUT();
    
    SSPCLK_DIR_OUT();
    
    SSPTR_DIR_OUT();


    /*
     * Set SSPEN to be HIGH and SSPCLK/SSPTR be be LOW to "Free" SSP Bus
     */
     
    SSPEN_HIGH();

#if 1
    SSPCLK_LOW();

    SSPTR_LOW();
#else
    SSPCLK_HIGH();

    SSPTR_HIGH();
#endif

    return 0;  // for success indication
}
EXPORT_SYMBOL(I2s_Gpio_SSP_Initialise);


/******************************************************************************
 *
 * FUNCTION:  I2s_Gpio_SSP_Write
 * PURPOSE:   
 *
 ******************************************************************************/
void I2s_Gpio_SSP_Write(u16 reg_data)
{
    unsigned long   cpsr_flags;
    u32 volatile    ii;


	local_irq_save(cpsr_flags);
    
    SSPEN_HIGH();

    SSPCLK_LOW();

    SSPTR_LOW();

    ndelay(2 * TIME_FACTOR);

    SSPEN_LOW();

    // Output D15 ~ D0 of reg_data
    for (ii = 0; ii < 16; ii++)
    {
        SSPCLK_LOW();
    
        ndelay(3 * TIME_FACTOR);//setup time
        
        if (reg_data & 0x8000)
        {
            SSPTR_HIGH();
        }
        else
        {
            SSPTR_LOW();
        }
        
        ndelay(5 * TIME_FACTOR);
        
        SSPCLK_HIGH();
        
        ndelay(5 * TIME_FACTOR);//hold time

        SSPCLK_LOW();
        
        ndelay(3 * TIME_FACTOR);

        // Shift left by one bit to get the next data bit
        reg_data <<= 1;
    }
    
    SSPEN_HIGH();

    SSPCLK_LOW();

    ndelay(1 * TIME_FACTOR);

    // for testing only
#if 0
    SSPCLK_HIGH();

//    SSPTR_HIGH();
#endif


    // Delay some time
//    for (ii = 0; ii < 0x100000; ii++);    

	local_irq_restore(cpsr_flags);
}
EXPORT_SYMBOL(I2s_Gpio_SSP_Write);


/******************************************************************************
 *
 * FUNCTION:  I2s_Gpio_SSP_Switch_To_Record_Data
 * PURPOSE:   
 *
 ******************************************************************************/
void I2s_Gpio_SSP_Switch_To_Record_Data(void)
{
    SSPSW_DIR_OUT();
    
    SSPSW_HIGH();
}


/******************************************************************************
 *
 * FUNCTION:  I2s_Gpio_SSP_Switcg_To_Playback_Data
 * PURPOSE:   
 *
 ******************************************************************************/
void I2s_Gpio_SSP_Switcg_To_Playback_Data(void)
{
    SSPSW_DIR_OUT();
    
    SSPSW_LOW();
}

