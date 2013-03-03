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

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <linux/sysctl.h>

#include <asm/arch/star_intc.h>
#include <asm/arch/star_i2c.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_powermgt.h>

#include "i2c-str8100.h"

#define STR8100_I2C_DATE          "20060613"
#define STR8100_I2C_VERSION       "1.0.0"

#define I2C_K                     1000
#define I2C_M                     1000000

//extern u32 PLL_clock;
//extern u32 CPU_clock;
//extern u32 AHB_clock;
extern u32 APB_clock;
#define I2C_PCLK                  APB_clock
/*#if 1 // for ASIC
#define I2C_PCLK                  200000000
#else // for FPGA
#define I2C_PCLK                  13000000
#endif
*/
#define TWI_TIMEOUT	          2*(HZ)

#define I2C_100KHZ	  100000
#define I2C_200KHZ	  200000
#define I2C_300KHZ	  300000
#define I2C_400KHZ	  400000

static i2c_transfer_t i2c_cmd_transfer;

unsigned int debug=0;
module_param(debug, uint, 0);
MODULE_PARM_DESC(debug, "STR8100 I2C debug option (0:off 1:on, default=0)");

static unsigned int current_clock;
unsigned int clock=400000;
module_param(clock, uint, 0);
MODULE_PARM_DESC(clock, "STR8100 I2C clock in Hz (default=400000)");

static wait_queue_head_t waitqueue;	/* wait queue for read/write to complete */

extern void str8100_set_interrupt_trigger (unsigned int, unsigned int, unsigned int, unsigned int);
#define u_int32 unsigned int

/******************************************************************************
 *
 * FUNCTION:  Hal_I2c_Is_Bus_Idle
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 Hal_I2c_Is_Bus_Idle(void)
{
    /*
     * Return value :
     *    1 : Bus Idle
     *    0 : Bus Busy
     */    
    return ((I2C_CONTROLLER_REG & (0x1 << 6)) ? 0 : 1);
}

/******************************************************************************
 *
 * FUNCTION:  Hal_I2c_Is_Action_Done
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 Hal_I2c_Is_Action_Done(void)
{
    /*
     * Return value :
     *    1 : Action Done
     *    0 : Action is NOT Done
     */    
    return ((I2C_INTERRUPT_STATUS_REG & I2C_ACTION_DONE_FLAG) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Hal_I2c_Dispatch_Transfer
 * PURPOSE:
 *
 ******************************************************************************/
void Hal_I2c_Dispatch_Transfer(i2c_transfer_t *i2c_transfer)
{
    u_int32 volatile    i2c_control;
    u_int32 volatile    i2c_control_reg;


    /*
     * Wait unti I2C Bus is idle and the previous action is done
     */
//    while (!Hal_I2c_Is_Bus_Idle() && !Hal_I2c_Is_Action_Done());
    int retries = 2000;    
   
    while (!Hal_I2c_Is_Bus_Idle() && !Hal_I2c_Is_Action_Done() && retries--);	
         udelay(1000); 

    if (retries == 0) {
      printk ("%s: Bus idle fail!!\n",__FUNCTION__);  
      return;
    }


    // Configure transfer command, write data length, and read data length
    i2c_control = ((i2c_transfer->transfer_cmd & 0x3) << 4) |
                  ((i2c_transfer->write_data_len & 0x3) << 2) |
                  ((i2c_transfer->read_data_len & 0x3) << 0);
    
    // Note we enable I2C again!!
    i2c_control_reg = I2C_CONTROLLER_REG;
    
    i2c_control_reg &= ~(0x3F);
    i2c_control_reg |= (i2c_control & 0x3F) | ((u_int32)0x1 << 31);

    I2C_CONTROLLER_REG = i2c_control_reg;

    // Write output data
    I2C_WRITE_DATA_REG = i2c_transfer->write_data;

    // Configure slave address
    I2C_SLAVE_ADDRESS_REG = i2c_transfer->slave_addr & 0xFE;

    // Start IC transfer
    HAL_I2C_START_TRANSFER();
}

/******************************************************************************
 *
 * FUNCTION:  I2c_Read_Only_Command
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 I2c_Read_Only_Command(u_int32 slave_addr, u_int32 read_data_len, 
                              u_int32 *read_data)
{
    long timeout;


    // Clear previous I2C interrupt status
    IO_OUT_WORD(I2C_INTERRUPT_STATUS_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);

    // Enable I2C interrupt sources
    IO_OUT_WORD(I2C_INTERRUPT_ENABLE_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);


    /*
     * Configure this I2C command tranfer settings
     */
    i2c_cmd_transfer.transfer_cmd = I2C_READ_ONLY_CMD;
    
    i2c_cmd_transfer.write_data_len = 0;
    
    i2c_cmd_transfer.read_data_len = read_data_len & 0x3;

    i2c_cmd_transfer.slave_addr = slave_addr & 0xFF;

    i2c_cmd_transfer.write_data = 0;

    i2c_cmd_transfer.error_status = 0;

    i2c_cmd_transfer.action_done = 0;


    /*
     * Issue this command
     */
    Hal_I2c_Dispatch_Transfer(&i2c_cmd_transfer);


    // Check if this I2C bus action is done or not
/*    while (1)
    {
        Sys_Interrupt_Disable_Save_Flags(&cpsr_flags);
   
        if ((i2c_cmd_transfer.action_done) || (i2c_cmd_transfer.error_status))
        {
            break;
        }

        Sys_Interrupt_Restore_Flags(cpsr_flags);
    }

    Sys_Interrupt_Restore_Flags(cpsr_flags);
*/
    timeout = interruptible_sleep_on_timeout(&waitqueue, TWI_TIMEOUT);
    if (timeout == 0) return 0x99; 

    // I2C Bus error!!
    if (i2c_cmd_transfer.error_status && (i2c_cmd_transfer.error_status != 0xFF))
    {
        return (i2c_cmd_transfer.error_status);
    }

    // Get the read data byte
    i2c_cmd_transfer.read_data = IO_IN_WORD(I2C_READ_DATA_REG_ADDR);
    
    switch (read_data_len & 0x3)
    {
        case I2C_DATA_LEN_1_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFF;//8
            
            break;

        case I2C_DATA_LEN_2_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFFFF;//16
            
            break;

        case I2C_DATA_LEN_3_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFFFFFF;//24
            
            break;

        case I2C_DATA_LEN_4_BYTE :

        default :
        
            break;
    }


    // Set the data for return
    *read_data = i2c_cmd_transfer.read_data;

    return (0);
}



/******************************************************************************
 *
 * FUNCTION:  I2c_Write_Only_Command
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 I2c_Write_Only_Command(u_int32 slave_addr, u_int32 write_data_len,
                               u_int32 write_data)
{
    long timeout;


    // Clear previous I2C interrupt status
    IO_OUT_WORD(I2C_INTERRUPT_STATUS_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);

    // Enable I2C interrupt sources
    IO_OUT_WORD(I2C_INTERRUPT_ENABLE_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);


    /*
     * Configure this I2C command tranfer settings
     */
    i2c_cmd_transfer.transfer_cmd = I2C_WRITE_ONLY_CMD;
    
    i2c_cmd_transfer.write_data_len = write_data_len & 0x3;
    
    i2c_cmd_transfer.read_data_len = 0;

    i2c_cmd_transfer.slave_addr = slave_addr & 0xFF;

    switch (write_data_len & 0x3)
    {
        case I2C_DATA_LEN_1_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFF;
            
            break;

        case I2C_DATA_LEN_2_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFFFF;
            
            break;

        case I2C_DATA_LEN_3_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFFFFFF;
            
            break;

        case I2C_DATA_LEN_4_BYTE :

            i2c_cmd_transfer.write_data = write_data;

        default :

            i2c_cmd_transfer.write_data = write_data;
            
            break;
    }

    i2c_cmd_transfer.error_status = 0;

    i2c_cmd_transfer.action_done = 0;


    /*
     * Issue this command
     */
    Hal_I2c_Dispatch_Transfer(&i2c_cmd_transfer);


    // Check if this I2C bus action is done or not
/*    while (1)
    {
        Sys_Interrupt_Disable_Save_Flags(&cpsr_flags);
   
        if ((i2c_cmd_transfer.action_done) || (i2c_cmd_transfer.error_status))
        {
            break;
        }

        Sys_Interrupt_Restore_Flags(cpsr_flags);
    }

    Sys_Interrupt_Restore_Flags(cpsr_flags);
*/
    timeout = interruptible_sleep_on_timeout(&waitqueue, TWI_TIMEOUT);
    if (timeout == 0) return 0x99; 

    // I2C Bus error!!
    if (i2c_cmd_transfer.error_status && (i2c_cmd_transfer.error_status != 0xFF))
    {
        return (i2c_cmd_transfer.error_status);
    }
    else
    {
        return (0);
    }
}


/******************************************************************************
 *
 * FUNCTION:  I2c_Write_Read_Command
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 I2c_Write_Read_Command(u_int32 slave_addr, 
                               u_int32 write_data_len, u_int32 write_data,
                               u_int32 read_data_len, u_int32 *read_data)
{
    long timeout;


    // Clear previous I2C interrupt status
    IO_OUT_WORD(I2C_INTERRUPT_STATUS_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);

    // Enable I2C interrupt sources
    IO_OUT_WORD(I2C_INTERRUPT_ENABLE_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);


    /*
     * Configure this I2C command tranfer settings
     */
    i2c_cmd_transfer.transfer_cmd = I2C_WRITE_READ_CMD;
    
    i2c_cmd_transfer.write_data_len = write_data_len & 0x3;
    
    i2c_cmd_transfer.read_data_len = read_data_len & 0x3;

    i2c_cmd_transfer.slave_addr = slave_addr & 0xFF;

    switch (write_data_len & 0x3)
    {
        case I2C_DATA_LEN_1_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFF;
            
            break;

        case I2C_DATA_LEN_2_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFFFF;
            
            break;

        case I2C_DATA_LEN_3_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFFFFFF;
            
            break;

        case I2C_DATA_LEN_4_BYTE :

            i2c_cmd_transfer.write_data = write_data;

        default :

            i2c_cmd_transfer.write_data = write_data;
            
            break;
    }

    i2c_cmd_transfer.error_status = 0;

    i2c_cmd_transfer.action_done = 0;


    /*
     * Issue this command
     */
    Hal_I2c_Dispatch_Transfer(&i2c_cmd_transfer);


    // Check if this I2C bus action is done or not
/*    while (1)
    {
        Sys_Interrupt_Disable_Save_Flags(&cpsr_flags);
   
        if ((i2c_cmd_transfer.action_done) || (i2c_cmd_transfer.error_status))
        {
            break;
        }

        Sys_Interrupt_Restore_Flags(cpsr_flags);
    }

    Sys_Interrupt_Restore_Flags(cpsr_flags);
*/
    timeout = interruptible_sleep_on_timeout(&waitqueue, TWI_TIMEOUT);
    if (timeout == 0) return 0x99; 

    // I2C Bus error!!
    if (i2c_cmd_transfer.error_status && (i2c_cmd_transfer.error_status != 0xFF))
    {
        return (i2c_cmd_transfer.error_status);
    }

    // Get the read data byte
    i2c_cmd_transfer.read_data = IO_IN_WORD(I2C_READ_DATA_REG_ADDR);
    
    switch (read_data_len & 0x3)
    {
        case I2C_DATA_LEN_1_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFF;
            
            break;

        case I2C_DATA_LEN_2_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFFFF;
            
            break;

        case I2C_DATA_LEN_3_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFFFFFF;
            
            break;

        case I2C_DATA_LEN_4_BYTE :

        default :
            break;
    }


    // Set the data for return
    *read_data = i2c_cmd_transfer.read_data;

    return (0);
}



/******************************************************************************
 *
 * FUNCTION:  I2c_Read_Write_Command
 * PURPOSE:
 *
 ******************************************************************************/
u_int32 I2c_Read_Write_Command(u_int32 slave_addr, 
                               u_int32 read_data_len, u_int32 *read_data,
                               u_int32 write_data_len, u_int32 write_data)
{
    long timeout;


    // Clear previous I2C interrupt status
    IO_OUT_WORD(I2C_INTERRUPT_STATUS_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);

    // Enable I2C interrupt sources
    IO_OUT_WORD(I2C_INTERRUPT_ENABLE_REG_ADDR, I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG);


    /*
     * Configure this I2C command tranfer settings
     */
    i2c_cmd_transfer.transfer_cmd = I2C_READ_WRITE_CMD;
    
    i2c_cmd_transfer.write_data_len = write_data_len & 0x3;
    
    i2c_cmd_transfer.read_data_len = read_data_len & 0x3;

    i2c_cmd_transfer.slave_addr = slave_addr & 0xFF;

    switch (write_data_len & 0x3)
    {
        case I2C_DATA_LEN_1_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFF;
            
            break;

        case I2C_DATA_LEN_2_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFFFF;
            
            break;

        case I2C_DATA_LEN_3_BYTE :
        
            i2c_cmd_transfer.write_data = write_data & 0xFFFFFF;
            
            break;

        case I2C_DATA_LEN_4_BYTE :

            i2c_cmd_transfer.write_data = write_data;

        default :
        
            i2c_cmd_transfer.write_data = write_data;
            
            break;
    }

    i2c_cmd_transfer.error_status = 0;

    i2c_cmd_transfer.action_done = 0;


    /*
     * Issue this command
     */
    Hal_I2c_Dispatch_Transfer(&i2c_cmd_transfer);


    // Check if this I2C bus action is done or not
/*    while (1)
    {
        Sys_Interrupt_Disable_Save_Flags(&cpsr_flags);
   
        if ((i2c_cmd_transfer.action_done) || (i2c_cmd_transfer.error_status))
        {
            break;
        }

        Sys_Interrupt_Restore_Flags(cpsr_flags);
    }

    Sys_Interrupt_Restore_Flags(cpsr_flags);
*/
    timeout = interruptible_sleep_on_timeout(&waitqueue, TWI_TIMEOUT);
    if (timeout == 0) return 0x99; 


    // I2C Bus error!!
    if (i2c_cmd_transfer.error_status && (i2c_cmd_transfer.error_status != 0xFF))
    {
        return (i2c_cmd_transfer.error_status);
    }

    // Get the read data byte
    i2c_cmd_transfer.read_data = IO_IN_WORD(I2C_READ_DATA_REG_ADDR);
    
    switch (read_data_len & 0x3)
    {
        case I2C_DATA_LEN_1_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFF;
            
            break;

        case I2C_DATA_LEN_2_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFFFF;
            
            break;

        case I2C_DATA_LEN_3_BYTE :
        
            i2c_cmd_transfer.read_data &= 0xFFFFFF;
            
            break;

        case I2C_DATA_LEN_4_BYTE :

        default :

            break;
    }

    // Set the data for return
    *read_data = i2c_cmd_transfer.read_data;

    return (0);
}
//====================================================================================
static void str8100_i2c_init(void)
{ 
//	unsigned long clock = 100 * (priv->twi_cwgr + 1) * I2C_K; 

	current_clock=clock;
//	if(debug)
		printk("%s: current_clock=%ul, CLKDIV=%d\n",__FUNCTION__,current_clock,(I2C_PCLK / (2 * current_clock) - 1));
	
	HAL_MISC_ENABLE_I2C_PINS();	
	HAL_PWRMGT_ENABLE_I2C_CLOCK();
#if 0
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << PWRMGT_P2S_SOFTWARE_RESET_BIT_INDEX);   
	PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << PWRMGT_P2S_SOFTWARE_RESET_BIT_INDEX);   
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << PWRMGT_P2S_SOFTWARE_RESET_BIT_INDEX);
#endif
//	I2C_CONTROLLER_REG = (0 << 6) | (0 << 24) | (0 << 31);
	I2C_CONTROLLER_REG = 0 ;
//	I2C_TIME_OUT_REG = (((I2C_PCLK / (2 * clock) - 1)<<8)|(1 << 7)|0x40);
	I2C_TIME_OUT_REG = (((I2C_PCLK / (2 * current_clock) - 1)<<8)|(1 << 7)|0x10);
	I2C_INTERRUPT_ENABLE_REG = 0;
	I2C_INTERRUPT_STATUS_REG = I2C_BUS_ERROR_FLAG | I2C_ACTION_DONE_FLAG;
	HAL_I2C_ENABLE_I2C();
}
/*
typedef union __data_t{
	struct{
		unsigned char byte[4];
	} byte;
	unsigned int u32;
} data;

unsigned int i2c_read_len(unsigned int addr, unsigned char *buf, unsigned int len){
	data	data;
	if((ret=I2c_Read_Only_Command(addr,&data.u32,len)))
		return ret;
		
	switch(rem_len){
	case 1:
		*tmp_buf=byte[0];tmp_buf++;
		break;
	case 2:
		*tmp_buf=byte[1];tmp_buf++;
		*tmp_buf=byte[0];tmp_buf++;
		break;
	case 3:
		*tmp_buf=byte[2];tmp_buf++;
		*tmp_buf=byte[1];tmp_buf++;
		*tmp_buf=byte[0];tmp_buf++;
		break;
	case 4:
		*tmp_buf=byte[3];tmp_buf++;
		*tmp_buf=byte[2];tmp_buf++;
		*tmp_buf=byte[1];tmp_buf++;
		*tmp_buf=byte[0];tmp_buf++;
		break;
	default:
		return 99;
	}
	return 0;
}
*/
static int
i2c_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	unsigned int i;
	unsigned int data;
	int ret;

	if (len == 0) return 0;

	for(i=0;i<len;i++){
//	if(I2c_Write_Read_Command((addr<<1),0,0,3,&data)){
//	if(I2c_Eeprom_AT24C16A_Read_Byte(0x0a,7,0,&data)){
		if((ret=I2c_Read_Only_Command((addr<<1),0,&data))){
			if(debug)
				printk("Error %s: ret=0x%x\n",__FUNCTION__,ret);
			return -EIO;
		}
		buf[i] = data;
	}

	return 0;
}

static int
i2c_write(unsigned int addr, unsigned char *buf, unsigned int len)
{
	unsigned int i,data=0;
	int ret;
	if (len == 0) return 0;

	if (len >4) return -EIO;
	
	for(i=0;i<len;i++) data=data|(buf[i]<<(i<<3));

		if((ret=I2c_Write_Only_Command((addr<<1),len-1,data))){
			if(debug)
				printk("Error %s: ret=0x%x\n",__FUNCTION__,ret);
			return -EIO;
		}

	return 0;
}

static int str8100_xfer(struct i2c_adapter *adapter, struct i2c_msg msgs[], int num)
{
	struct i2c_msg *p;
	int i, err = 0;

	if(clock!=current_clock) str8100_i2c_init();
	 
	if(debug)
		printk("\n%s: num=%d\n",__FUNCTION__,num);
	for (i = 0; !err && i < num; i++) {
		if(debug)
			printk("%s: %s msgs[%d] addr=%x len=%d\n",__FUNCTION__,(msgs[i].flags & I2C_M_RD)?"read":"write",i,msgs[i].addr,msgs[i].len);
		p = &msgs[i];
		if (!p->len) continue;
		if (p->flags & I2C_M_RD)
			err = i2c_read(p->addr, p->buf, p->len);
		else
			err = i2c_write(p->addr, p->buf, p->len);
	}

	/* Return the number of messages processed, or the error code.
	*/
	if (err == 0)
		err = num;

	return err;
}

static u32 str8100_func(struct i2c_adapter *adapter)
{
/*    return I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE
		| I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA
		| I2C_FUNC_SMBUS_BLOCK_DATA;
*/
//    return I2C_FUNC_I2C;
	return I2C_FUNC_SMBUS_EMUL|I2C_FUNC_I2C | I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE
		| I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA
		| I2C_FUNC_SMBUS_BLOCK_DATA;
}

static int str8100_ioctl(struct i2c_adapter *adapter,unsigned int cmd, unsigned long arg)
{
	unsigned int s_msg;    
    
	if(debug)
		printk("===> %s: \n",__FUNCTION__);
	if (copy_from_user(&s_msg, (unsigned int *)arg, sizeof(unsigned int))) 
		return -EFAULT;
	if ((clock != s_msg) && s_msg>= I2C_100KHZ && s_msg<= I2C_400KHZ){
		clock=s_msg;
		str8100_i2c_init();
	}
	return 0;   
}


//MKL: adapter ====================================================================
static irqreturn_t str8100_i2c_int(int irq, void *private, struct pt_regs *regs)
{
	unsigned int volatile interrupt_status;	
    
	interrupt_status = *((u32 volatile *)I2C_INTERRUPT_STATUS_REG_ADDR);
	*((u32 volatile *)I2C_INTERRUPT_STATUS_REG_ADDR) = interrupt_status;
	i2c_cmd_transfer.action_done = (interrupt_status & I2C_ACTION_DONE_FLAG) ? 1 : 0;
	i2c_cmd_transfer.error_status = (interrupt_status & I2C_BUS_ERROR_FLAG) ? ((interrupt_status >> 8) & 0xFF) : 0;
	if(debug)
		printk("%s: i2c_cmd_transfer.error_status=0x%x\n",__FUNCTION__,i2c_cmd_transfer.error_status);
	if (i2c_cmd_transfer.error_status && (i2c_cmd_transfer.error_status != 0xFF))
	HAL_I2C_DISABLE_I2C();  
	wake_up_interruptible(&waitqueue);
	return IRQ_HANDLED;
}

#define I2C_HW_STR8100	0x1b0000
static struct i2c_algorithm str8100_algorithm = {
//    name:"str8100 i2c",
//    id:I2C_ALGO_SMBUS,
    master_xfer: str8100_xfer,
    algo_control: str8100_ioctl, 
    functionality: str8100_func,
};

static struct i2c_adapter str8100_i2c_adapter = {
	name:              "Str8100 i2c",
	id:                I2C_HW_STR8100,
	algo:              &str8100_algorithm,
};

enum {
	DEV_I2C_CLOCK=1,
	DEV_I2C_DEBUG,
	DEV_I2C_END
};

static ctl_table str8100_i2c_table[]={
	{	.ctl_name = DEV_I2C_CLOCK,
		.procname = "str8100_clock",
		.data=&clock,
		.maxlen=sizeof(clock),
		.mode = 0644,
		.proc_handler=&proc_dointvec
	},
	{	.ctl_name = DEV_I2C_DEBUG,
		.procname = "str8100_debug",
		.data=&debug,
		.maxlen=sizeof(debug),
		.mode = 0644,
		.proc_handler=&proc_dointvec
	},
	{}
};
#define DEV_I2C (DEV_IPMI+1)
static ctl_table i2c_dir_table[] = {
	{ .ctl_name	= DEV_I2C,
	  .procname	= "i2c",
	  .mode		= 0555,
	  .child	= str8100_i2c_table },
	{ }
};

static ctl_table i2c_root_table[] = {
	{ .ctl_name	= CTL_DEV,
	  .procname	= "dev",
	  .mode		= 0555,
	  .child	= i2c_dir_table },
	{ }
};
static struct ctl_table_header *i2c_table_header=NULL;

int  str8100_i2c_dev_init(void)
{
    
	int rc;

	printk(KERN_INFO "%s: i2c module version %s\n",__FUNCTION__, STR8100_I2C_VERSION); 

	init_waitqueue_head(&waitqueue);
	str8100_i2c_init();
	if ((rc = i2c_add_adapter(&str8100_i2c_adapter))) {
		printk(KERN_ERR "%s: Adapter %s registration failed\n",__FUNCTION__, str8100_i2c_adapter.name);
	}
	if (request_irq(INTC_I2C_BIT_INDEX, str8100_i2c_int, 0, "HS STR8100_I2C", NULL)) {
		printk("%s: unable to get IRQ %d\n",__FUNCTION__, INTC_I2C_BIT_INDEX);
		return -EAGAIN;
	}
	i2c_table_header = register_sysctl_table(i2c_root_table, 1);
	if(!i2c_table_header)
		printk("%s: unable register sysctl\n",__FUNCTION__);
	
	return rc; 
}

static __init int i2c_init(void) 
{
	if(debug)
		printk("%s: \n",__FUNCTION__);
	return str8100_i2c_dev_init();
}

static __exit void i2c_exit(void) 
{ 
	int rc;
	if(debug)
		printk("%s: \n",__FUNCTION__);
	if ((rc = i2c_del_adapter(&str8100_i2c_adapter))) printk(KERN_ERR "%s: i2c_del_adapter failed (%i), that's bad!\n",__FUNCTION__, rc);	
	unregister_sysctl_table(i2c_table_header);
	free_irq(INTC_I2C_BIT_INDEX,NULL);
	
}

module_init(i2c_init);
module_exit(i2c_exit);

MODULE_AUTHOR("Mac Lin");
MODULE_DESCRIPTION("I2C driver for Str8100");
MODULE_LICENSE("GPL");

