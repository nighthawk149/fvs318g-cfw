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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <asm/semaphore.h>
#include <asm/arch/star_spi.h>

static struct spi_device *le88221_spidev;
static struct semaphore le88221_lock;

/******************************************************************************
* Busy Tone
* Frequency: 480Hz + 620Hz
* Temporal Pattern: 0.5s on/ 0.5s off
* Event Reported After: 2 cycles of precise, 3 cycles of nonprecise
*******************************************************************************/
void le88221_busy_tone(struct spi_device *spi)
{
	u8 rx_data;
	u8 write_data;
	int retval;

	//enable channel 1 and channel 2
	write_data = 0x4A;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x03;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator 
	//0x00: all disabled
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator C and D    
	// 05 1F 1C C5 06 9D 1C C5
	//FreqC= 1311(0x051f) * 0.3662 = 480.0882 Hz
	//AmpC = 7365(0x1cc5) * 
	//FreqD= 1693(0x069d) * 0.3662 = 619.9766 Hz
	//AmpCD= 7365(0x1cc5) *
	write_data = 0xD4;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x05;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x1F;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x1C;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xC5;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x06;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x9D;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x1C;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xC5;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Cadence Timer
	// 00 64 00 64 (100ms On/ 100ms Off)
	write_data = 0xE0;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x64;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x64;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator
	//Eanble Signal generator cadencing, Signal generator C,D
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x8C;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set System State
	//Activate codec, Active Low battery
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x23;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
}

/******************************************************************************
* Dial Tone
* Frequency: 350Hz + 440Hz
* Temporal Pattern: Steady tone
* Event Reported After: Approximately 0.75 seconds
*******************************************************************************/
void spi_le88221_dial_tone(struct spi_device *spi)
{
	u8 rx_data;
	u8 write_data;
	int retval;

	//enable channel 1 and channel 2
	write_data = 0x4A;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x03;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator 
	//0x00: all disabled
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator C and D    
	// 03 BC 1C C5 04 B2 1C C5
	write_data = 0xD4;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x03;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xBC;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x1C;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xC5;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x04;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xB2;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x1C;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xC5;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Cadence Timer
	// 00 00 00 00
	write_data = 0xE0;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator
	//Eanble Signal generator cadencing, Signal generator C,D
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x8C;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set System State
	//Activate codec, Active Low battery
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x23;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
}

void le88221_balanced_ring(struct spi_device *spi)
{
	u8 rx_data;
	u8 write_data;
	int retval;
 
	//enable channel 1 and channel 2
	write_data = 0x4A;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x03;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator
	//0x00: all disabled
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Write Signal Generator A,B and Bias Param
	// 00 00 00 00 37 42 5B 00 00 00 00
	//0x00:Ramp has a positive slope, SG A,B out continuous, sinusoidal waves
	//Bias : 0 (0x0000) V
	//FreqA: 55(0x0037) * 0.3662 = 20.141 Hz 
	//AmpA : 16987(0x425B)
	//FreqB: 0(0x0000) * 0.3662 = 0 Hz
	//AmpB : 0(0x0000)
	write_data = 0xD2;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x37;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x42;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x5B;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set System State
	//Activate codec, Active Mid Battery
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x2B;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Loop Supervision Param
	// 1B 84 B3 05
	write_data = 0xC2;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x1B;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x84;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xB3;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x05;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set System state        
	//Deactivate codec, Balanced ringing
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x07;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
}

void le88221_unbalanced_ring(struct spi_device *spi)
{
	u8 rx_data;
	u8 write_data;
	int retval;

	//enable channel 1 and channel 2
	write_data = 0x4A;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x03;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator 
	//0x00: all disabled
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Cadence Timer
	//01 90 03 20 (400ms on/800ms off)
	write_data = 0xE0;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x01;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x90;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x03;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x20;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Signal generator 
	// Enable tone generator specified by EGA...
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x80;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Write Signal Generator A,B and Bias Param
	// 00 00 00 00 37 21 2D 00 00 00 00
	//0x00:Ramp has a positive slope, SG A,B out continuous, sinusoidal waves
	//Bias : 0(0x0000) V
	//FreqA: 55(0x0037) * 0.3662 = 20.141 Hz 
	//AmpA : 8493(0x212D)
	//FreqB: 0(0x0000) * 0.3662 = 0 Hz
	//AmpB : 0(0x0000)
	write_data = 0xD2;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x37;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x21;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x2D;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set System State
	//Activate codec, Active Mid Battery
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x2B;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set Loop Supervision Param
	// 1B 84 33 05
	write_data = 0xC2;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x1B;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x84;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x33;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x05;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);

	//Set System state        
	//Deactivate codec, Balanced ringing
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0x0A;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
}

u32 le88221_hook_off(struct spi_device *spi)
{
	u8 rx_data;
	u8 rx_data21, rx_data22;
	u8 write_data;
	int retval;

	/* if Hook1/2 off hook status at the same time */ 	        	    
	/*
	 * Check Hook1/2 On or Off (Read Signaling Register)
	 * Hook Switch, 0:On hook , 1:Off hook
	 */
	write_data = 0x4F;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data21, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &rx_data22, 1);

	return ((rx_data21 & 0x01) && (rx_data22 & 0x01));
}

static int le88221_init_hw(struct spi_device *spi)
{
#if 0
	int retval;
	u8 write_data;
	u8 read_data;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	if (retval < 0) {

	}
#endif

#if 0
	u8 tx_buf[] = { 0x00, 0x00, 0x00, 0x00 };
	u8 rx_buf[] = { 0x00, 0x00, 0x00, 0x00 };
	struct spi_transfer t[2];
	struct spi_message m;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = tx_buf;
	t[0].len = sizeof(tx_buf);
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = rx_buf;
	t[1].len = sizeof(rx_buf);
	spi_message_add_tail(&t[1], &m);

	down(&le88221_lock);
	spi_sync(flash->spi, &m);
	up(&le88221_lock);

#endif

	int retval;
	u8 write_data;
	u8 read_data;

	// Hardware Reset
	write_data = 0x04;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// 1ms delay
	udelay(1000);

	// I/O direction
	write_data = 0x54;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x02;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0x55;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif

	// Write Device Register(Define PCLK freq,ie 2.048MHz)
	write_data = 0x46;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x02;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	write_data = 0x47;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// Read Revision Code Number (RCN)
	write_data = 0x73;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	printk("Revision:0x%02x\n", read_data);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	printk("Product:0x%02x\n", read_data);

	/* 
	 * Write SRP register for Flyback power supply(VBL reference) 
	 * Note there should be voltages on VBH, VBL, VREF after the following codes are invoked.
	 */
	write_data = 0xE4;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x05;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x80;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0xE5;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif
    
	// Write SRC register
	write_data = 0xE6;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x07;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0xE7;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);   
#endif
    
    	// wait 100ms delay until switing regulator is stable
	mdelay(100);

	// System State Register
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#ifdef Print_Reg
	write_data = 0x57;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif    

	// I/O Data Register
	write_data = 0x52;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0x53;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif        

	/*
	 * For channel 1,2 operation
	 */
	write_data = 0x4A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x03;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0x4B;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif

	// Operating Functions
	write_data = 0x60;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x3F;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0x61;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif

	// Operating Conditions
	write_data = 0x70;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0x71;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif

	// Loop Supervision Parameters  19 88 A4 00 */
	write_data = 0xC2;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x19;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x88;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA4;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// DC Feed Parameters  2C 08
	write_data = 0xC6;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2C;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x08;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// Signal Generator A and B Parameters  00 04 25 00 37 3E C3 00 00 00 00
	write_data = 0xD2;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x04;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x25;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x37;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x3E;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xC3;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x44;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x68;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x35;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// Digital Impedance Scaling Network
	write_data = 0xCA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xEA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0xCB;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif
    
	// Z-Filter (FIR Only) Coefficients BA EB 2A 2C B5 25 AA 24 2C 3D
	write_data = 0x98;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xBA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xEB;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2C;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xB5;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x25;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xAA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x24;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2C;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x3D;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// Z-Filter (IIR Only) Coefficients  AA BA 27 9F 01
	write_data = 0x9A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xAA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xBA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x27;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x9F;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x01;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// R-Filter Coefficients  2D 01 2B B0 5A 33 24 5C 35 A4 5A 3D 33 B6
	write_data = 0x8A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2D;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x01;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2B;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xB0;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x5A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x33;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x24;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x5C;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x35;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA4;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x5A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x3D;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x33;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xB6;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// X-Filter Coefficients 3A 10 3D 3D B2 A7 6B A5 2A CE 2A 8F
	write_data = 0x88;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x3A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x10;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x3D;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xB2;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA7;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x6B;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA5;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xCE;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x8F;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// GR (Receive Gain A8 71
	write_data = 0x82;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA8;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x71;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// GX (Transmit Gain) A9 F0
	write_data = 0x80;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA9;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xF0;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// Voice Path Gains
	write_data = 0x50;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef PRINT_REG
	write_data = 0x51;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif

	// B1-Filter (FIR Only) Coefficients  2A 42 22 4B 1C A3 A8 FF 8F AA F5 9F BA F0
	write_data = 0x86;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x42;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x22;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x4B;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x1C;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA3;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xA8;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x8F;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xAA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xF5;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x9F;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xBA;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xF0;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// B2-Filter (IIR Only) Coefficients 2E 01
	write_data = 0x96;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x2E;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
 	write_data = 0x01;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);   

	//le88221_busy_tone(spi);
	//le88221_dial_tone(spi);
	//le88221_balanced_ring(spi);
	le88221_unbalanced_ring(spi);

#if 1
	while (!le88221_hook_off(spi))
		; // do nothing
#endif

#ifdef OPEN_CH1_2
	SPI_DEBUG("=== Configure channel 1...\n");
	/*
	 * For channel 1 operation, Active mode
	 */
	//Set Channel Enable and Operating Mode
	//Channel 1 enabled, Channel 2 disabled
	write_data = 0x4A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x01;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	//Set Signal generator
	//0x00: all disabled
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	//Set Operating Conditions
	//0x00 : all disabled
	write_data = 0x70;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	//Set System state
	//Activate codec, Active Low Battery
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x23;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	/*
	 * If you want loopback test , you can TX_Time_Slot 00, TX_Time_Slot 02
	 */
	//Set Transmit time slot
	write_data = 0x40;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

#ifdef LEGERITY_LOOPBACK_TEST
	//Set Receive time slot
	write_data = 0x42;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x02;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#else
	//Set Receive time slot
	write_data = 0x42;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif                                        

	//Read Register Status
	write_data = 0x4B;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	SPI_DEBUG("Read Register Status(0x4B): channel 1 %s, channel 2 %s\n",(rx_data&0x01)?"enabled":"disabled",(rx_data&0x02)?"enabled":"disabled");

	//Read System State Register
	write_data = 0x57;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	SPI_DEBUG("Read System State Register(0x57): REX=%d, METR=%d, Codec %s, POLNR=%d, SS=0x%x\n",(rx_data&(1<<7)),(rx_data&(1<<6)),(rx_data&(1<<5))?"enabled":"disabled",(rx_data&(1<<4)),(rx_data&0xF));
    
	//Read Transmit time slot
	write_data = 0x41;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	//Read Receive time slot
	write_data = 0x43;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	SPI_DEBUG("=== Configure channel 2...\n");
	/* 
	 * For channel 2 operation, Active mode 
	 */
	//Set Channel Enable and Operating Mode
	//Channel 1 disabled, Channel 2 enabled
	write_data = 0x4A;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x02;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	//Set Signal generator
	//0x00: all disabled
	write_data = 0xDE;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	//Set Operating Conditions
	//0x00 : all disabled
	write_data = 0x70;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	//Set System state
	//Activate codec, Active Low Battery
	write_data = 0x56;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x23;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	/*
	 * If you want loopback test , you can TX_Time_Slot 00, TX_Time_Slot 02 
	 */
#ifdef LEGERITY_LOOPBACK_TEST
	// Transmit time slot
	write_data = 0x40;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x02;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// Receive time slot
	write_data = 0x42;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x00;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#else
	// Transmit time slot
	write_data = 0x40;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x01;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);

	// Receive time slot
	write_data = 0x42;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0x01;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
#endif

	//Read Register Status
	write_data = 0x4B;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	SPI_DEBUG("Read Register Status(0x4B): channel 1 %s, channel 2 %s\n",(rx_data&0x01)?"enabled":"disabled",(rx_data&0x02)?"enabled":"disabled");

	//Read System State Register
	write_data = 0x57;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	SPI_DEBUG("Read System State Register(0x57): REX=%d, METR=%d, Codec %s, POLNR=%d, SS=0x%x\n",(rx_data&(1<<7)),(rx_data&(1<<6)),(rx_data&(1<<5))?"enabled":"disabled",(rx_data&(1<<4)),(rx_data&0xF));

	//Read Transmit time slot
	write_data = 0x41;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	SPI_DEBUG("Read Transmit time slot(0x41): Transmit time slot = 0x%02x\n",rx_data&0x7f);

	//Read Receive time slot
	write_data = 0x43;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	write_data = 0xFF;
	retval = spi_write_read_sync(spi, &write_data, 1, &read_data, 1);
	SPI_DEBUG("Read Receive time slot(0x43): Receive time slot = 0x%02x\n",rx_data&0x7f);
#endif      

	return 0;
}

void Pcm_Initial_Legerity_Le88221(void)
{
	le88221_init_hw(le88221_spidev);
}

static int __devinit le88221_probe(struct spi_device *spi)
{
	le88221_spidev = spi;
	init_MUTEX(&le88221_lock);
	return 0;
}

static int __devexit le88221_remove(struct spi_device *spi)
{
	printk("le88221_remove() enter\n");
	return 0;
}

static struct spi_driver le88221_driver = {
	.driver = {
		.name	= "le88221",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= le88221_probe,
	.remove	= __devexit_p(le88221_remove),
};

static int le88221_init(void)
{
	return spi_register_driver(&le88221_driver);
}

static void le88221_exit(void)
{
	spi_unregister_driver(&le88221_driver);
}

module_init(le88221_init);
module_exit(le88221_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Star Semi");
MODULE_DESCRIPTION("Legerity Le88221 driver");

