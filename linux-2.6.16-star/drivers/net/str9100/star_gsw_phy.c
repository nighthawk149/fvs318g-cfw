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


#include "star_gsw_phy.h"
#include "star_gsw.h"
#include "star_gsw_config.h"


void star_gsw_mdio_wreg(unsigned char page, unsigned char reg, u8 *data, unsigned char len);


#if 0
void init_switch()
{
        u32 sw_config;

	PDEBUG("init_switch\n");
        /*
         * Configure GSW configuration
         */
        sw_config = GSW_SWITCH_CONFIG;

#if 0
        // orignal virgon configuration
        // enable fast aging
        sw_config |= (0xF);

        // CRC stripping
        sw_config |= (0x1 << 21);

        // IVL learning
        sw_config |= (0x1 << 22);

        // HNAT disable
        sw_config &= ~(0x1 << 23);

        GSW_SWITCH_CONFIG = sw_config;

        sw_config = GSW_SWITCH_CONFIG;
#endif

        /* configure switch */
        sw_config = GSW_SWITCH_CONFIG;

        sw_config &= ~0xF;      /* disable aging */
        sw_config |= 0x1;       /* disable aging */

#ifdef JUMBO_ENABLE

        // CRC stripping and GSW_CFG_MAX_LEN_JMBO
        sw_config |= (GSW_CFG_CRC_STRP | GSW_CFG_MAX_LEN_JMBO);
#else
        // CRC stripping and 1536 bytes
        sw_config |= (GSW_CFG_CRC_STRP | GSW_CFG_MAX_LEN_1536);
#endif

        /* IVL */
        sw_config |= GSW_CFG_IVL;

        /* disable HNAT */
        sw_config &= ~GSW_CFG_HNAT_EN;

        GSW_SWITCH_CONFIG = sw_config;
}
#endif






// add by descent 2006/07/05
// for configure packet forward and rate control
void init_packet_forward(int port)
{
	u32 mac_port_config;

	PDEBUG("port%d configure\n", port);
	if (port==0)
	{
		mac_port_config = GSW_MAC_PORT_0_CONFIG;
	}
	else if (port==1)
	{
		mac_port_config = GSW_MAC_PORT_1_CONFIG;
	}
	else
	{
		printk("MAC port number %d is not support!", port);
		return;
	}
		
	if (STR9100_GSW_BROADCAST_RATE_CONTROL)
		mac_port_config |=  (0x1 << 31); // STR9100_GSW_BROADCAST_RATE_CONTROLL on
	else
		mac_port_config &=  (~(0x1 << 31)); // STR9100_GSW_BROADCAST_RATE_CONTROLL off

	if (STR9100_GSW_MULTICAST_RATE_CONTROL)
		mac_port_config |=  (0x1 << 30); // STR9100_GSW_MULTICAST_RATE_CONTROLL on
	else
		mac_port_config &=  (~(0x1 << 30)); // STR9100_GSW_MULTICAST_RATE_CONTROLL off

	if (STR9100_GSW_UNKNOW_PACKET_RATE_CONTROL)
		mac_port_config |=  (0x1 << 29); // STR9100_GSW_UNKNOW_PACKET_RATE_CONTROLL on
	else
		mac_port_config &=  (~(0x1 << 29)); // STR9100_GSW_UNKNOW_PACKET_RATE_CONTROLL off


	if (STR9100_GSW_DISABLE_FORWARDING_BROADCAST_PACKET)
		mac_port_config |=  (0x1 << 27); // STR9100_GSW_DISABLE_FORWARDING_BROADCAST_PACKET on
	else
		mac_port_config &=  (~(0x1 << 27)); // STR9100_GSW_DISABLE_FORWARDING_BROADCAST_PACKET off

	if(STR9100_GSW_DISABLE_FORWARDING_MULTICAST_PACKET)
	{
		mac_port_config |=  (0x1 << 26); // STR9100_GSW_DISABLE_FORWARDING_MULTICAST_PACKET on
		PDEBUG("STR9100_GSW_DISABLE_FORWARDING_MULTICAST_PACKET on\n");
	}
	else
	{
		mac_port_config &=  (~(0x1 << 26)); // STR9100_GSW_DISABLE_FORWARDING_MULTICAST_PACKET off
		PDEBUG("STR9100_GSW_DISABLE_FORWARDING_MULTICAST_PACKET off\n");
	}

	if(STR9100_GSW_DISABLE_FORWARDING_UNKNOW_PACKET)
		mac_port_config |=  (0x1 << 25); // STR9100_GSW_DISABLE_FORWARDING_UNKNOW_PACKET on
	else
		mac_port_config &=  (~(0x1 << 25)); // STR9100_GSW_DISABLE_FORWARDING_UNKNOW_PACKET off

	//GSW_MAC_PORT_0_CONFIG = mac_port_config;
	if (port==0)
		GSW_MAC_PORT_0_CONFIG = mac_port_config;
	if (port==1)
		GSW_MAC_PORT_1_CONFIG = mac_port_config;
}

static int star_gsw_set_phy_addr(u8 mac_port, u8 phy_addr)
{
	u32 status = 0;	/* for failure indication */

	if ((mac_port > 1) || (phy_addr > 31)) {
		return status;
	}

	if (mac_port == 0) {
		GSW_PORT_MIRROR &= ~(0x3 << 0); /* clear bit[1:0] for PHY_ADDR[1:0] */
		GSW_PORT_MIRROR &= ~(0x3 << 4); /* clear bit[5:4] for PHY_ADDR[3:2] */
		GSW_QUEUE_STATUS_TEST_1 &= ~(0x1 << 25); /* clear bit[25] for PHY_ADDR[4] */
		GSW_PORT_MIRROR |= (((phy_addr >> 0) & 0x3) << 0);
		GSW_PORT_MIRROR |= (((phy_addr >> 2) & 0x3) << 4);
		GSW_QUEUE_STATUS_TEST_1 |= (((phy_addr >> 4) & 0x1) << 25);
		status = 1; /* for ok indication */
	} else if (mac_port == 1) {
		GSW_PORT_MIRROR &= ~(0x1 << 6); /* clear bit[6] for PHY_ADDR[0] */
		GSW_PORT_MIRROR &= ~(0x7 << 8); /* clear bit[10:8] for PHY_ADDR[3:1] */
		GSW_QUEUE_STATUS_TEST_1 &= ~(0x1 << 26); /* clear bit[26] for PHY_ADDR[4] */
		GSW_PORT_MIRROR |= (((phy_addr >> 0) & 0x1) << 6);
		GSW_PORT_MIRROR |= (((phy_addr >> 1) & 0x7) << 8);
		GSW_QUEUE_STATUS_TEST_1 |= (((phy_addr >> 4) & 0x1) << 26);
		status = 1; /* for ok indication */
	}

	return status;
}

int star_gsw_read_phy(u8 phy_addr, u8 phy_reg, u16 volatile *read_data)
{
	u32 status;
	int i;

	// clear previous rw_ok status
	GSW_PHY_CONTROL = (0x1 << 15);

        // 20061013 descent 
	// for ORION EOC
        GSW_QUEUE_STATUS_TEST_1 &= ~( 0XF << 16);

        GSW_PHY_CONTROL   &= ~(0x1<<0);

        GSW_QUEUE_STATUS_TEST_1 |= (((phy_addr >> 1) & 0xF) << 16);
        // 20061013 descent end


	GSW_PHY_CONTROL = ((phy_addr & 0x1) | ((phy_reg & 0x1F) << 8) | (0x1 << 14));

	for (i = 0; i < 0x1000; i++) {
		status = GSW_PHY_CONTROL;
		if (status & (0x1 << 15)) {
			// clear the rw_ok status, and clear other bits value
			GSW_PHY_CONTROL = (0x1 << 15);
			*read_data = (u16) ((status >> 16) & 0xFFFF);
			return (1);
		} else {
			udelay(10);
		}
	}

	return (0);
}

int star_gsw_write_phy(u8 phy_addr, u8 phy_reg, u16 write_data)
{
	int i;

	// clear previous rw_ok status
	GSW_PHY_CONTROL = (0x1 << 15);

        // 20061013 descent 
	// for ORION EOC
        GSW_QUEUE_STATUS_TEST_1 &= ~( 0XF << 16);

        GSW_PHY_CONTROL   &= ~(0x1<<0);

        GSW_QUEUE_STATUS_TEST_1 |= (((phy_addr >> 1) & 0xF) << 16);
        // 20061013 descent end


	GSW_PHY_CONTROL = ((phy_addr & 0x1) |
		((phy_reg & 0x1F) << 8) |
		(0x1 << 13) | ((write_data & 0xFFFF) << 16));

	for (i = 0; i < 0x1000; i++) {
		if ((GSW_PHY_CONTROL) & (0x1 << 15)) {
			// clear the rw_ok status, and clear other bits value
			GSW_PHY_CONTROL = (0x1 << 15);
			return (1);
		} else {
			udelay(10);
		}
	}

	return (0);
}

#ifdef DORADO2_PCI_FASTPATH_MAC_PORT1_LOOPBACK
static int star_gsw_config_mac_port1_loopback(void)
{
	u32 mac_port_base;
	u32 mac_port_config;
	int i;

	PRINT_INFO("\nconfigure mac port1 loopback\n");
	
	mac_port_base = GSW_PORT1_CFG_REG;

	mac_port_config = __REG(mac_port_base);

	// disable PHY's AN
	mac_port_config &= ~(0x1 << 7);

	// enable RGMII-PHY mode
	mac_port_config |= (0x1 << 15);

	// reversed RGMII mode
	mac_port_config |= (0x1 << 14);

	// enable GSW MAC port 0
	mac_port_config &= ~(0x1 << 18);

	__REG(mac_port_base) = mac_port_config;

	// SA learning disable
	mac_port_config |= (0x1 << 19);

	// disable TX flow control
	mac_port_config &= ~(0x1 << 12);

	// disable RX flow control
	mac_port_config &= ~(0x1 << 11);

	// force duplex
	mac_port_config |= (0x1 << 10);

	// force speed at 1000Mbps
	mac_port_config &= ~(0x3 << 8);
	mac_port_config |= (0x2 << 8);

	__REG(mac_port_base) = mac_port_config;

	// adjust MAC port 1 RX/TX clock skew
	GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 28) | (0x3 << 30));
	//GSW_BIST_RESULT_TEST_0 |= ((0x2 << 28) | (0x2 << 30));
	GSW_BIST_RESULT_TEST_0 |= (0x2 << 30);

	return 0;
}
#endif

int VSC8201_phy_power_down(int phy_addr, int y)
{
	u16 phy_data = 0;
	// power-down or up the PHY
	star_gsw_read_phy(phy_addr, 0, &phy_data);
	if (y==1) // down
		phy_data |= (0x1 << 11);
	if (y==0) // up
		phy_data |= (~(0x1 << 11));
	star_gsw_write_phy(phy_addr, 0, phy_data);
	return 0;

}

static int star_gsw_config_VSC8201(u8 mac_port, u8 phy_addr)	// include cicada 8201
{
	u32 mac_port_base = 0;
	u32 mac_port_config;
	u16 phy_reg;
	int i;

	PRINT_INFO("\nconfigure VSC8201\n");
	PDEBUG("mac port : %d phy addr : %d\n", mac_port, phy_addr);
	/*
	 * Configure MAC port 0
	 * For Cicada CIS8201 single PHY
	 */
	if (mac_port == 0) {
		PDEBUG("port 0\n");
		mac_port_base = GSW_PORT0_CFG_REG;
	}
	if (mac_port == 1) {
		PDEBUG("port 1\n");
		mac_port_base = GSW_PORT1_CFG_REG;
	}

	star_gsw_set_phy_addr(mac_port, phy_addr);
	//star_gsw_set_phy_addr(1, 1);

	mac_port_config = __REG(mac_port_base);

	// enable PHY's AN
	mac_port_config |= (0x1 << 7);

	// enable RGMII-PHY mode
	mac_port_config |= (0x1 << 15);

	// enable GSW MAC port 0
	mac_port_config &= ~(0x1 << 18);

	__REG(mac_port_base)=  mac_port_config;

	/*
	 * Configure Cicada's CIS8201 single PHY
	 */
	/* 2007.04.24 Richard.Liu Marked, we don't need VSC8201 lookback mode */
#if 0
#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
	/* near-end loopback mode */
	star_gsw_read_phy(phy_addr, 0x0, &phy_reg);
	phy_reg |= (0x1 << 14);
	star_gsw_write_phy(phy_addr, 0x0, phy_reg);
#endif
#endif

	star_gsw_read_phy(phy_addr, 0x1C, &phy_reg);

	// configure SMI registers have higher priority over MODE/FRC_DPLX, and ANEG_DIS pins
	phy_reg |= (0x1 << 2);

	star_gsw_write_phy(phy_addr, 0x1C, phy_reg);

	star_gsw_read_phy(phy_addr, 0x17, &phy_reg);

	// enable RGMII MAC interface mode
	phy_reg &= ~(0xF << 12);
	phy_reg |= (0x1 << 12);

	// enable RGMII I/O pins operating from 2.5V supply
	phy_reg &= ~(0x7 << 9);
	phy_reg |= (0x1 << 9);

	star_gsw_write_phy(phy_addr, 0x17, phy_reg);

	star_gsw_read_phy(phy_addr, 0x4, &phy_reg);

	// Enable symmetric Pause capable
	phy_reg |= (0x1 << 10);

	star_gsw_write_phy(phy_addr, 0x4, phy_reg);

	mac_port_config = __REG(mac_port_base);

	/* 2007.04.24 Richard.Liu Marked, we don't need VSC8201 lookback mode */
//#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
#if 0
	// near-end loopback mode, must disable AN
	mac_port_config &= ~(0x1 << 7);

	// SA learning disable
	mac_port_config |= (0x1 << 19);

	// disable TX flow control
	mac_port_config &= ~(0x1 << 12);

	// disable RX flow control
	mac_port_config &= ~(0x1 << 11);

	// force duplex
	mac_port_config |= (0x1 << 10);

	// force speed at 1000Mpbs
	mac_port_config &= ~(0x3 << 8);
	mac_port_config |= (0x2 << 8);
#else
	// enable PHY's AN
	mac_port_config |= (0x1 << 7);
#endif

	__REG(mac_port_base) = mac_port_config;

	/*
	 * Enable PHY1 AN restart bit to restart PHY1 AN
	 */
	star_gsw_read_phy(phy_addr, 0x0, &phy_reg);

	phy_reg |= (0x1 << 9) | (0x1 << 12);

	star_gsw_write_phy(phy_addr, 0x0, phy_reg);

	/*
	 * Polling until PHY0 AN restart is complete
	 */
	for (i = 0; i < 0x1000; i++) {
		star_gsw_read_phy(phy_addr, 0x1, &phy_reg);

		if ((phy_reg & (0x1 << 5)) && (phy_reg & (0x1 << 2))) {
			printk("0x1 phy reg: %x\n", phy_reg);
			break;
		} else {
			udelay(100);
		}
	}

	mac_port_config = __REG(mac_port_base);

	if (((mac_port_config & 0x1) == 0) || (mac_port_config & 0x2)) {
		printk("Check MAC/PHY%s Link Status : DOWN!\n", (mac_port == 0 ? "0" : "1"));
	} else {
		printk("Check MAC/PHY%s Link Status : UP!\n", (mac_port == 0 ? "0" : "1"));
		/*
		 * There is a bug for CIS8201 PHY operating at 10H mode, and we use the following
		 * code segment to work-around
		 */
		star_gsw_read_phy(phy_addr, 0x05, &phy_reg);

		if ((phy_reg & (0x1 << 5)) && (!(phy_reg & (0x1 << 6))) && (!(phy_reg & (0x1 << 7))) && (!(phy_reg & (0x1 << 8)))) {	/* 10H,10F/100F/100H off */
			star_gsw_read_phy(phy_addr, 0x0a, &phy_reg);

			if ((!(phy_reg & (0x1 << 10))) && (!(phy_reg & (0x1 << 11)))) {	/* 1000F/1000H off */
				star_gsw_read_phy(phy_addr, 0x16, &phy_reg);

				phy_reg |= (0x1 << 13) | (0x1 << 15);	// disable "Link integrity check(B13)" & "Echo mode(B15)"

				star_gsw_write_phy(phy_addr, 0x16, phy_reg);
			}
		}
	}

	if (mac_port == 0) {
		// adjust MAC port 0 RX/TX clock skew
		GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 24) | (0x3 << 26));
		GSW_BIST_RESULT_TEST_0 |= ((0x2 << 24) | (0x2 << 26));
	}

	if (mac_port == 1) {
		// adjust MAC port 1 RX/TX clock skew
		GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 28) | (0x3 << 30));
		GSW_BIST_RESULT_TEST_0 |= ((0x2 << 28) | (0x2 << 30));
	}

	return 0;
}


static int star_gsw_config_VSC8601(u8 mac_port, u8 phy_addr)	
{
	u32 mac_port_base = 0;
        u32 mac_port_config;
        u16 phy_data;
	u16 ancount = 0;
	
	printk("INIT VSC8601\n");

	/* reset phy */
	star_gsw_read_phy(phy_addr, 0, &phy_data);
	phy_data |= (0x1 << 15);
	star_gsw_write_phy(phy_addr, 0, phy_data);
	udelay(10);
	star_gsw_write_phy(phy_addr, 31, 0x0000);		// select main register to avoid SMI write fail

        if (mac_port == 0) {
		PDEBUG("port 0\n");
		mac_port_base = GSW_PORT0_CFG_REG;
	}
	if (mac_port == 1) {
	        PDEBUG("port 1\n");
	        mac_port_base = GSW_PORT1_CFG_REG;
	}
        star_gsw_set_phy_addr(mac_port, phy_addr);
				
        mac_port_config = __REG(mac_port_base);

        // enable PHY's AN
        mac_port_config |= (0x1 << 7);

        // enable RGMII-PHY mode
        mac_port_config |= (0x1 << 15);

        // enable GSW MAC port 0
        mac_port_config &= ~(0x1 << 18);

        __REG(mac_port_base)=  mac_port_config;
	udelay(1000);
	
        star_gsw_read_phy(phy_addr, 3, &phy_data);
        if ((phy_data & 0x000F) == 0x0000) {
	        u16 tmp16;
		printk("VSC8601 Type A Chip\n");
		star_gsw_write_phy(phy_addr, 31, 0x52B5);
		star_gsw_write_phy(phy_addr, 16, 0xAF8A);

		phy_data = 0x0;
		star_gsw_read_phy(phy_addr, 18, &tmp16);
		phy_data |= (tmp16 & ~0x0);
		star_gsw_write_phy(phy_addr, 18, phy_data);
                phy_data = 0x0008;
                star_gsw_read_phy(phy_addr, 17, &tmp16);
                phy_data |= (tmp16 & ~0x000C);
                star_gsw_write_phy(phy_addr, 17, phy_data);
                star_gsw_write_phy(phy_addr, 16, 0x8F8A);
                star_gsw_write_phy(phy_addr, 16, 0xAF86);
                phy_data = 0x0008;
                star_gsw_read_phy(phy_addr, 18, &tmp16);
		phy_data |= (tmp16 & ~0x000C);
		star_gsw_write_phy(phy_addr, 18, phy_data);
                phy_data = 0x0;
                star_gsw_read_phy(phy_addr, 17, &tmp16);
                phy_data |= (tmp16 & ~0x0);
                star_gsw_write_phy(phy_addr, 17, phy_data);

                star_gsw_write_phy(phy_addr, 16, 0x8F8A);

                star_gsw_write_phy(phy_addr, 16, 0xAF82);

                phy_data = 0x0;
                star_gsw_read_phy(phy_addr, 18, &tmp16);
                phy_data |= (tmp16 & ~0x0);
                star_gsw_write_phy(phy_addr, 18, phy_data);

                phy_data = 0x0100;
                star_gsw_read_phy(phy_addr, 17, &tmp16);
                phy_data |= (tmp16 & ~0x0180);
                star_gsw_write_phy(phy_addr, 17, phy_data);

                star_gsw_write_phy(phy_addr, 16, 0x8F82);

                star_gsw_write_phy(phy_addr, 31, 0x0);

                //Set port type: single port
		star_gsw_read_phy(phy_addr, 9, &phy_data);
		phy_data &= ~( 0x1 << 10);
		star_gsw_write_phy(phy_addr, 9, phy_data);
	} else if ((phy_data & 0x000F) == 0x0001) {
		printk("VSC8601 Type B Chip\n");
		star_gsw_read_phy(phy_addr, 23, &phy_data);
		phy_data |= ( 0x1 << 8); //set RGMII timing skew
		star_gsw_write_phy(phy_addr, 23, phy_data);
	}

    /*
     * Enable full-duplex mode
     */
    star_gsw_read_phy(phy_addr, 0, &phy_data);
    phy_data |= (0x1 << 8);
    star_gsw_write_phy(phy_addr, 0, phy_data);

        star_gsw_write_phy(phy_addr, 31, 0x0001); // change to extended registers
        star_gsw_read_phy(phy_addr, 28, &phy_data);
        phy_data &= ~(0x3 << 14); // RGMII TX timing skew
        //phy_data |=  (0x0 << 14); // 2.0ns
        phy_data &= ~(0x3 << 12); // RGMII RX timing skew
        //phy_data |=  (0x0 << 12); // 0.0ns
        star_gsw_write_phy(phy_addr, 28, phy_data);
        star_gsw_write_phy(phy_addr, 31, 0x0000); // change to normal registers

        mac_port_config = __REG(mac_port_base);
        mac_port_config |= (0x1 << 7); // enable phy's AN
        __REG(mac_port_base) = mac_port_config;

        star_gsw_read_phy(phy_addr, 4, &phy_data);
        phy_data |= (0x1 << 10); // enable flow control (Symmetric PAUSE frame)
        star_gsw_write_phy(phy_addr, 4, phy_data);

        star_gsw_read_phy(phy_addr, 0, &phy_data);
        phy_data |= (0x1 << 9) | (0x1 << 12); // restart phy's AN
        star_gsw_write_phy(phy_addr, 0, phy_data);

	/* wait for an complete */
	while (1)
	{
		ancount++;
		// check LP ack
		if (ancount > 12000) 
		{
			star_gsw_read_phy(phy_addr, 5, &phy_data);
			if ( !(phy_data & (0x1 << 14))) {
				printk("VSC8601: Ack time out.\n");		
				break;
			}
		}
		// check if AN is completed
        star_gsw_read_phy(phy_addr, 1, &phy_data);
		if (phy_data & (0x1 << 5)) {
			printk("VSC8601: AN Completed.\n");
			break;
		}
	}

        mac_port_config = __REG(mac_port_base);
        //mac_port_config &= ~(0x1 << 18); // enable mac port 1
        mac_port_config |= (0x1 << 18); // disable mac port 1
        //mac_port_config |= (0x1 << 19); // disable SA learning
	mac_port_config &= ~(0x1 << 19); // enable SA learning
	mac_port_config &= ~(0x1 << 24); // disable ingress check
	// forward unknown, multicast and broadcast packets to CPU
	mac_port_config &= ~((0x1 << 25) | (0x1 << 26) | (0x1 << 27));
	// storm rate control for unknown, multicast and broadcast packets
	//mac_port_config |= ((0x1 << 29) | (0x1 << 30) | ((u32)0x1 << 31));
	__REG(mac_port_base) = mac_port_config;
	
	if (mac_port == 0) {
		// adjust MAC port 0 RX/TX clock skew
		GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 24) | (0x3 << 26));
		GSW_BIST_RESULT_TEST_0 |= ((0x3 << 24) | (0x3 << 26));
	}

	if (mac_port == 1) {
		// adjust MAC port 1 RX/TX clock skew
		GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 28) | (0x3 << 30));
		GSW_BIST_RESULT_TEST_0 |= ((0x3 << 28) | (0x3 << 30));
	}

	return 0;                                                                                
}

static void star_gsw_config_bcm5081()
{

     u32 mac_port_config;
     u16 link_status;
     int speed,duplex;
     int ii;
     u32 mac_port_base = 0;
     
     /*
      * Configure GSW's MAC port 1
      */
     
     
     /*Get it some time for the auto neg. complete */
     /* wait for PHY auto neg. complete */
     for (ii = 0; ii < 0x20; ii++)
     {
	  star_gsw_read_phy(9, 0x1, &link_status);
	  
	  if ((link_status & (0x1 << 2)) && (link_status & (0x1 << 5)))
	  {
	       break;
	  }
	  
     }
     
     
     /*The following code will tell the link status, but this will not work 
       if the link is on sometime later than in the init. stage*/
     star_gsw_read_phy(9, 0x19, &link_status);
     
     speed = (link_status >> 9) & 0x3;
     
     duplex = (link_status >> 8) & 0x1;
     
     speed -= 1;
     
     
     printk("speed is %x, duplex is %x\n", speed, duplex);
     
     
     mac_port_base = GSW_PORT1_CFG_REG;
     
     
     
     star_gsw_write_phy(9, 0x0, 0x1140);
     
     udelay(1000);
     
     star_gsw_write_phy(9, 0x1c,0x8804);
     
     udelay(1000);

     star_gsw_write_phy(9, 0x1c, 0xa414);
      
      udelay(1000);
     
     star_gsw_write_phy(9, 0x1c, 0xb833);

     udelay(1000);
     
     
     mac_port_config = __REG(mac_port_base);
     
     // enable RGMII-PHY mode
     mac_port_config |= (0x1 << 15);
     
     // enable GSW MAC port 0
     mac_port_config &= ~(0x1 << 18);
     
     // force speed = 1000Mbps
     mac_port_config &= ~(0x3 << 8);
     mac_port_config |= (2 << 8);
     
     // force full-duplex
     mac_port_config |= (1 << 10);
     
     // force Tx/Rx flow-control on
     mac_port_config |= (0x1 << 11) | (0x1 << 12);

     __REG(mac_port_base) = mac_port_config;
     
     
     udelay(1000);
     
     star_gsw_set_phy_addr(1, 8);

     mac_port_config = __REG(mac_port_base);
     
     if (((mac_port_config & 0x1) == 0) || (mac_port_config & 0x2)) {
	  printk("Check MAC/PHY 1 Link Status : DOWN!\n");
     } else {
	  printk("Check MAC/PHY 1 Link Status : UP!\n");
     }
     

     
     // enable MAC port 1
     mac_port_config &= ~(0x1 << 18);
     
     // disable SA learning
     mac_port_config |= (0x1 << 19);
     
     // forward unknown, multicast and broadcast packets to CPU
     mac_port_config &= ~((0x1 << 25) | (0x1 << 26) | (0x1 << 27));
     
     // storm rate control for unknown, multicast and broadcast packets
     mac_port_config |= (0x1 << 29) | (0x1 << 30) | ((u32)0x1 << 31);
     
     __REG(mac_port_base) = mac_port_config;
     
     
     // adjust MAC port 1 RX/TX clock skew
     GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 28) | (0x3 << 30));
     GSW_BIST_RESULT_TEST_0 |= ((0x2 << 28) | (0x2 << 30));
     
}


#ifdef CONFIG_DORADO2
static void star_gsw_config_VSC8X01()
{
       	u16  phy_id = 0;

#ifdef CONFIG_DORADO2
	star_gsw_set_phy_addr(1,1);
	star_gsw_read_phy(1, 0x02, &phy_id);
//	printk("phy id = %X\n", phy_id);
	if (phy_id == 0x000F) //VSC8201
	       	star_gsw_config_VSC8201(1,1);
	else
		star_gsw_config_VSC8601(1,1);
#else
#ifdef CONFIG_LEO
	star_gsw_set_phy_addr(0,0);
	star_gsw_read_phy(0, 0x02, &phy_id);
//	printk("phy id = %X\n", phy_id);
	if (phy_id == 0x000F) //VSC8201
	       	star_gsw_config_VSC8201(0,0);
	else
		star_gsw_config_VSC8601(0,0);
#endif
#endif
	
}
#endif
#define REG_PSEUDO_PHY_REG16                0x10
#define REG_PSEUDO_PHY_REG17                0x11
#define REG_PSEUDO_PHY_REG24                0x18
#define REG_PSEUDO_PHY_REG25                0x19
#define REG_PSEUDO_PHY_REG26                0x1a
#define REG_PSEUDO_PHY_REG27                0x1b

#define PSEUDO_PHY_ADDR                     0x1e

#define REG_REG16_MDIO_ENABLE               0x01
#define REG_REG17_OP_READ                   0x02
#define REG_REG17_OP_WRITE                  0x01
#define REG_REG17_OP_DONE                   0x00

#define REG_REG16_PAGE_NUMBER_SHIFT         8
#define REG_REG17_REG_NUMBER_SHIFT          8

void star_gsw_mdio_rreg(unsigned char page, unsigned char reg, u8 *data, unsigned char len)
{
    u16 cmd, read_data;
    u16 retry = 0;
    u64 ret = 0 ;
    unsigned char * p = (unsigned char *)&ret;

    cmd = (page << REG_REG16_PAGE_NUMBER_SHIFT) | (REG_REG16_MDIO_ENABLE) ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG16, cmd) ;

    cmd = (reg << REG_REG17_REG_NUMBER_SHIFT) | (REG_REG17_OP_READ) ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG17, cmd) ;

    do
    {
        star_gsw_read_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG17, &read_data) ;
        udelay(10) ;
    }
    while(((read_data & (REG_REG17_OP_WRITE|REG_REG17_OP_READ)) != REG_REG17_OP_DONE) && (retry++ < 5)) ;

    read_data = 0 ;
    star_gsw_read_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG24, &read_data) ;
    memcpy(p,(u8 *)&read_data,2*sizeof(unsigned char));

    read_data = 0 ;
    star_gsw_read_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG25, &read_data) ;
    memcpy(p+2,(u8 *)&read_data,2*sizeof(unsigned char));

    read_data = 0;
    star_gsw_read_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG26, &read_data) ;
    memcpy(p+4,(u8 *)&read_data,2*sizeof(unsigned char));

    read_data = 0;
    star_gsw_read_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG27, &read_data) ;
    memcpy(p+6,(u8 *)&read_data,2*sizeof(unsigned char));

    switch(len)
    {
        case 1:
            *data = (u8)ret ;
            break ;
        case 2:
            *(u16 *)data = (u16)ret ;
            break ;
        case 4:
        default:
            *(u32 *)data = (u32)ret ;
            break ;
        case 6:
            memcpy(data, &ret, 6*sizeof(unsigned char));
            break;
        case 8:
            //*(u64 *)data = (u64)ret ;
            memcpy(data, &ret, sizeof(unsigned long long));
            break;
    }
}

void star_gsw_mdio_wreg(unsigned char page, unsigned char reg, u8 *data, unsigned char len)
{
    u16 cmd, read_data;
    u16 retry = 0;
    u64 write_data = 0;
    switch(len)
    {
        case 1:
            write_data = *data ;
            break ;
        case 2:
            write_data = *(u16 *)data ;
            break ;
        case 4:
        default:
            write_data = *(u32 *)data ;
            break;
        case 6:
            memcpy(&write_data, data, 6*sizeof(unsigned char));
            break;
        case 8:
            write_data = *(u64 *)data ;
            break;
    }

    cmd = (page << REG_REG16_PAGE_NUMBER_SHIFT) | (REG_REG16_MDIO_ENABLE) ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG16, cmd) ;

    cmd = (write_data >> 0) & 0xffff ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG24, cmd) ;

    cmd = (write_data >> 16) & 0xffff ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG25, cmd) ;

    cmd = (write_data >> 32) & 0xffff ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG26, cmd) ;

    cmd = (write_data >> 48) & 0xffff ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG27, cmd) ;

    cmd = (reg << REG_REG17_REG_NUMBER_SHIFT) | (REG_REG17_OP_WRITE) ;
    star_gsw_write_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG17, cmd) ;

    do
    {
        star_gsw_read_phy(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_REG17, &read_data) ;
        udelay(10) ;
    }
    while(((read_data & (REG_REG17_OP_WRITE|REG_REG17_OP_READ)) != REG_REG17_OP_DONE) && (retry++ < 5)) ;
}

// add by descent 2006/07/10
// port : 0 => port0 ; port : 1 => port1
// y = 1 ; disable AN
int disable_AN(int port, int y)
{
	u32 mac_port_config;
	if (port==0)
	{
		mac_port_config = GSW_MAC_PORT_0_CONFIG;
	}
	else if (port==1)
	{
		mac_port_config = GSW_MAC_PORT_1_CONFIG;
	}
	else
	{
		printk("MAC port number %d is not support!", port);
		return (1);
	}

	// disable PHY's AN
	if (y==1)
	{
	  PDEBUG("disable AN\n");
	  mac_port_config &= ~(0x1 << 7);
	}

	// enable PHY's AN
	if (y==0)
	{
	  PDEBUG("enable AN\n");
	  mac_port_config |= (0x1 << 7);
	}

	if (port==0)
		GSW_MAC_PORT_0_CONFIG = mac_port_config;
	if (port==1)
		GSW_MAC_PORT_1_CONFIG = mac_port_config;
	return 0;
}

int disable_AN_VSC7385(int y)
{
	u32 mac_port_config;
	mac_port_config = GSW_MAC_PORT_0_CONFIG;

	// disable PHY's AN
	if (y==1)
	{
	  PDEBUG("disable AN\n");
	  mac_port_config &= ~(0x1 << 7);
	}

	// enable PHY's AN
	if (y==0)
	{
	  PDEBUG("enable AN\n");
	  mac_port_config |= (0x1 << 7);
	}

	GSW_MAC_PORT_0_CONFIG = mac_port_config;
	return 0;
}

void star_gsw_mdio_rreg(unsigned char page, unsigned char reg, u8 *data, unsigned char len);

void star_gsw_config_bcm53118(void)
{
     u32 mac_port_config;
     u8 i;
     u32 data;


     printk("in star_gsw_config_BCM53118 \n");

     /*
      * phy addr define : bcm53118(0~7) and bcm5081 (9)
      * set MAC0 auto-polling phy addr
      * there is no 8st phy, a unmeaning data,
      * polling null phy addr
      */
     star_gsw_set_phy_addr(0, 8);
     /*
      * Configure GSW's MAC port 0
      * For ASIX's 5-port GbE Switch setting
      * 1. No SMI (MDC/MDIO) connection between Orion's MAC port 0 and ASIX's MAC port 4
      * 2. Force Orion's MAC port 0 to be 1000Mbps, and full-duplex, and flow control on
      */
     mac_port_config = GSW_MAC_PORT_0_CONFIG;

     // disable PHY's AN
     mac_port_config &= ~(0x1 << 7);

     // enable RGMII-PHY mode
     mac_port_config |= (0x1 << 15);

     // force speed = 1000Mbps
     mac_port_config &= ~(0x3 << 8);
     mac_port_config |= (0x2 << 8);

     // force full-duplex
     mac_port_config |= (0x1 << 10);

     // force Tx/Rx flow-control on
     mac_port_config |= (0x1 << 11) | (0x1 << 12);

     GSW_MAC_PORT_0_CONFIG = mac_port_config;

     udelay(1000);

     mac_port_config = GSW_MAC_PORT_0_CONFIG;

     if (((mac_port_config & 0x1) == 0) || (mac_port_config & 0x2)) {
	  printk("Check MAC/PHY 0 Link Status : DOWN!\n");
     } else {
	  printk("Check MAC/PHY 0 Link Status : UP!\n");
     }

     /* adjust MAC port 0 /RX/TX clock skew */
//     GSW_BIST_RESULT_TEST_0 |= ((0x2 << 24) | (0x2 << 26)); //enable CPU TX/RX delay
     GSW_BIST_RESULT_TEST_0 |= (0x2 << 26); // enable CPU-->53118 TX delay 2.0ns
     /* adjust 53118 cpu port rx/tx delay (2ns) */
     star_gsw_mdio_rreg(0x0, 0x60, (char *)&data, 1);
//     data |= 0x3; //enable 53118 TX, RX delay
     data |= 0x1; // enable switch TX delay (53118 --> CPU)
     star_gsw_mdio_wreg(0x0, 0x60, (char *)&data, 1);
   

    /*to read bcm53118's version*/
    printk("Init for IEEE802.3 test\n");
    memset(&data,0,sizeof(data));
    star_gsw_mdio_rreg(0x2, 0x40, (char *)&data, 1);
    /*bcm53118 Rev.B0*/
    if (data == 4)
    {
        printk("bcm53118 Rev.B0\n");
        for (i=0;i<8;i++)
        {
            data = 0x0f75;
            star_gsw_write_phy(i,0x17,data);
            data = 0x0029;
            star_gsw_write_phy(i,0x15,data);
            data = 0x0f76;
            star_gsw_write_phy(i,0x17,data);
            data = 0x0100;
            star_gsw_write_phy(i,0x15,data);
            data = 0x0f74;
            star_gsw_write_phy(i,0x17,data);
            data = 0xd087;
            star_gsw_write_phy(i,0x15,data);
            data = 0x0c00;
            star_gsw_write_phy(i,0x18,data);
            data = 0x601f;
            star_gsw_write_phy(i,0x17,data);
            data = 0x0110;
            star_gsw_write_phy(i,0x15,data);
        }
    }
    else if (data == 5)/*bcm53118 Rev.B1*/
    {
        printk("bcm53118 Rev.B1\n");
        for (i=0;i<8;i++)
        {
            data = 0x0f76;
            star_gsw_write_phy(i,0x17,data);
            data = 0x0040;
            star_gsw_write_phy(i,0x15,data);
        }
    }

    printk("Set bcm53118 to unManaged mode\n");
    memset(&data,0,sizeof(data));
    star_gsw_mdio_rreg(0x0, 0xb, (char *)&data, 1);
    /*set bit[1] to 1*/
    data &= ~(1<<0);
    data |= (1<<1);
    star_gsw_mdio_wreg(0x0, 0xb, (char *)&data, 1);

    for (i=0;i<8;i++)
    {
        data = 0;
        star_gsw_mdio_wreg(0, i, (char *)&data, 1);
    }
}



void star_gsw_config_ASIX()
{
	u32 mac_port_config;

	printk("configure port0 ASIX\n");
	mac_port_config = GSW_MAC_PORT_0_CONFIG;

	//Disable AN
	mac_port_config &= (~(0x1 << 7));

	//force speed to 1000Mbps
	mac_port_config &= (~(0x3 << 8));
	mac_port_config |= (0x2 << 8);	//jacky

	//force tx and rx follow control
	mac_port_config |= (0x1 << 11) | (0x1 << 12);

	//force full deplex
	mac_port_config |= 0x1 << 10;

	//RGMII ENABLR
	mac_port_config |= 0x1 << 15;

	GSW_MAC_PORT_0_CONFIG = mac_port_config;

	udelay(1000);

	/* adjust MAC port 0 RX/TX clock skew */
	GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 24) | (0x3 << 26));
	GSW_BIST_RESULT_TEST_0 |= ((0x2 << 24) | (0x2 << 26));
	
	// configure MAC port 0 pad drive strength = 10/100 mode
	
	//*(u32 volatile *) (0xf770001C) |= (0x1 << 2);

	PWRMGT_PAD_DRIVE_STRENGTH_CONTROL |= (0x1 << 2);
}



// agere power down/up
int AGERE_phy_power_down(int phy_addr, int y)
{
	u16 phy_data = 0;
	// power-down or up the PHY
	star_gsw_read_phy(phy_addr, 0, &phy_data);
	if (y==1) // down
		phy_data |= (0x1 << 11);
	if (y==0) // up
		phy_data &= (~(0x1 << 11));
	star_gsw_write_phy(phy_addr, 0, phy_data);
	return 0;
}

// add by descent 2006/07/31
// standard phy register 0 and offset is 11 is power down
int std_phy_power_down(int phy_addr, int y)
{
	u16 phy_data = 0;
	/* set isolate bit instead of powerdown */
	star_gsw_read_phy(phy_addr, 0, &phy_data);
	if (y==1) // set isolate
#if (defined VELA  || defined LEO)
		phy_data |= (0x1 << 11);
#else
		phy_data |= (0x1 << 10);
#endif
	
	if (y==0) // unset isolate
#if (defined VELA  || defined LEO)
		phy_data &= (~(0x1 << 11));
#else
		phy_data &= (~(0x1 << 10));
#endif
	star_gsw_write_phy(phy_addr, 0, phy_data);
	return 0;
}


/*
 *  AGERE PHY is attached to MAC PORT 1
 * with phy_addr 1
 */
void star_gsw_config_AGERE()
{
	u32 mac_port_config;
	u16 phy_data = 0;
	int i;

	printk("configure port1 AGERE\n");

	/*
	 * Configure MAC port 1
	 * For Agere Systems's ET1011 single PHY
	 */
	mac_port_config = GSW_MAC_PORT_1_CONFIG;

	// disable PHY's AN
	mac_port_config &= ~(0x1 << 7);

	// enable RGMII-PHY mode
	mac_port_config |= (0x1 << 15);

	GSW_MAC_PORT_1_CONFIG = mac_port_config;

#if 1
	/*
	 * configure Agere's ET1011 Single PHY
	 */
	/* Configure Agere's ET1011 by Agere's programming note */
	//1. power-down the PHY
	star_gsw_read_phy(1, 0, &phy_data);
	phy_data |= (0x1 << 11);
	star_gsw_write_phy(1, 0, phy_data);

	//2. Enable PHY programming mode
	star_gsw_read_phy(1, 18, &phy_data);
	phy_data |= (0x1 << 0);
	phy_data |= (0x1 << 2);
	star_gsw_write_phy(1, 18, phy_data);

	//3.Perform some PHY register with the Agere-specfic value
	star_gsw_write_phy(1, 16, 0x880e);
	star_gsw_write_phy(1, 17, 0xb4d3);

	star_gsw_write_phy(1, 16, 0x880f);
	star_gsw_write_phy(1, 17, 0xb4d3);

	star_gsw_write_phy(1, 16, 0x8810);
	star_gsw_write_phy(1, 17, 0xb4d3);

	star_gsw_write_phy(1, 16, 0x8817);
	star_gsw_write_phy(1, 17, 0x1c00);

	star_gsw_write_phy(1, 16, 0x8805);
	star_gsw_write_phy(1, 17, 0xb03e);

	star_gsw_write_phy(1, 16, 0x8806);
	star_gsw_write_phy(1, 17, 0xb03e);

	star_gsw_write_phy(1, 16, 0x8807);
	star_gsw_write_phy(1, 17, 0xff00);

	star_gsw_write_phy(1, 16, 0x8808);
	star_gsw_write_phy(1, 17, 0xe110);

	star_gsw_write_phy(1, 16, 0x300d);
	star_gsw_write_phy(1, 17, 0x0001);

	//4. Disable PHY programming mode
	star_gsw_read_phy(1, 18, &phy_data);
	phy_data &= ~(0x1 << 0);
	phy_data &= ~(0x1 << 2);
	star_gsw_write_phy(1, 18, phy_data);

	//5. power-up the PHY
	star_gsw_read_phy(1, 0, &phy_data);
	phy_data &= ~(0x1 << 11);
	star_gsw_write_phy(1, 0, phy_data);

	star_gsw_read_phy(1, 22, &phy_data);

	// enable RGMII MAC interface mode : RGMII/RMII (dll delay or trace delay) mode
	phy_data &= ~(0x7 << 0);

	// phy_data |= (0x6 << 0); // RGMII/RMII dll delay mode : not work!!
	phy_data |= (0x4 << 0);	// RGMII/RMII trace delay mode

	star_gsw_write_phy(1, 22, phy_data);
#endif

	mac_port_config = GSW_MAC_PORT_1_CONFIG;

	// enable PHY's AN
	mac_port_config |= (0x1 << 7);

	GSW_MAC_PORT_1_CONFIG = mac_port_config;

	/*
	 * Enable flow-control on (Symmetric PAUSE frame)
	 */
	star_gsw_read_phy(1, 0x4, &phy_data);

	phy_data |= (0x1 << 10);

	star_gsw_write_phy(1, 0x4, phy_data);

	/*
	 * Enable PHY1 AN restart bit to restart PHY1 AN
	 */
	star_gsw_read_phy(1, 0x0, &phy_data);

	phy_data |= (0x1 << 9) | (0x1 << 12);

	star_gsw_write_phy(1, 0x0, phy_data);

	/*
	 * Polling until PHY1 AN restart is complete and PHY1 link status is UP
	 */
	for (i = 0; i < 0x2000; i++) {
		star_gsw_read_phy(1, 0x1, &phy_data);
		if ((phy_data & (0x1 << 5)) && (phy_data & (0x1 << 2))) {
			break;
		}
	}

	// adjust MAC port 1 RX/TX clock skew
	GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 28) | (0x3 << 30));
	GSW_BIST_RESULT_TEST_0 |= ((0x2 << 28) | (0x3 << 30));

	udelay(100);

	mac_port_config = GSW_MAC_PORT_1_CONFIG;
	if (!(mac_port_config & 0x1) || (mac_port_config & 0x2)) {
		/*
		 * Port 1 PHY link down or no TXC in Port 1
		 */
		PDEBUG("PHY1: Link Down, 0x%08x!\n", mac_port_config);
		return;
	}

}



int str9100_gsw_config_mac_port0()
{
        PDEBUG("str9100_gsw_config_mac_port0\n");
        INIT_PORT0_PHY
	INIT_PORT0_MAC
        PORT0_LINK_DOWN
        return 0;
}

int str9100_gsw_config_mac_port1()
{
        INIT_PORT1_PHY
	INIT_PORT1_MAC
        PORT1_LINK_DOWN
        //PORT1_LINK_UP
        return 0;
}


#define PHY_CONTROL_REG_ADDR 0x00
#define PHY_AN_ADVERTISEMENT_REG_ADDR 0x04


int icp_101a_init (int port)
{
	u32 mac_port_config;
        u16 phy_data = 0;


	PRINT_INFO("init IC+101A\n");

	if (port==0)
	{
		mac_port_config = GSW_MAC_PORT_0_CONFIG;
	}
	else if (port==1)
	{
		mac_port_config = GSW_MAC_PORT_1_CONFIG;
	}
	else
	{
		printk("MAC port number %d is not support!", port);
		return (1);
	}
	
	if (!(mac_port_config & (0x1 << 5))) {
		if (!star_gsw_read_phy (port, PHY_AN_ADVERTISEMENT_REG_ADDR, &phy_data))
	    	{
			PDEBUG("\n PORT%d, enable local flow control capability Fail\n", port);
			return (1);
	    	}
		else
	    	{
	      		// enable PAUSE frame capability
			phy_data |= (0x1 << 10);

	      		if (!star_gsw_write_phy (port, PHY_AN_ADVERTISEMENT_REG_ADDR, phy_data))
			{
				PDEBUG("\nPORT%d, enable PAUSE frame capability Fail\n", port);
				return (1);
			}
	    	}
	}


	// restart PHY0 AN
	if (!star_gsw_read_phy (port, PHY_CONTROL_REG_ADDR, &phy_data)) {
		PDEBUG ("\n restart PHY%d AN Fail \n", port);
		return (1);
	}
	else {
		// enable PHY0 AN restart
		phy_data |= (0x1 << 9);

		if (!star_gsw_write_phy (port, PHY_CONTROL_REG_ADDR, phy_data)) {
			PDEBUG ("\n  enable PHY0 AN restart \n");
			return (1);
		}
	}



	while (1)
	{
		PDEBUG ("\n Polling  PHY%d AN \n", port);
		star_gsw_read_phy (port, PHY_CONTROL_REG_ADDR, &phy_data);

		if (phy_data & (0x1 << 9)) {
		  continue;
		}
		else {
			PDEBUG ("\n PHY%d AN restart is complete \n", port);
			break;
		}
	}

	return 0;
}

#ifdef CONFIG_LIBRA
void icp_175c_all_phy_power_down(int y)
{
	int i=0;

	for (i=0 ; i < 5 ; ++i)
		std_phy_power_down(i, y);

}

#define CONFIG_PORT0_5

void configure_icplus_175c_phy(void)
{
	u32 volatile	II, jj;
	u32 volatile	mac_port_config;
	u16 volatile reg;
	u32 volatile reg2;
	int i=0;

	//star_gsw_set_phy_addr(0, 0);
	//star_gsw_set_phy_addr(1, 1);

	printk("\n ICPLUS175C_PHY,enable PORT0 local flow control capability \n");
	/* adjust MAC port 0 /RX/TX clock skew */
	GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 24) | (0x3 << 26));
	GSW_BIST_RESULT_TEST_0 |= ((0x2 << 24) | (0x2 << 26));

// 20061117 descent
	// auto polling default phy address is 0
	// so set phy address to not exist address to avoid auto polling
	// in STAR Libra board if no these code, port 0 link state will get half duplex
	// port 1-4 get full duplex
        if (star_gsw_set_phy_addr(0, 15))
                printk ("star_gsw_set_phy_addr(0,2) is successful\n");
        else
                printk ("star_gsw_set_phy_addr(0,2) is fail\n");

        if (star_gsw_set_phy_addr(1, 16))
                printk ("star_gsw_set_phy_addr(1,3) is successful\n");
        else
                printk ("star_gsw_set_phy_addr(1,3) is fail\n");



	#if 0
	// for PHY_AN_ADVERTISEMENT_REG_ADDR
	// in this case needn't this code, only for reference
		star_gsw_read_phy(i, 4, &phy_data);
		phy_data |= (0x1 << 10);  // Enable PAUSE frame capability of PHY 0
		phy_data |= (0x1 << 5) | (0x1 << 6) | (0x1 << 7) | (0x1 << 8);
		star_gsw_write_phy(i, 4, phy_data);
		star_gsw_read_phy(i, 0, &phy_data);
		phy_data |= (0x1 << 9) | (0x1 << 12);  // Enable AN and Restart-AN
		star_gsw_write_phy(i, 0, phy_data);
	#endif
// 20061117 descent end


#if 0
	for (i=0 ; i < 10; ++i)
	{
	  star_gsw_read_phy(i, 2, &reg);
	  printk("addr: %d, phyid: %x\n", i, reg);
	}




	/*
	 * Configure GSW's MAC port 0
	 * For ASIX's 5-port GbE Switch setting
	 * 1. No SMI (MDC/MDIO) connection between Orion's MAC port 0 and ASIX's MAC port 4
	 * 2. Force Orion's MAC port 0 to be 1000Mbps, and full-duplex, and flow control on
	 */
	mac_port_config = GSW_MAC_PORT_0_CONFIG;



	// enable RGMII-PHY mode
	mac_port_config &= ~(0x1 << 15);

	// force speed = 100Mbps
	mac_port_config &= ~(0x3 << 8);
	mac_port_config |= (0x1 << 8);

	// force full-duplex
	mac_port_config |= (0x1 << 10);

	// force Tx/Rx flow-control on
	mac_port_config |= (0x1 << 11) | (0x1 << 12);

	GSW_MAC_PORT_0_CONFIG = mac_port_config;
	star_gsw_write_phy(29, 31, 0x175C);
	//star_gsw_write_phy(30, 9, 0x1089);
	star_gsw_write_phy(29, 23, 0x2);

	star_gsw_write_phy(29, 24, 0x2);
	star_gsw_write_phy(29, 25, 0x1);
	star_gsw_write_phy(29, 26, 0x1);
	star_gsw_write_phy(29, 27, 0x1);
	star_gsw_write_phy(29, 28, 0x1);
	star_gsw_write_phy(29, 29, 0x2);

	star_gsw_write_phy(30, 1, (0x3e << 8) );
	star_gsw_write_phy(30, 2, 0x21);
	star_gsw_write_phy(30, 9, 0x80);

	udelay(1000);

	mac_port_config = GSW_MAC_PORT_0_CONFIG;

	if (((mac_port_config & 0x1) == 0) || (mac_port_config & 0x2)) {
		printk("Check MAC/PHY 0 Link Status : DOWN!\n");
	} else {
		printk("Check MAC/PHY 0 Link Status : UP!\n");
	}

	/* adjust MAC port 0 /RX/TX clock skew */
	GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 24) | (0x3 << 26));
	GSW_BIST_RESULT_TEST_0 |= ((0x2 << 24) | (0x2 << 26));


#endif



#if 1

	
#ifdef LINUX24
	reg2 = __REG(PWR_PAD_DRV_REG);
	PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
	reg2 = reg2 | 0x4;

	__REG(PWR_PAD_DRV_REG) = reg2;

	reg2 = __REG(PWR_PAD_DRV_REG);
	PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
#endif // LINUX24


#ifdef LINUX26
	reg2 = PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG;
	PDEBUG("[PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = %x]\n", reg2);
	// Set MAC port 0 I/O pad drive strength as 10/100 mode.
	reg2 = reg2 | 0x04;

	PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = reg2;

	reg2 = PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG;
	PDEBUG("[PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = %x]\n", reg2);
#endif // LINUX26
	
	star_gsw_write_phy(29, 31, 0x175C);
	

	star_gsw_read_phy(0, 2, &reg);
	PDEBUG("[0,%d,%x]\n", 2, reg);
	star_gsw_read_phy(29, 31, &reg);
	PDEBUG("[29,%d,%x]\n", 31, reg);

	// Tag/un-tag function setup
	star_gsw_write_phy(29, 23, 0x7C2);
	// PVID function setup
	star_gsw_write_phy(29, 24, 0x1); // Define PVID of Port 0
	star_gsw_write_phy(29, 25, 0x1); // Define PVID of Port 1
	star_gsw_write_phy(29, 26, 0x1); // Define PVID of Port 2

#ifdef CONFIG_PORT0_5
	star_gsw_write_phy(29, 27, 0x1); // Define PVID of Port 3
#else
	star_gsw_write_phy(29, 27, 0x2); // Define PVID of Port 3
#endif // CONFIG_PORT0_5

	star_gsw_write_phy(29, 28, 0x2); // Define PVID of Port 4
	star_gsw_write_phy(29, 30, 0x2); // Define PVID of MII0

	// VLAN Mask function setup
	// Tag VLAN0[5:0] / VLAN1[13:8] output mask
#ifdef CONFIG_PORT0_5
	printk("CONFIG_PORT0_5\n");
	// VLAN0 (101111)
	// VLAN1 (110000)
	star_gsw_write_phy(20, 1, 0x0C2F);
#else // port0_4
	printk("not CONFIG_PORT0_5\n");
	// VLAN0 (111000)
	// VLAN1 (100111)
	star_gsw_write_phy(30, 1, 0x09F8);
#endif // CONFIG_PORT0_5

	// Smart MAC function setup
	// 30.9[2:0]  001    Define 1 LAN group
	// 30.9[3]    1      Enable router function
	// 30.9[6:4]  000    Enable ID index as 000
	// 30.9[7]    1      Enable tag VLAN
	// 30.9[12:8] 10000  define port 4 as a WAN port
	star_gsw_write_phy(30, 9, 0x1089);




// 20061115 descent
// If no these code,
// in STAR libra board, will get crc drop
	// configure port 4 (MII 0)
	// configure port 4 (MII 0)

	// P4_FORCE
	star_gsw_read_phy(29, 22, &reg);
	reg |= (0x1 << 15);
        star_gsw_write_phy(29, 22, reg);

	
	// MAC_X_EN, flow control enable of MII0 and MII2
	star_gsw_read_phy(29, 18, &reg);
	reg |= (0x1 << 10);
        star_gsw_write_phy(29, 18, reg);


	// P4_FORCE 100 Mbps
	star_gsw_read_phy(29, 22, &reg);
	reg |= (0x1 << 10);
        star_gsw_write_phy(29, 22, reg);


	// P4_FORCE_FULL duplex
	star_gsw_read_phy(29, 22, &reg);
	reg |= (0x1 << 5);
        star_gsw_write_phy(29, 22, reg);

// 20061115 descent end

#if 0 // Tommy debug
	// Enable PAUSE frame capability
	star_gsw_read_phy(4, 4, &reg);
	reg |= (0x01 << 10);
	star_gsw_write_phy(4, 4, reg);
	
	// MAC1 - phy4 flow control handshake
	// Auto-negotiation enable(bit12) and Restart auto-negotiation(bit9)
	star_gsw_read_phy(4, 0, &reg);
	reg |= ((0x01 << 9) | (0x01 << 12));
	star_gsw_write_phy(4, 0, reg);



	// Print out BW_CONTROL_P5_TX
	star_gsw_read_phy(31, 2, &reg);
	printk("star_gsw_read_phy(31, 2) = %04X\n", reg);

	star_gsw_read_phy(31, 3, &reg);
	printk("star_gsw_read_phy(31, 3) = %04X\n", reg);

	star_gsw_read_phy(31, 4, &reg);
	printk("star_gsw_read_phy(31, 4) = %04X\n", reg);

	star_gsw_read_phy(31, 5, &reg);
	printk("star_gsw_read_phy(31, 5) = %04X\n", reg);

	star_gsw_read_phy(31, 6, &reg);
	printk("star_gsw_read_phy(31, 6) = %04X\n", reg);

	star_gsw_read_phy(29, 22, &reg);
	printk("star_gsw_read_phy(29, 22) = %04X\n", reg);

	star_gsw_read_phy(29, 18, &reg);
	printk("star_gsw_read_phy(29, 18) = %04X\n", reg);

	star_gsw_read_phy(4, 0, &reg);
	printk("star_gsw_read_phy(4, 0) = %04X\n", reg);

	star_gsw_read_phy(4, 1, &reg);
	printk("star_gsw_read_phy(4, 1) = %04X\n", reg);
#endif // Tommy debug



	
	#if 0
	// enable flow control
	star_gsw_read_phy(29, 18, &reg);
	reg |= (0x1 < 13);
	star_gsw_write_phy(29, 18, reg);
	star_gsw_read_phy(29, 18, &reg);
	printk("[29,%d,%x]\n", 18, reg);
	#endif
#if 0
	//star_gsw_read_phy(29, 23, &reg);
	//reg = reg & 0xF800;
	//reg = reg | 0x7C2;


	star_gsw_write_phy(29, 24, 0x1);

	star_gsw_read_phy(29, 24, &reg);
	PDEBUG("[  29, 24, %x]\n", reg);

	star_gsw_write_phy(29, 25, 0x2);
	udelay(1000);

	star_gsw_read_phy(29, 25, &reg);
	PDEBUG("[  29, 25, %x]\n", reg);

	star_gsw_write_phy(29, 26, 0x1);
	star_gsw_write_phy(29, 27, 0x1);
	star_gsw_write_phy(29, 28, 0x1);
	star_gsw_write_phy(29, 30, 0x2);

	// tag vlan mask

	star_gsw_write_phy(30, 1, 0x3F3D);

	star_gsw_write_phy(30, 2, 0x3F22);
	star_gsw_write_phy(30, 9, 0x1089);
	// star_gsw_write_phy(30, 1, 0x3D22);
	// star_gsw_write_phy(30, 9, 0x028A);
#endif

#if 0

	for(II=18;II<=30;II++)
	{
		star_gsw_read_phy(29, II, &reg);
		PDEBUG("[29,%d,%x]\n", II, reg);
	}

	for(II=1;II<=10;II++)
	{
		star_gsw_read_phy(30, II, &reg);
		PDEBUG("[30,%d,%x]\n", II, reg);
	}

#endif

	mac_port_config = GSW_MAC_PORT_0_CONFIG;

	// disable PHY's AN
	mac_port_config &= ~(0x1 << 7);

	// disable RGMII-PHY mode
	mac_port_config &= ~(0x1 << 15);

	// force speed = 100Mbps
	mac_port_config &= ~(0x3 << 8);
	mac_port_config |= (0x1 << 8);
	
	// force full-duplex
	mac_port_config |= (0x1 << 10);

	// force Tx/Rx flow-control on
	mac_port_config |= (0x1 << 11) | (0x1 << 12);

	GSW_MAC_PORT_0_CONFIG = mac_port_config;


#if 0
	for (II = 0; II < 0x2000; II++)
	{
		mac_port_config = GSW_MAC_PORT_0_CONFIG;
		
		if ((mac_port_config & 0x1) && !(mac_port_config & 0x2))
		{

			/* enable MAC port 0
			*/
			mac_port_config &= ~(0x1 << 18);

		
			/*
			* enable the forwarding of unknown, multicast and broadcast packets to CPU
			*/
			mac_port_config &= ~((0x1 << 25) | (0x1 << 26) | (0x1 << 27));
		
			/*
			* include unknown, multicast and broadcast packets into broadcast storm
			*/
			mac_port_config |= ((0x1 << 29) | (0x1 << 30) | ((u32)0x1 << 31));
			// mac_port_config |= ( (0x1 << 30) | ((u32)0x1 << 31));
			
			GSW_MAC_PORT_0_CONFIG = mac_port_config;
			
			break;
		}
	}
#endif

	if (!(mac_port_config & 0x1) || (mac_port_config & 0x2))
	{
		/*
		* Port 0 PHY link down or no TXC in Port 0
		*/
		printk("\rCheck MAC/PHY 0 Link Status : DOWN!\n");
		
	}
	else
	{
		printk("\rCheck MAC/PHY 0 Link Status : UP!\n");
	}
#endif

	//printk("Found ICPLUS175C_PHY\n");
}
#endif // CONFIG_LIBRA


/**********************************************************************
 * Orion --- Libra2
 *********************************************************************/
#ifdef CONFIG_LIBRA2
void icplus_175c_phy_power_down(int port_no, int type)
{
	int i=0;

	if (port_no==0)
	{
		for (i=0 ; i < 4 ; ++i)
			std_phy_power_down(i, type);
	}
	
	if (port_no==1)
	{
		std_phy_power_down(4, type);
	}

}


void configure_icplus_175c_phy0_3(void)
{
	u16 volatile reg;
	u32 volatile reg2;
	
	printk("\n ICPLUS175C_PHY,enable PORT0 local flow control capability \n");
	/* adjust MAC port 0 /RX/TX clock skew */
	GSW_BIST_RESULT_TEST_0 &= ~((0x3 << 24) | (0x3 << 26));
	GSW_BIST_RESULT_TEST_0 |= ((0x2 << 24) | (0x2 << 26));

// 20061117 descent
	// auto polling default phy address is 0
	// so set phy address to not exist address to avoid auto polling
	// in STAR library board if no these code, port 0 link state will get half duplex
	// port 1-4 get full duplex
	// Set MAC0 phy assress to 15 not 0~3 to prevent address parsed
	// by switch.
	if (star_gsw_set_phy_addr(0, 15))
		printk ("star_gsw_set_phy_addr(0,15) is successful\n");
	else
		printk ("star_gsw_set_phy_addr(0,15) is fail\n");

	
#ifdef LINUX24
	reg2 = __REG(PWR_PAD_DRV_REG);
	PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
	reg2 = reg2 | 0x4;

	__REG(PWR_PAD_DRV_REG) = reg2;

	reg2 = __REG(PWR_PAD_DRV_REG);
	PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
#endif // LINUX24

#ifdef LINUX26
	reg2 = PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG;
	PDEBUG("[PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = %x]\n", reg2);
	// Set MAC port 0 I/O pad drive strength as 10/100 mode.
	reg2 = reg2 | 0x04;

	PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = reg2;

	reg2 = PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG;
	PDEBUG("[PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = %x]\n", reg2);
#endif // LINUX26

	star_gsw_write_phy(29, 31, 0x175C);
	
// 20061115 descent
// If no these code,
// in STAR libra board, will get crc drop
	// configure port 4 (MII 0)
	// P4_FORCE
	star_gsw_read_phy(29, 22, &reg);
	reg |= (0x1 << 15);
	star_gsw_write_phy(29, 22, reg);

	
	// MAC_X_EN, flow control enable of MII0 and MII2
	star_gsw_read_phy(29, 18, &reg);
	reg |= (0x1 << 10);
	star_gsw_write_phy(29, 18, reg);


	// P4_FORCE 100 Mbps
	star_gsw_read_phy(29, 22, &reg);
	reg |= (0x1 << 10);
	star_gsw_write_phy(29, 22, reg);


	// P4_FORCE_FULL duplex
	star_gsw_read_phy(29, 22, &reg);
	reg |= (0x1 << 5);
	star_gsw_write_phy(29, 22, reg);

// 20061115 descent end

	config_MAC_port0_libra2();

}

void configure_icplus_175c_phy4(void)
{
	u16 volatile reg;
	u32 volatile reg2;

	// Set auto-polling phy address.
	if (star_gsw_set_phy_addr(1, 4))
		printk ("star_gsw_set_phy_addr(1, 4) is successful\n");
	else
		printk ("star_gsw_set_phy_addr(1, 4) is fail\n");
	
#ifdef LINUX24
	reg2 = __REG(PWR_PAD_DRV_REG);
	PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
	reg2 = reg2 | 0x8;

	__REG(PWR_PAD_DRV_REG) = reg2;

	reg2 = __REG(PWR_PAD_DRV_REG);
	PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
#endif // LINUX24

#ifdef LINUX26
	reg2 = PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG;
	PDEBUG("[PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = %x]\n", reg2);
	// Set MAC port 1 I/O pad drive strength as 10/100 mode.
	reg2 = reg2 | 0x08;

	PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = reg2;

	reg2 = PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG;
	PDEBUG("[PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = %x]\n", reg2);
#endif // LINUX26

	// Read phy4 "Organizationally unique identifier - 0x0243)
	//star_gsw_read_phy(4, 2, &reg);
	//printk("   *** star_gsw_read_phy(4,2) = 0x%04x\n", reg);
	
	// Enable PAUSE frame capability
	star_gsw_read_phy(4, 4, &reg);
	reg |= (0x01 << 10);
	star_gsw_write_phy(4, 4, reg);
	
	// MAC1 - phy4 flow control handshake
	// Auto-negotiation enable(bit12) and Restart auto-negotiation(bit9)
	star_gsw_read_phy(4, 0, &reg);
	reg |= ((0x01 << 9) | (0x01 << 12));
	star_gsw_write_phy(4, 0, reg);

	config_MAC_port1_libra2();
	
}

void config_MAC_port0_libra2(void)
{
	u32 volatile mac_port_config;
	
	mac_port_config = GSW_MAC_PORT_0_CONFIG;
	
	// Disable MAC0 PHY's AN
	mac_port_config &= ~(0x1 << 7);

	// Disable RGMII-PHY mode
	mac_port_config &= ~(0x1 << 15);

	// Force speed = 100Mbps
	mac_port_config &= ~(0x3 << 8);
	mac_port_config |= (0x1 << 8);
	
	// Force full-duplex
	mac_port_config |= (0x1 << 10);

	// Force Tx/Rx flow-control on
	mac_port_config |= (0x1 << 11) | (0x1 << 12);

	GSW_MAC_PORT_0_CONFIG = mac_port_config;
	printk("   ---> MAC Port 0 initial!\n");
		
#if 0 // DEBUG_LIBRA2_FLAG_1
	mac_port_config = GSW_MAC_PORT_0_CONFIG;
	if (!(mac_port_config & 0x1) || (mac_port_config & 0x2))
	{
		/*
		* Port 0 PHY link down or no TXC in Port 0
		*/
		printk("\r   ---> Check MAC/PHY 0 Link Status : DOWN!\n");
	}
	else
	{
		printk("\r   ---> Check MAC/PHY 0 Link Status : UP!\n");
	}
#endif // DEBUG_LIBRA2_FLAG_1

}


void config_MAC_port1_libra2(void)
{
	u32 volatile mac_port_config;
	
	mac_port_config = GSW_MAC_PORT_1_CONFIG;

	// Enable MAC port 1
	mac_port_config &= ~(0x01 << 18);
	
	mac_port_config &= ~((0x01 << 25) | (0x01 << 26) | (0x01 << 27));
	
	// Eable MAC0 PHY's AN
	mac_port_config |= (0x1 << 7);

	// Disable RGMII-PHY mode
	mac_port_config &= ~(0x1 << 15);

	// Force speed = 100Mbps
	mac_port_config &= ~(0x3 << 8);
	mac_port_config |= (0x1 << 8);
	
	// Force full-duplex
	mac_port_config |= (0x1 << 10);

	// Force Tx/Rx flow-control on
	mac_port_config |= (0x1 << 11) | (0x1 << 12);

	GSW_MAC_PORT_1_CONFIG = mac_port_config;
	printk("   ---> MAC Port 1 initial!\n");


#if 0 // DEBUG_LIBRA2_FLAG_2
	mac_port_config = GSW_MAC_PORT_1_CONFIG;
	if (!(mac_port_config & 0x1) || (mac_port_config & 0x2))
	{
		/*
		* Port 1 PHY link down or no TXC in Port 1
		*/
		printk("\r   ---> Check MAC/PHY 1 Link Status : DOWN!\n");
	}
	else
	{
		printk("\r   ---> Check MAC/PHY 1 Link Status : UP!\n");
	}
#endif // DEBUG_LIBRA2_FLAG_2	

}

#endif // CONFIG_LIBRA2

void star_read_reg(unsigned long int addr, unsigned char *data, unsigned char len)
{
    volatile unsigned long remap = (unsigned long)ioremap(addr, len*8);
    memcpy(data, (char *)remap, len);
}

void star_write_reg(unsigned long int addr, unsigned char *data, unsigned char len)
{
    volatile unsigned long remap = (unsigned long)ioremap(addr, len*8);
    memcpy((char *)remap, data, len);
}

asmlinkage int sys_bcm_mdio_wreg(unsigned char page, unsigned char reg, unsigned char *data, unsigned char len)
{
    star_gsw_mdio_wreg(page, reg, data, len);
    return 0;
}

asmlinkage int sys_bcm_mdio_rreg(unsigned char page, unsigned char reg, unsigned char *data, unsigned char len)
{
    star_gsw_mdio_rreg(page, reg, data, len);
    return 0;
}

/* 
 * TF1 Change
 * These routines are used in bcmSwitchdriver
 */
EXPORT_SYMBOL (sys_bcm_mdio_wreg);
EXPORT_SYMBOL (sys_bcm_mdio_rreg);

#define PHY_READ    0
#define PHY_WRITE   1
asmlinkage int sys_bcm_phy_opt(unsigned char opt, unsigned char phy_addr, unsigned char phy_reg, unsigned short int * data)
{
    switch (opt)
    {
        case PHY_READ:
            {
                star_gsw_read_phy(phy_addr, phy_reg, data);
            }
            break;
        case PHY_WRITE:
            {
                star_gsw_write_phy(phy_addr, phy_reg, *data);
            }
            break;
        default:
            {
                printk ("Error : invalid operation to r/w phy!\n");
            }
            break;
    }
    return 0;
}

#define STAR_REG_READ   0
#define STAR_REG_WRITE  1
asmlinkage int sys_star_reg_opt(unsigned char opt, unsigned long int addr, unsigned char *data, unsigned char len)
{
    switch (opt)
    {
        case STAR_REG_READ:
            {
                star_read_reg(addr, data, len);
            }
            break;
        case STAR_REG_WRITE:
            {
                star_write_reg(addr, data, len);
            }
            break;
        default:
            {
                printk("Error : invalid operation to r/w registers\n");
            }
            break;
    }
    return 0;
}
