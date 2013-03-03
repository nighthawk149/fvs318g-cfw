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
		mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;
	if (port==1)
		mac_port_config = GSW_MAC_PORT_1_CONFIG_REG;
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
		GSW_MAC_PORT_0_CONFIG_REG = mac_port_config;
	if (port==1)
		GSW_MAC_PORT_1_CONFIG_REG = mac_port_config;
}

static int star_gsw_set_phy_addr(u8 mac_port, u8 phy_addr)
{
	u32 status = 0;	/* for failure indication */

	if ((mac_port > 1) || (phy_addr > 31)) {
		return status;
	}

	if (mac_port == 0) {
		GSW_PORT_MIRROR_REG &= ~(0x3 << 0); /* clear bit[1:0] for PHY_ADDR[1:0] */
		GSW_PORT_MIRROR_REG &= ~(0x3 << 4); /* clear bit[5:4] for PHY_ADDR[3:2] */
		GSW_QUEUE_STATUS_TEST_1_REG &= ~(0x1 << 25); /* clear bit[25] for PHY_ADDR[4] */
		GSW_PORT_MIRROR_REG |= (((phy_addr >> 0) & 0x3) << 0);
		GSW_PORT_MIRROR_REG |= (((phy_addr >> 2) & 0x3) << 4);
		GSW_QUEUE_STATUS_TEST_1_REG |= (((phy_addr >> 4) & 0x1) << 25);
		status = 1; /* for ok indication */
	} else if (mac_port == 1) {
		GSW_PORT_MIRROR_REG &= ~(0x1 << 6); /* clear bit[6] for PHY_ADDR[0] */
		GSW_PORT_MIRROR_REG &= ~(0x7 << 8); /* clear bit[10:8] for PHY_ADDR[3:1] */
		GSW_QUEUE_STATUS_TEST_1_REG &= ~(0x1 << 26); /* clear bit[26] for PHY_ADDR[4] */
		GSW_PORT_MIRROR_REG |= (((phy_addr >> 0) & 0x1) << 6);
		GSW_PORT_MIRROR_REG |= (((phy_addr >> 1) & 0x7) << 8);
		GSW_QUEUE_STATUS_TEST_1_REG |= (((phy_addr >> 4) & 0x1) << 26);
		status = 1; /* for ok indication */
	}

	return status;
}

static int star_gsw_read_phy(u8 phy_addr, u8 phy_reg, u16 *read_data)
{
	u32 status;
	int i;

	// clear previous rw_ok status
	GSW_PHY_CONTROL_REG = (0x1 << 15);

        // 20061102 descent 
	// for ORION EOC
        GSW_QUEUE_STATUS_TEST_1_REG &= ~( 0XF << 16);

        GSW_PHY_CONTROL_REG   &= ~(0x1<<0);

        GSW_QUEUE_STATUS_TEST_1_REG |= (((phy_addr >> 1) & 0xF) << 16);
        // 20061102 descent end

	GSW_PHY_CONTROL_REG = ((phy_addr & 0x1) | ((phy_reg & 0x1F) << 8) | (0x1 << 14));

	for (i = 0; i < 0x1000; i++) {
		status = GSW_PHY_CONTROL_REG;
		if (status & (0x1 << 15)) {
			// clear the rw_ok status, and clear other bits value
			GSW_PHY_CONTROL_REG = (0x1 << 15);
			*read_data = (u16) ((status >> 16) & 0xFFFF);
			return (1);
		} else {
			udelay(10);
		}
	}

	return (0);
}

static int star_gsw_write_phy(u8 phy_addr, u8 phy_reg, u16 write_data)
{
	int i;

	// clear previous rw_ok status
	GSW_PHY_CONTROL_REG = (0x1 << 15);

        // 20061102 descent 
	// for ORION EOC
        GSW_QUEUE_STATUS_TEST_1_REG &= ~( 0XF << 16);

        GSW_PHY_CONTROL_REG   &= ~(0x1<<0);

        GSW_QUEUE_STATUS_TEST_1_REG |= (((phy_addr >> 1) & 0xF) << 16);
        // 20061102 descent end

	GSW_PHY_CONTROL_REG = ((phy_addr & 0x1) |
		((phy_reg & 0x1F) << 8) |
		(0x1 << 13) | ((write_data & 0xFFFF) << 16));

	for (i = 0; i < 0x1000; i++) {
		if ((GSW_PHY_CONTROL_REG) & (0x1 << 15)) {
			// clear the rw_ok status, and clear other bits value
			GSW_PHY_CONTROL_REG = (0x1 << 15);
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
	
	mac_port_base = SYSVA_GSW_BASE_ADDR + 0x0C;

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
	GSW_BIST_RESULT_TEST_0_REG &= ~((0x3 << 28) | (0x3 << 30));
	//GSW_BIST_RESULT_TEST_0 |= ((0x2 << 28) | (0x2 << 30));
	GSW_BIST_RESULT_TEST_0_REG |= (0x2 << 30);

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
		mac_port_base = SYSVA_GSW_BASE_ADDR + 0x08;
	}
	if (mac_port == 1) {
		PDEBUG("port 1\n");
		mac_port_base = SYSVA_GSW_BASE_ADDR + 0x0C;
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
#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
	/* near-end loopback mode */
	star_gsw_read_phy(phy_addr, 0x0, &phy_reg);
	phy_reg |= (0x1 << 14);
	star_gsw_write_phy(phy_addr, 0x0, phy_reg);
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

#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
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
		GSW_BIST_RESULT_TEST_0_REG &= ~((0x3 << 24) | (0x3 << 26));
		GSW_BIST_RESULT_TEST_0_REG |= ((0x2 << 24) | (0x2 << 26));
	}

	if (mac_port == 1) {
		// adjust MAC port 1 RX/TX clock skew
		GSW_BIST_RESULT_TEST_0_REG &= ~((0x3 << 28) | (0x3 << 30));
		GSW_BIST_RESULT_TEST_0_REG |= ((0x2 << 28) | (0x2 << 30));
	}

	return 0;
}

// add by descent 2006/07/10
// port : 0 => port0 ; port : 1 => port1
// y = 1 ; disable AN
int disable_AN(int port, int y)
{
	u32 mac_port_config;
	if (port==0)
		mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;
	if (port==1)
		mac_port_config = GSW_MAC_PORT_1_CONFIG_REG;
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
		GSW_MAC_PORT_0_CONFIG_REG = mac_port_config;
	if (port==1)
		GSW_MAC_PORT_1_CONFIG_REG = mac_port_config;
	return 0;
}

int disable_AN_VSC7385(int y)
{
	u32 mac_port_config;
	mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;

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

	GSW_MAC_PORT_0_CONFIG_REG = mac_port_config;
	return 0;
}

void star_gsw_config_VSC7385(void)
{
	u32 mac_port_config;

	printk("\nconfigure VSC7385\n");

	// disable auto-polling
	star_gsw_set_phy_addr(0, 0x10);

	/*
	 * Configure GSW's MAC port 0
	 * For ASIX's 5-port GbE Switch setting
	 * 1. No SMI (MDC/MDIO) connection between Orion's MAC port 0 and ASIX's MAC port 4
	 * 2. Force Orion's MAC port 0 to be 1000Mbps, and full-duplex, and flow control on
	 */
	mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;


	// enable RGMII-PHY mode
	mac_port_config |= (0x1 << 15);

	// force speed = 1000Mbps
	mac_port_config &= ~(0x3 << 8);
	mac_port_config |= (0x2 << 8);

	// force full-duplex
	mac_port_config |= (0x1 << 10);

	// force Tx/Rx flow-control on
	mac_port_config |= (0x1 << 11) | (0x1 << 12);

	GSW_MAC_PORT_0_CONFIG_REG = mac_port_config;

	udelay(1000);

	mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;

	if (((mac_port_config & 0x1) == 0) || (mac_port_config & 0x2)) {
		printk("Check MAC/PHY 0 Link Status : DOWN!\n");
	} else {
		printk("Check MAC/PHY 0 Link Status : UP!\n");
	}

	/* adjust MAC port 0 /RX/TX clock skew */
	GSW_BIST_RESULT_TEST_0_REG &= ~((0x3 << 24) | (0x3 << 26));
	GSW_BIST_RESULT_TEST_0_REG |= ((0x2 << 24) | (0x2 << 26));
}



void star_gsw_config_ASIX()
{
	u32 mac_port_config;

	printk("configure port0 ASIX\n");
	mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;

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

	GSW_MAC_PORT_0_CONFIG_REG = mac_port_config;

	udelay(1000);

	/* adjust MAC port 0 RX/TX clock skew */
	GSW_BIST_RESULT_TEST_0_REG &= ~((0x3 << 24) | (0x3 << 26));
	GSW_BIST_RESULT_TEST_0_REG |= ((0x2 << 24) | (0x2 << 26));
	
	// configure MAC port 0 pad drive strength = 10/100 mode
	//*(u_int32 volatile *) (0xf770001C) |= (0x1 << 2);
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
	// power-down or up the PHY
	star_gsw_read_phy(phy_addr, 0, &phy_data);
	if (y==1) // down
		phy_data |= (0x1 << 11);
	if (y==0) // up
		phy_data &= (~(0x1 << 11));
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
	mac_port_config = GSW_MAC_PORT_1_CONFIG_REG;

	// disable PHY's AN
	mac_port_config &= ~(0x1 << 7);

	// enable RGMII-PHY mode
	mac_port_config |= (0x1 << 15);

	GSW_MAC_PORT_1_CONFIG_REG = mac_port_config;

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

	mac_port_config = GSW_MAC_PORT_1_CONFIG_REG;

	// enable PHY's AN
	mac_port_config |= (0x1 << 7);

	GSW_MAC_PORT_1_CONFIG_REG = mac_port_config;

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
	GSW_BIST_RESULT_TEST_0_REG &= ~((0x3 << 28) | (0x3 << 30));
	GSW_BIST_RESULT_TEST_0_REG |= ((0x2 << 28) | (0x3 << 30));

	udelay(100);

	mac_port_config = GSW_MAC_PORT_1_CONFIG_REG;
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

	if (port == 0)		// port 0
		mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;
	if (port == 1)		// port 1
		mac_port_config = GSW_MAC_PORT_1_CONFIG_REG;

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
	GSW_BIST_RESULT_TEST_0_REG &= ~((0x3 << 24) | (0x3 << 26));
	GSW_BIST_RESULT_TEST_0_REG |= ((0x2 << 24) | (0x2 << 26));
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

	
#if 1
	//reg2 = __REG(PWR_PAD_DRV_REG);
	reg2 = PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG;
	PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
	reg2 = reg2 | 0x4;

	//__REG(PWR_PAD_DRV_REG) = reg2;
	PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG = reg2;

	//reg2 = __REG(PWR_PAD_DRV_REG);
	//PDEBUG("[PWR_PAD_DRV_REG = %x]\n", reg2);
#endif
	
	star_gsw_write_phy(29, 31, 0x175C);
	

	star_gsw_read_phy(0, 2, &reg);
	PDEBUG("[0,%d,%x]\n", 2, reg);
	star_gsw_read_phy(29, 31, &reg);
	PDEBUG("[29,%d,%x]\n", 31, reg);

	star_gsw_write_phy(29, 23, 0x7C2);
	star_gsw_write_phy(29, 24, 0x1);
	star_gsw_write_phy(29, 25, 0x1);
	star_gsw_write_phy(29, 26, 0x1);
	star_gsw_write_phy(29, 27, 0x2);
	star_gsw_write_phy(29, 28, 0x2);
	star_gsw_write_phy(29, 30, 0x2);

	//star_gsw_write_phy(30, 1, 0x2f00);
	star_gsw_write_phy(30, 1, 0x2700);
	//star_gsw_write_phy(30, 2, 0x30);
	star_gsw_write_phy(30, 2, 0x38);
	star_gsw_write_phy(30, 9, 0x80);
	
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

	mac_port_config = GSW_MAC_PORT_0_CONFIG_REG;

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

	GSW_MAC_PORT_0_CONFIG_REG = mac_port_config;


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
			mac_port_config |= ((0x1 << 29) | (0x1 << 30) | ((u_int32)0x1 << 31));
			// mac_port_config |= ( (0x1 << 30) | ((u_int32)0x1 << 31));
			
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
#endif
