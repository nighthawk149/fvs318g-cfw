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

#include "star_nic.h"

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,32)
#define IRQ_RETURN void
#define IRQ_HANDLED 
static const char star_nic_driver_version[] =
	"Star NIC Driver(for Linux Kernel 2.4) - Star Semiconductor\n";
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define IRQ_RETURN irqreturn_t
static const char star_nic_driver_version[] =
	"Star NIC Driver(for Linux Kernel 2.6) - Star Semiconductor\n";
#endif

//========================================================
#ifdef CONFIG_STAR_NIC_PHY_INTERNAL_PHY
#define FE_PHY_LED_MODE (0x1 << 12)
#define CONFIG_INTERNEL_PHY_PATCH
#endif

#ifdef CONFIG_INTERNEL_PHY_PATCH
#define INTERNAL_PHY_PATCH_CHECKCNT	16
#define INTERNAL_PHY_PATCH_CHECK_PERIOD	1000 //ms
static struct timer_list internal_phy_timer;
static void internal_phy_patch_check(int);
static void internal_phy_update(unsigned long data);
#endif
//========================================================

#define increase_cyclic(var, limit) {\
				var++; \
				if (var>=limit) var=0;\
			}

static struct net_device *CUR_NAPI_DEV;
static struct net_device *STAR_NIC_LAN_DEV;

static int install_isr_account = 0;
static int is_qf = 0; // determine queue full state

static spinlock_t star_nic_send_lock;

static TXRING_INFO txring;
static RXRING_INFO rxring;

static struct proc_dir_entry *star_nic_proc_entry;

static u8 default_mac_addr[] = { 0x08, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e };

typedef struct
{   
	u32 vid;	//0~4095
	u32 control;	//ENABLE or DISABLE
} my_vlan_entry_t;

static my_vlan_entry_t my_vlan_id[4] =
{
	{ 2, 0},	//value for my_vid0
	{ 2, 1},	//value for my_vid1
	{ 1, 1},	//value for my_vid2
	{ 1, 0}		//value for my_vid3
};

#ifdef CONFIG_STAR_NIC_NAPI
static void star_nic_receive_packet(int mode, int *work_done, int work_to_do);
#else
static void star_nic_receive_packet(int mode);
#endif

static void star_nic_phy_powerdown(struct net_device *dev);
static void star_nic_phy_powerup(struct net_device *dev);

#ifdef STAR_NIC_TIMER
static struct timer_list star_nic_timer;
static void star_nic_timer_func(unsigned long data)
{
	int i;
	int txsd_index;
	int txsd_current;
	int skb_free_count = 0;
	STAR_NIC_TXDESC volatile *txdesc_ptr;
	unsigned long flags;

	local_irq_save(flags);
	HAL_NIC_READ_TXSD(txsd_current);
	txsd_index = (txsd_current - (u32)txring.phy_addr) >> 4;
	if (txsd_index > txring.to_free_index) {
		skb_free_count = txsd_index - txring.to_free_index;
	} else if (txsd_index <= txring.to_free_index) {
		skb_free_count = STAR_NIC_MAX_TFD_NUM + txsd_index - txring.to_free_index;
	}
	for (i = 0; i < skb_free_count; i++) {
		txdesc_ptr = txring.vir_addr + txring.to_free_index;
		if (txdesc_ptr->cown == 0) {
			break;
		}
		if (txring.skb_ptr[txring.to_free_index]) {
			dev_kfree_skb_any(txring.skb_ptr[txring.to_free_index]);
			txring.skb_ptr[txring.to_free_index] = NULL;
		}
		txring.to_free_index++;
		if (txring.to_free_index == STAR_NIC_MAX_TFD_NUM) {
			txring.to_free_index = 0;
		}
	}
	local_irq_restore(flags);
}
#endif

#if 0
#define between(x, start, end) ((x)>=(start) && (x)<=(end))
static void print_packet(unsigned char *data, int len) 
{
	int i, j;

	printk("packet length: %d%s:\n", len, len>100?"(only show the first 100 bytes)":"");
	if (len > 100) {
		len = 100;
	}
	for (i = 0; len;) {
		if (len >=16) {
			for (j=0;j<16;j++) {
				printk("%02x ", data[i++]);
			}
			printk("| ");
			i -= 16;
			for(j=0;j<16;j++) {
				if (between(data[i], 0x21, 0x7e) ) {
					printk("%c", data[i++]);
				} else {
					printk(".");
					i++;
				}
			}
			printk("\n");
			len -= 16;
		} else {
			/* last line */
			for (j = 0; j < len; j++) {
				printk("%02x ", data[i++]);
			}
			for (;j < 16; j++) {
				printk("   ");
			}
			printk("| ");
			i -= len;
			for (j = 0;j < len; j++) {
				if (between(data[i], 0x21, 0x7e)) {
					printk("%c", data[i++]);
				} else {
					printk(".");
					i++;
				}
			}
			for (; j < 16; j++) {
				printk(" ");
			}
			printk("\n");
			len = 0;
		}
	}

	return;
}
#endif /* Disable function print_packet */ 

#ifdef STAR_NIC_DEBUG
static void star_nic_show_format_reg(u32 val)
{
	int i;

	for (i = 31; i >= 0; i--) {
		if (val & ((unsigned long)1 << i)) {
			printk("[%02d:1] ", i);
		} else {
			printk("[%02d:0] ", i);
		}
		if ((i % 8) == 0) {
			printk("\n");
		}
	}
	printk("==================================================================\n");
}

static void star_nic_show_reg(void)
{
	u32 reg_val;

	printk("\n");

	reg_val = NIC_MEM_MAP_VALUE(0x000);
	printk("NIC REG OFF 0x000: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x004);
	printk("NIC REG OFF 0x004: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x008);
	printk("NIC REG OFF 0x008: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x00C);
	printk("NIC REG OFF 0x00C: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x010);
	printk("NIC REG OFF 0x010: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x014);
	printk("NIC REG OFF 0x014: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x018);
	printk("NIC REG OFF 0x018: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x01C);
	printk("NIC REG OFF 0x01C: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x020);
	printk("NIC REG OFF 0x020: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x024);
	printk("NIC REG OFF 0x024: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x028);
	printk("NIC REG OFF 0x028: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x030);
	printk("NIC REG OFF 0x030: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x034);
	printk("NIC REG OFF 0x034: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x038);
	printk("NIC REG OFF 0x038: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x03C);
	printk("NIC REG OFF 0x03C: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x040);
	printk("NIC REG OFF 0x040: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x044);
	printk("NIC REG OFF 0x044: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x048);
	printk("NIC REG OFF 0x048: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x04C);
	printk("NIC REG OFF 0x04C: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x050);
	printk("NIC REG OFF 0x050: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x054);
	printk("NIC REG OFF 0x054: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x058);
	printk("NIC REG OFF 0x058: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);

	reg_val = NIC_MEM_MAP_VALUE(0x05C);
	printk("NIC REG OFF 0x05C: 0x%08x\n", reg_val);
	star_nic_show_format_reg(reg_val);
}
#endif

static void star_nic_mib_reset(void)
{
	u32 v;
	unsigned long flags;

	local_irq_save(flags);
	v = NIC_MIB_RX_OK_PKT_CNTR;
	v = NIC_MIB_RX_OK_BYTE_CNTR;
	v = NIC_MIB_RX_RUNT_BYTE_CNTR;
	v = NIC_MIB_RX_OSIZE_DROP_PKT_CNTR;
	v = NIC_MIB_RX_NO_BUF_DROP_PKT_CNTR;
	v = NIC_MIB_RX_CRC_ERR_PKT_CNTR;
	v = NIC_MIB_RX_ARL_DROP_PKT_CNTR;
	v = NIC_MIB_MYVLANID_MISMATCH_DROP_PKT_CNTR;
	v = NIC_MIB_RX_CHKSUM_ERR_PKT_CNTR;
	v = NIC_MIB_RX_PAUSE_FRAME_PKT_CNTR;
	v = NIC_MIB_TX_OK_PKT_CNTR;
	v = NIC_MIB_TX_OK_BYTE_CNTR;
	v = NIC_MIB_TX_PAUSE_FRAME_CNTR;
	local_irq_restore(flags);
}

static void star_nic_mib_read(struct net_device *dev)
{
	struct star_nic_private *priv = netdev_priv(dev);
	unsigned long flags;

	local_irq_save(flags);
	priv->mib_info.mib_rx_ok_pkt		+= NIC_MIB_RX_OK_PKT_CNTR;
	priv->mib_info.mib_rx_ok_byte		+= NIC_MIB_RX_OK_BYTE_CNTR;
	priv->mib_info.mib_rx_runt		+= NIC_MIB_RX_RUNT_BYTE_CNTR;
	priv->mib_info.mib_rx_over_size		+= NIC_MIB_RX_OSIZE_DROP_PKT_CNTR;
	priv->mib_info.mib_rx_no_buffer_drop	+= NIC_MIB_RX_NO_BUF_DROP_PKT_CNTR;
	priv->mib_info.mib_rx_crc_err		+= NIC_MIB_RX_CRC_ERR_PKT_CNTR;
	priv->mib_info.mib_rx_arl_drop		+= NIC_MIB_RX_ARL_DROP_PKT_CNTR;
	priv->mib_info.mib_rx_myvid_drop	+= NIC_MIB_MYVLANID_MISMATCH_DROP_PKT_CNTR;
	priv->mib_info.mib_rx_csum_err		+= NIC_MIB_RX_CHKSUM_ERR_PKT_CNTR;
	priv->mib_info.mib_rx_pause_frame	+= NIC_MIB_RX_PAUSE_FRAME_PKT_CNTR;
	priv->mib_info.mib_tx_ok_pkt		+= NIC_MIB_TX_OK_PKT_CNTR;
	priv->mib_info.mib_tx_ok_byte		+= NIC_MIB_TX_OK_BYTE_CNTR;
	priv->mib_info.mib_tx_pause_frame	+= NIC_MIB_TX_PAUSE_FRAME_CNTR;
	local_irq_restore(flags);
}

static int star_nic_write_phy(u8 phy_addr, u8 phy_reg, u16 write_data)
{
	int i;

	if (phy_addr > 31) {
		return 0;
	}

	//clear previous rw_ok status
	NIC_PHY_CONTROL_REG0 = (0x1 << 15);
 
	NIC_PHY_CONTROL_REG0 = ((phy_addr & 0x1F) |   
		((phy_reg & 0x1F) << 8) |
		(0x1 << 13) |
		((write_data & 0xFFFF) << 16));

	for (i = 0; i < 10000; i++) {
		// if write command completed
		if ((NIC_PHY_CONTROL_REG0) & (0x1 << 15)) {
			// clear the rw_ok status, and clear other bits value
			NIC_PHY_CONTROL_REG0 = (0x1 << 15);
			return (0);    /* for ok indication */
		}
		udelay(1000);
	}

	printk("star_nic_write_phy() failed!! phy_addr:0x%x phy_reg:0x%x write_data:0x%x\n",
		phy_addr, phy_reg, write_data);
	return (-1);    /* for failure indication */
}

static int star_nic_read_phy(u8 phy_addr, u8 phy_reg, u16 *read_data)
{
	u32 status;
	int i;

	if (phy_addr > 31) {
		return 0;
	}

	// clear previous rw_ok status
	NIC_PHY_CONTROL_REG0 = (0x1 << 15);

	NIC_PHY_CONTROL_REG0 = ((phy_addr & 0x1F) | 
		((phy_reg & 0x1F) << 8) | 
		(0x1 << 14));    

	for (i = 0; i < 10000; i++) {
		status = NIC_PHY_CONTROL_REG0;
		if (status & (0x1 << 15)) {
			// clear the rw_ok status, and clear other bits value
			NIC_PHY_CONTROL_REG0 = (0x1 << 15);
			*read_data = (u16)((status >> 16) & 0xFFFF);
			return (0);    /* for ok indication */
		}
		udelay(1000);
	}

	printk("star_nic_read_phy() failed!! phy_addr:0x%x phy_reg:0x%x\n",
		phy_addr, phy_reg);
	return (-1);    /* for failure indication */
}

static int star_nic_dma_config(struct net_device *dev)
{
	u32 dma_config = 0;

	dma_config = NIC_DMA_CONFIG_REG;

#if 1
	/* Config TX DMA */ 
	dma_config &=  ~(0x3 << 6); //TX auto polling :1  us
	//dma_config |=  (0x1 << 6); //TX auto polling :10 us
	dma_config |=  (0x2 << 6); //TX auto polling :100us
	//dma_config |=  (0x3 << 6); //TX auto polling :1000us
	dma_config |=  (0x1 << 5); //TX auto polling C-bit enable
	dma_config &=  ~(0x1 << 4); //TX can transmit packets,No suspend
#endif

#if 1
	/* Config RX DMA */
	dma_config &=  ~(0x3 << 2); //RX auto polling :1  us
	//dma_config |=  (0x1 << 2); //RX auto polling :10 us
	dma_config |=  (0x2 << 2); //RX auto polling :100us
	//dma_config |=  (0x3 << 2); //RX auto polling :1000us
	dma_config |=  (0x1 << 1); //RX auto polling C-bit enable
	dma_config &=  ~0x1; //RX can receive packets, No suspend
#endif

	// 4N+2(for Linux)
	dma_config &= ~(0x1 << 16);
	// 4N
	//dma_config |= (0x1 << 16);

	NIC_DMA_CONFIG_REG = dma_config;

	return 0;
}

static int star_nic_mac_config(struct net_device *dev)
{
	u32 mac_config;

	mac_config = NIC_MAC_CONTROL_REG;

#ifdef STAR_NIC_TX_HW_CHECKSUM
	// Tx ChkSum offload On: TCP/UDP/IP
	mac_config |= (0x1 << 26);
#else
	// Tx ChkSum offload Off: TCP/UDP/IP
	mac_config &= ~(0x1 << 26);
#endif

#ifdef STAR_NIC_RX_HW_CHECKSUM
	// Rx ChkSum offload On: TCP/UDP/IP
	mac_config |= (0x1 << 25);
#else
	// Rx ChkSum offload Off: TCP/UDP/IP
	mac_config &= ~(0x1 << 25);
#endif

	mac_config |= (0x1 << 24);	// Accept CSUM error pkt
	//mac_config &= ~(0x1 << 24);	// Discard CSUM error pkt

	//mac_config |= (0x1 << 23);	// IST Enable
	mac_config &= ~(0x1 << 23);	// IST disable

	mac_config |= (0x1 << 22);	// Strip vlan tag
	//mac_config &= ~(0x1 << 22);	// Keep vlan tag

	mac_config |= (0x1 << 21);	// Accept CRC error pkt
	//mac_config &= ~(0x1 << 21);	// Disacrd CRC error pkt

	mac_config |= (0x1 << 20);	// CRC strip
	//mac_config &= ~(0x1 << 20);	// Keep CRC

#ifdef CONFIG_STAR_JUMBO
	mac_config |= (0x1 << 18);	// Accept oversize pkt
#else
	mac_config &= ~(0x1 << 18);	// Discard oversize pkt
#endif

	mac_config &= ~(0x3 << 16);	// clear, set 1518

#ifdef CONFIG_STAR_JUMBO
	mac_config |= (0x3 << 16);	//set reserved, for jumbo frame
#else
	mac_config |= (0x2 << 16);	// 1536
	//mac_config |= (0x1 << 16);	// 1522
#endif

	// IPG
	mac_config |= (0x1f << 10);

	// Do not skip 16 consecutive collisions pkt
	mac_config |= (0x1 << 9);	// allow to re-tx
	//mac_config &= ~(0x1 << 9);	// drop pkt

	mac_config |= (0x1 << 8);	// Fast retry
	//mac_config &= ~(0x1 << 8);	// standard

	NIC_MAC_CONTROL_REG = mac_config;

	return 0;
}

static int star_nic_fc_config(struct net_device *dev)
{
	u32 fc_config;

	fc_config = NIC_FLOW_CONTROL_CONFIG_REG;

	// Send pause on frame threshold
	fc_config &= ~(0xfff << 16);	// Clear
	fc_config |= (0x360 << 16);	// Set

	//fc_config |= (0x1 << 8);	// Enable UC_PAUSE
	fc_config &= ~(0x1 << 8);	// Disable UC_PAUSE

	fc_config |= (0x1 << 7);	// Enable Half Duplex backpressure
	//fc_config &= ~(0x1 << 7);	// Disable Half Duplex backpressure

    	//fc_config |= (0x1 << 6);	// CRS-based BP
	fc_config &= ~(0x1 << 6);	// Collision-based BP

	//fc_config |= (0x1 << 5);	// Enable max BP collision
	fc_config &= ~(0x1 << 5);	// Disable max BP collision

	// max BP collision count
	fc_config &= ~(0x1f);		// Clear
	fc_config |= (0xc);		// Set

	NIC_FLOW_CONTROL_CONFIG_REG = fc_config;

	return 0;
}

static int star_nic_phy_config(struct net_device *dev)
{
	struct star_nic_private *priv = netdev_priv(dev);
	u32 phy_config = NIC_PHY_CONTROL_REG1;
#ifdef CONFIG_STAR_NIC_PHY_VSC8601
	u32 phy_addr=0;
#endif /* CONFIG_STAR_NIC_PHY_VSC8601 */
	//int i;

#ifdef CONFIG_STAR_NIC_PHY_INTERNAL_PHY 
	printk("Star Internal PHY\n");

#if 0
	{
		u16 phy_data;
		// restart the internal phy
		star_nic_write_phy(STAR_NIC_PHY_ADDR, 0, 0x8000);
		while (1) {
			star_nic_read_phy(STAR_NIC_PHY_ADDR, 0, &phy_data);
			if ( (phy_data&0x8000) ==0x0000) { // phy now at normal mode
				break;
			}
		}
	}
#endif

	priv->phy_addr = STAR_NIC_PHY_ADDR;
	// set phy addr for auto-polling
	phy_config |= (priv->phy_addr & 0x1f) << 24;

	// set internal phy mode
	// internel 10/100 phy
	phy_config |= 0x1 << 18;

	// MII
	phy_config &= ~(0x1 << 17);

	// MAC mode
	phy_config &= ~(0x1 << 16);

	// config PHY LED bit[13:12]
	star_nic_read_phy(priv->phy_addr, 31, (u16 *)(&phy_config));
	phy_config &= ~(0x3 << 12); // clear LED control
	phy_config |= FE_PHY_LED_MODE;
	star_nic_write_phy(priv->phy_addr, 31, phy_config);
#endif
#ifdef CONFIG_STAR_NIC_PHY_VSC8601
	u16 phy_data;

	printk("VSC8601 Chip\n");

	// phy addr for auto-polling
	phy_config |= ((phy_addr & 0x1f) << 24);

	// set external phy mode
	phy_config &= ~(0x1 << 18);

	// set RGMII
	phy_config |= (0x1 << 17);

	// set MII interface
	phy_config &= ~(0x1 << 16);

	NIC_PHY_CONTROL_REG1 = phy_config;
//=========================================================

	priv->phy_addr = STAR_NIC_PHY_ADDR;
	// set phy addr for auto-polling
	phy_config |= (priv->phy_addr & 0x1f) << 24;

	// set external phy mode
	// MII/RGMII interface
	phy_config &= ~(0x1 << 18);

	// RGMII
	phy_config |= (0x1 << 17);

	// MAC mode
	phy_config &= ~(0x1 << 16);

	star_nic_read_phy(priv->phy_addr, 3, &phy_data);
	if ((phy_data & 0x000f) == 0x0000) { // type A chip
		u16 tmp16;

		printk("VSC8601 Type A Chip\n");
		star_nic_write_phy(priv->phy_addr, 31, 0x52B5);
		star_nic_write_phy(priv->phy_addr, 16, 0xAF8A);

		phy_data = 0x0;
		star_nic_read_phy(priv->phy_addr, 18, &tmp16);
		phy_data |= (tmp16 & ~0x0);
		star_nic_write_phy(priv->phy_addr, 18, phy_data);

		phy_data = 0x0008;
		star_nic_read_phy(priv->phy_addr, 17, &tmp16);
		phy_data |= (tmp16 & ~0x000C);
		star_nic_write_phy(priv->phy_addr, 17, phy_data);        	

		star_nic_write_phy(priv->phy_addr, 16, 0x8F8A);        	

		star_nic_write_phy(priv->phy_addr, 16, 0xAF86);        	

		phy_data = 0x0008;
		star_nic_read_phy(priv->phy_addr, 18, &tmp16);
		phy_data |= (tmp16 & ~0x000C);
		star_nic_write_phy(priv->phy_addr, 18, phy_data);        	

		phy_data = 0x0;
		star_nic_read_phy(priv->phy_addr, 17, &tmp16);
		phy_data |= (tmp16 & ~0x0);
		star_nic_write_phy(priv->phy_addr, 17, phy_data);        	

		star_nic_write_phy(priv->phy_addr, 16, 0x8F8A);        	

		star_nic_write_phy(priv->phy_addr, 16, 0xAF82);        	

		phy_data = 0x0;
		star_nic_read_phy(priv->phy_addr, 18, &tmp16);
		phy_data |= (tmp16 & ~0x0);
		star_nic_write_phy(priv->phy_addr, 18, phy_data);        	

		phy_data = 0x0100;
		star_nic_read_phy(priv->phy_addr, 17, &tmp16);
		phy_data |= (tmp16 & ~0x0180);
		star_nic_write_phy(priv->phy_addr, 17, phy_data);        	

		star_nic_write_phy(priv->phy_addr, 16, 0x8F82);        	

		star_nic_write_phy(priv->phy_addr, 31, 0x0);        	
           
		//Set port type: single port
		star_nic_read_phy(priv->phy_addr, 9, &phy_data);        	
		phy_data &= ~(0x1 << 10);
		star_nic_write_phy(priv->phy_addr, 9, phy_data);        	
	} else if ((phy_data & 0x000f) == 0x0001) { // type B chip
		printk("VSC8601 Type B Chip\n");
		star_nic_read_phy(priv->phy_addr, 23, &phy_data);
		phy_data |= ( 0x1 << 8); //set RGMII timing skew
		star_nic_write_phy(priv->phy_addr, 23, phy_data);
	}

	// change to extened registers
	star_nic_write_phy(priv->phy_addr, 31, 0x0001);

	star_nic_read_phy(priv->phy_addr, 28, &phy_data);
	phy_data &= ~(0x3 << 14); // set RGMII TX timing skew
	phy_data |= (0x3 << 14); // 2.0ns
	phy_data &= ~(0x3 << 12); // set RGMII RX timing skew
	phy_data |= (0x3 << 12); // 2.0ns
	star_nic_write_phy(priv->phy_addr, 28, phy_data);

	// change to normal registers
	star_nic_write_phy(priv->phy_addr, 31, 0x0000);

	// set TX and RX clock skew
	//NIC_TEST_0_REG = (0x2 << 2) | (0x2 << 0);

#endif

#ifdef CONFIG_STAR_NIC_PHY_IP101A
	// ICPlus IP101A
	printk("ICPlus IP101A\n");
	priv->phy_addr = 1;
	// set phy addr for auto-polling
	phy_config |= (priv->phy_addr & 0x1f) << 24;

	// set external phy mode
	// MII/RGMII interface
	phy_config &= ~(0x1 << 18);

	// MII
	phy_config &= ~(0x1 << 17);

	// MAC mode
	phy_config &= ~(0x1 << 16);
#endif

/* robin 080102 				*/
/* added ICPlus IP1001 support	*/
#ifdef CONFIG_STAR_NIC_PHY_IP1001
u16 phy_data;

u32 phy_addr = 1;

phy_config = NIC_PHY_CONTROL_REG1;

	// set phy addr for auto-polling
	phy_config |= ((phy_addr & 0x1f) << 24);

	// set external phy mode
	// MII/RGMII interface
	phy_config &= ~(0x1 << 18);

	// RGMII
	phy_config |= (0x1 << 17);

	// MAC mode
	phy_config &= ~(0x1 << 16);

 NIC_PHY_CONTROL_REG1 = phy_config;
    star_nic_read_phy(phy_addr,2,&phy_data);
    //printf("\n phy.reg2=0x%04x",phy_data);
	
#if 1//set AN capability


    star_nic_read_phy(phy_addr,4,&phy_data);

    phy_data &= ~(0xf<<5);//clear
    phy_data |= (0x1<<5); //10Half
    phy_data |= (0x1<<6); //10Full
    phy_data |= (0x1<<7); //100Half
    phy_data |= (0x1<<8); //100Full
//    phy_data &= ~(0x1<<10); //FC off
    phy_data |= (0x1<<10); //FC on
    star_nic_write_phy(phy_addr,4,phy_data);


    star_nic_read_phy(phy_addr,9,&phy_data);

    phy_data |= (0x1<<9); //1000Full on

    phy_data &= ~(0x1<<10); 

    phy_data |= (0x1<<12); 

    star_nic_write_phy(phy_addr,9,phy_data);




    star_nic_read_phy(phy_addr,16,&phy_data);

    phy_data &= ~(0x1<<11); //Smart function off

    phy_data |=  (0x1<<0); //TX delay

    phy_data |=  (0x1<<1); //RX delay

    star_nic_write_phy(phy_addr,16,phy_data);

    star_nic_read_phy(phy_addr,16,&phy_data);
    //printf("\n phy.reg16=0x%04x",phy_data);


//    Hal_Nic_Read_PHY(NIC_PHY_ADDRESS,20,&phy_data);
//
//    phy_data &= ~(0x1<<2); 
//
//    phy_data |=  (0x1<<9); 
//    Hal_Nic_Write_PHY(NIC_PHY_ADDRESS,20,phy_data);




    star_nic_read_phy(phy_addr,0,&phy_data);
    phy_data |= (0x1<<9); //re-AN
    star_nic_write_phy(phy_addr,0,phy_data);


    star_nic_read_phy(phy_addr,9,&phy_data);
    //printf("\n phy.reg9=0x%04x",phy_data);

  
#endif	
#endif // CONFIG_STAR_NIC_PHY_IP1001
/* robin 080102 - end of modification */



	phy_config |= (0x1 << 8); // AN On
	//phy_config &= ~(0x1 << 8); // AN off

	if (!((phy_config >> 8) & 0x1)) { // AN disbale
		// Force to FullDuplex mode
		phy_config &= ~(0x1 << 11); // Half

		// Force to 100Mbps mode
		phy_config &= ~(0x3 << 9); // clear to 10M
		phy_config |= (0x1 << 9); // set to 100M
	}

	// Force TX FlowCtrl On,in 1000M
	phy_config |= (0x1 << 13);

	// Force TX FlowCtrl On, in 10/100M
	phy_config |= (0x1 << 12);

	// Enable MII auto polling
	phy_config &= ~(0x1 << 7); // auto-polling enable
	//phy_config |= (0x1 << 7); // auto-polling disable

	NIC_PHY_CONTROL_REG1 = phy_config;
#if 1
	star_nic_phy_powerdown(dev);
#endif

	return 0;
}

static int star_nic_vlan_config(struct net_device *dev)
{
	u32 vlan_id;

	//1.Setup MyVLAN ID0_1
	vlan_id  = 0; //clear
	vlan_id |= (my_vlan_id[0].vid & 0x0fff);
	vlan_id |= ((my_vlan_id[1].vid & 0x0fff) << 16);
	NIC_MY_VLANID_0_1 = vlan_id;

	//2.Setup MyVLAN ID2_3
	vlan_id  = 0; //clear
	vlan_id |= (my_vlan_id[2].vid & 0x0fff);
	vlan_id |= ((my_vlan_id[3].vid & 0x0fff) << 16);
	NIC_MY_VLANID_2_3 = vlan_id;

	//3.Setup vlan_id control bits
	NIC_MY_VLANID_CONTROL_REG = ( (my_vlan_id[0].control << 0) |
		(my_vlan_id[1].control << 1) |
		(my_vlan_id[2].control << 2) |
		(my_vlan_id[3].control << 3) );

	return 0;
}

static int star_nic_arl_config(struct net_device *dev)
{
	u32 arl_config;

	arl_config = NIC_ARL_CONFIG_REG;
	arl_config |= (0x1 << 4); // Misc Mode ON
	//arl_config &= ~(0x1 << 4); // Misc Mode Off
	arl_config |= (0x1 << 3); // My MAC only enable
	arl_config &= ~(0x1 << 2); // Learn SA On
	arl_config &= ~(0x1 << 1); // Forward MC to CPU
	arl_config &= ~(0x1); // Hash direct mode
	NIC_ARL_CONFIG_REG = arl_config;

	return 0;
}

#if 0
static void star_nic_interrupt_disable(void)
{
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_STATUS_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_TXTC_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXRC_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_TXQE_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXQF_BIT_INDEX);
}
#endif /* Disable function star_nic_interrupt_disable */

static void star_nic_interrupt_enable(void)
{
#ifdef STAR_NIC_STATUS_ISR
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_STATUS_BIT_INDEX);
#endif
	
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_RXRC_BIT_INDEX);

#ifdef STAR_NIC_RXQF_ISR
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_RXQF_BIT_INDEX);
#endif
}

#if 0
static int star_nic_show_rxdesc(char *page)
{
	int i;
	int num = 0;
	u32 rxsd_current;
	int rxsd_index;
	STAR_NIC_RXDESC volatile *rxdesc_ptr = rxring.vir_addr;

	HAL_NIC_READ_RXSD(rxsd_current);
	rxsd_index = (rxsd_current - (u32)rxring.phy_addr) >> 4;

	num += sprintf(page + num, "rxring.cur_index: %d\n", rxring.cur_index);
	num += sprintf(page + num, "rxsd_index:       %d\n", rxsd_index);

	for (i = 0; i < STAR_NIC_MAX_RFD_NUM; i++) {
		num += sprintf(page + num, "rxring[%02d].cown ==> %d\n", i, rxdesc_ptr->cown);
		rxdesc_ptr++;
	}

	return num;
}
#endif /* Disable function star_nic_show_rxdesc */

#if 0
static int star_nic_show_txdesc(char *page)
{
	int i;
	int num = 0;
	u32 txsd_current;
	int txsd_index;
	STAR_NIC_TXDESC volatile *txdesc_ptr = txring.vir_addr;

	HAL_NIC_READ_TXSD(txsd_current);
	txsd_index = (txsd_current - (u32)txring.phy_addr) >> 4;

	num += sprintf(page + num, "txring.cur_index: %d\n", txring.cur_index);
	num += sprintf(page + num, "txsd_index:       %d\n", txsd_index);

	for (i = 0; i < STAR_NIC_MAX_TFD_NUM; i++) {
		num += sprintf(page + num, "txring[%02d].cown ==> %d\n", i, txdesc_ptr->cown);
		txdesc_ptr++;
	}

	return num;
}
#endif /* star_nic_show_txdesc */

static int star_nic_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct star_nic_private *priv = netdev_priv(STAR_NIC_LAN_DEV);
	int num = 0;

	star_nic_mib_read(STAR_NIC_LAN_DEV);
	num += sprintf(page + num, "mib_rx_ok_pkt          %08d\n", priv->mib_info.mib_rx_ok_pkt);
	num += sprintf(page + num, "mib_rx_ok_byte         %08d\n", priv->mib_info.mib_rx_ok_byte);
	num += sprintf(page + num, "mib_rx_runt            %08d\n", priv->mib_info.mib_rx_runt);
	num += sprintf(page + num, "mib_rx_over_size       %08d\n", priv->mib_info.mib_rx_over_size);
	num += sprintf(page + num, "mib_rx_no_buffer_drop  %08d\n", priv->mib_info.mib_rx_no_buffer_drop);
	num += sprintf(page + num, "mib_rx_crc_err         %08d\n", priv->mib_info.mib_rx_crc_err);
	num += sprintf(page + num, "mib_rx_arl_drop        %08d\n", priv->mib_info.mib_rx_arl_drop);
	num += sprintf(page + num, "mib_rx_myvid_drop      %08d\n", priv->mib_info.mib_rx_myvid_drop);
	num += sprintf(page + num, "mib_rx_csum_err        %08d\n", priv->mib_info.mib_rx_csum_err);
	num += sprintf(page + num, "mib_rx_pause_frame     %08d\n", priv->mib_info.mib_rx_pause_frame);
	num += sprintf(page + num, "mib_tx_ok_pkt          %08d\n", priv->mib_info.mib_tx_ok_pkt);
	num += sprintf(page + num, "mib_tx_ok_byte         %08d\n", priv->mib_info.mib_tx_ok_byte);
	num += sprintf(page + num, "mib_tx_pause_frame     %08d\n", priv->mib_info.mib_tx_pause_frame);

	//num += star_nic_show_rxdesc(page + num);
	//num += star_nic_show_txdesc(page + num);

	return num;
}


static int
star_nic_write_proc(struct file *file, const char __user *buffer,
	unsigned long count, void *data)
{
	char *str;
	char *cmd;

	if (count > 0) {
		str = (char *)buffer,
		cmd = strsep(&str, "\t \n");
		if (!cmd) goto err_out;
		if (strcmp(cmd, "clear") == 0) {
			struct star_nic_private *priv = netdev_priv(STAR_NIC_LAN_DEV);

			star_nic_mib_read(STAR_NIC_LAN_DEV);
			memset(&priv->mib_info,0,sizeof(priv->mib_info));

		//} else if (strcmp(cmd, "write") == 0) {
		} else {
			goto err_out;
		}
	}

	return count;

err_out:
	return -EFAULT;
}

static void star_nic_phy_powerdown(struct net_device *dev)
{
	struct star_nic_private *priv = netdev_priv(dev);
	u16 phy_data = 0;
	// power down the PHY
	star_nic_read_phy(priv->phy_addr, 0, &phy_data);
	phy_data |= (0x1 << 11);
	star_nic_write_phy(priv->phy_addr, 0, phy_data);

	// set hight
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << 15);
	// set low
	PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << 15);
}

static void star_nic_phy_powerup(struct net_device *dev)
{
	struct star_nic_private *priv = netdev_priv(dev);
	u16 phy_data = 0;
	// power up the PHY
	star_nic_read_phy(priv->phy_addr, 0, &phy_data);
	phy_data &= ~(0x1 << 11);
	star_nic_write_phy(priv->phy_addr, 0, phy_data);

	// set hight
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << 15);
}

static void star_nic_enable(struct net_device *dev)
{
#if 0
	// enable NIC clock
	HAL_PWRMGT_ENABLE_NIC_CLOCK();
	NIC_MAC_CONTROL_REG &= ~((u32)0x3 << 30);
	udelay(100);
#endif

	star_nic_interrupt_enable();
	HAL_NIC_RX_DMA_START();
#if 1
	star_nic_phy_powerup(dev);
#endif
#ifdef CONFIG_INTERNEL_PHY_PATCH
	printk("%s: starting patch check.\n", __FUNCTION__);
	internal_phy_patch_check(1);
	mod_timer(&internal_phy_timer, jiffies + INTERNAL_PHY_PATCH_CHECK_PERIOD / 10);
#endif

}

static void star_nic_shutdown(struct net_device *dev)
{
	if (install_isr_account == 0) {
		DBG_PRINT("disable port 0\n");
		HAL_NIC_RX_DMA_STOP();
		HAL_NIC_TX_DMA_STOP();
#if 0
		NIC_MAC_CONTROL_REG |= ((u32)0x1 << 31);
		while (!(NIC_MAC_CONTROL_REG & (0x1 << 29))) {
			udelay(1000);
		}
		HAL_PWRMGT_DISABLE_NIC_CLOCK();
		NIC_MAC_CONTROL_REG |= (0x1 <<29);
#endif
#ifdef CONFIG_INTERNEL_PHY_PATCH
	printk("%s: stoping patch check.\n", __FUNCTION__);
	del_timer_sync(&internal_phy_timer);
#endif
	}
}

IRQ_RETURN star_nic_receive_isr(int irq, void *dev_id, struct pt_regs *regs)
{
#ifdef CONFIG_STAR_NIC_NAPI
#if 0
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_NIC_RXRC_BIT_INDEX);
#endif
	if (!test_bit(__LINK_STATE_RX_SCHED, &STAR_NIC_LAN_DEV->state)) {
#if 0
		disable_irq(INTC_NIC_RXRC_BIT_INDEX);
#endif
		if (likely(netif_rx_schedule_prep(CUR_NAPI_DEV))) {
			__netif_rx_schedule(CUR_NAPI_DEV);
		} else {
#if 0
			enable_irq(INTC_NIC_RXRC_BIT_INDEX);
#endif
		}
	}
#else
#ifndef CONFIG_VIC_INTERRUPT
	// TODO: mask interrupt
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_NIC_RXRC_BIT_INDEX);
#endif
	// MASK Interrupt
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXRC_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXQF_BIT_INDEX);

	star_nic_receive_packet(0); // Receive Once

	// TODO: unmask interrupt
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_RXRC_BIT_INDEX);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_RXQF_BIT_INDEX);
#endif

	return IRQ_HANDLED;
}

#ifdef STAR_NIC_RXQF_ISR
IRQ_RETURN star_nic_rxqf_isr(int irq, void *dev_id, struct pt_regs *regs)
{
#ifndef CONFIG_VIC_INTERRUPT
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_NIC_RXQF_BIT_INDEX);
#endif
#ifdef CONFIG_STAR_NIC_NAPI
	// because in normal state, fsql only invoke once and set_bit is atomic function.
	// so I don't mask it.
	set_bit(0, (unsigned long *)&is_qf);
	if (!test_bit(__LINK_STATE_RX_SCHED, &STAR_NIC_LAN_DEV->state)) {
#if 0
		disable_irq(INTC_NIC_RXRC_BIT_INDEX);
#endif
		if (likely(netif_rx_schedule_prep(CUR_NAPI_DEV))) {
			__netif_rx_schedule(CUR_NAPI_DEV);
		} else {
#if 0
			enable_irq(INTC_NIC_RXRC_BIT_INDEX);
#endif
		}
	}
#else
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXRC_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXQF_BIT_INDEX);
	
	star_nic_receive_packet(1); // Receive at Queue Full Mode

	// TODO: unmask interrupt
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_RXRC_BIT_INDEX);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_RXQF_BIT_INDEX);
#endif

	return IRQ_HANDLED;
}
#endif

#ifdef STAR_NIC_STATUS_ISR
#ifdef STAR_NIC_PRINT_ISR_STATUS
static char *star_nic_status_tbl[] = {
	"\nTX buffer under run.\n",
	"\nRX buffer full.\n",
	"\nMAC port change link state.\n",
	"\nMIB counter reach 0x80000000.\n",
	"\nMagic packet received.\n",
};
#endif /* STAR_NIC_PRINT_ISR_STATUS */

IRQ_RETURN star_nic_status_isr(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 int_status;
#ifdef STAR_NIC_PRINT_ISR_STATUS
	u32 i;
#endif /* STAR_NIC_PRINT_ISR_STATUS */
#ifdef CONFIG_PM
	extern int nic_suspended;
	u32 nic_suspended_tmp=nic_suspended;
#endif
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_STATUS_BIT_INDEX);
	HAL_NIC_READ_INTERRUPT_STATUS(int_status);

	//printk("%s: NIC status:%08X \n",__FUNCTION__, int_status);

	if (int_status & 0x8) {
		star_nic_mib_read((struct net_device *)STAR_NIC_LAN_DEV);
	}

#ifdef CONFIG_PM
	if ((int_status & 0x10)&&nic_suspended_tmp) {
//		printk("W\n");
		str8100_nic_resume();
	}
#endif

#ifdef STAR_NIC_PRINT_ISR_STATUS
	for (i = 0; i < 5; i++) {
		if (int_status & (1 << i)) {
			DO_PRINT(star_nic_status_tbl[i]);
		}
	}
#endif /* STAR_NIC_PRINT_ISR_STATUS */

	HAL_NIC_CLEAR_INTERRUPT_STATUS_SOURCES(int_status);
#ifdef CONFIG_PM
	if ((int_status & 0x10)&&nic_suspended_tmp) {
		str8100_nic_suspend(0);
	}
#endif
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_NIC_STATUS_BIT_INDEX);

	return IRQ_HANDLED;
}
#endif // STAR_NIC_STATUS_ISR

static int star_nic_uninstall_isr(struct net_device *dev)
{
	--install_isr_account;
	if (install_isr_account == 0) {
		printk("star nic uninstall isr\n");
		free_irq(INTC_NIC_RXRC_BIT_INDEX, STAR_NIC_LAN_DEV);

#ifdef STAR_NIC_RXQF_ISR
		free_irq(INTC_NIC_RXQF_BIT_INDEX, STAR_NIC_LAN_DEV);
#endif

#ifdef STAR_NIC_STATUS_ISR
		free_irq(INTC_NIC_STATUS_BIT_INDEX, STAR_NIC_LAN_DEV);
#endif
	}

	return 0;
}

static int star_nic_install_isr(struct net_device *dev)
{
	int retval;

	if (install_isr_account == 0) {
#ifdef STAR_NIC_DELAYED_INTERRUPT
		NIC_DELAYED_INT_CONFIG_REG = (1 << 16) | ((MAX_PEND_INT_CNT & 0xFF) << 8) | (MAX_PEND_TIME & 0xFF);
#endif
		retval = request_irq(INTC_NIC_RXRC_BIT_INDEX, &star_nic_receive_isr, SA_INTERRUPT, "NIC RXRC INT", STAR_NIC_LAN_DEV);

		if (retval) {
			DO_PRINT("%s: unable to get IRQ %d (irqval=%d).\n", "NIC RXRC INT", INTC_NIC_RXRC_BIT_INDEX, retval);
			return 1;
		}

#ifdef STAR_NIC_RXQF_ISR
		/*  QUEUE full interrupt handler */
		retval = request_irq(INTC_NIC_RXQF_BIT_INDEX, &star_nic_rxqf_isr, SA_INTERRUPT, "NIC RXQF INT", STAR_NIC_LAN_DEV);

		if (retval) {
			DO_PRINT("%s: unable to get IRQ %d (irqval=%d).\n", "NIC RXQF INT", INTC_NIC_RXQF_BIT_INDEX, retval);
			return 2;
		}
#endif	

#ifdef STAR_NIC_STATUS_ISR
		/*  NIC Status interrupt handler */
		retval = request_irq(INTC_NIC_STATUS_BIT_INDEX, &star_nic_status_isr, SA_INTERRUPT, "NIC STATUS", STAR_NIC_LAN_DEV);

		if (retval) {
			DO_PRINT("%s: unable to get IRQ %d (irqval=%d).\n", "NIC STATUS INT", INTC_NIC_STATUS_BIT_INDEX, retval);
			return 3;
		}
		//HAL_NIC_ENABLE_ALL_INTERRUPT_STATUS_SOURCES();
		HAL_NIC_DISABLE_ALL_INTERRUPT_STATUS_SOURCES();
		//Enable NIC Status Interrupt: MIB counter th (3)
		HAL_NIC_ENABLE_INTERRUPT_STATUS_SOURCE_BIT(3);

#endif
	} // end if(install_isr_account == 0)

	++install_isr_account;

	return 0;
}

static int star_nic_lan_open(struct net_device *dev)
{
	DBG_PRINT("%s:star_nic_lan_open\n", dev->name);

#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif

	CUR_NAPI_DEV = dev;

	netif_start_queue(dev);

	star_nic_install_isr(dev);

	star_nic_enable(dev);

	return 0;
}

static struct net_device_stats *star_nic_get_stats(struct net_device *dev)
{
	struct star_nic_private *priv = netdev_priv(dev);

	return &priv->stats;
}

static void star_nic_timeout(struct net_device *dev)
{
	DBG_PRINT("%s:star_nic_timeout\n", dev->name);
	netif_wake_queue(dev);
	dev->trans_start = jiffies;
}

static int star_nic_close(struct net_device *dev)
{
#if 1
	star_nic_phy_powerdown(dev);
#endif
	star_nic_uninstall_isr(dev);
	netif_stop_queue(dev);
	star_nic_shutdown(dev);

#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif

	CUR_NAPI_DEV = STAR_NIC_LAN_DEV;

	return 0;
}

static inline struct sk_buff *star_nic_alloc_skb(void)
{
	struct sk_buff *skb;

	skb = dev_alloc_skb(MAX_PACKET_LEN + 2);

	if (unlikely(!skb)) {
		printk("\n dev_alloc_skb fail!! while allocate RFD ring !!\n");
		return NULL;
	}

	/* Make buffer alignment 2 beyond a 16 byte boundary
	 * this will result in a 16 byte aligned IP header after
	 * the 14 byte MAC header is removed
	 */
	skb_reserve(skb, 2);	/* 16 bit alignment */

	return skb;
}

static void __init star_nic_buffer_free(void)
{
	int i;

	if (rxring.vir_addr) {
		for (i = 0; i < STAR_NIC_MAX_RFD_NUM; i++) {
			if (rxring.skb_ptr[i]) {
				dev_kfree_skb(rxring.skb_ptr[i]);
			}
		}
		pci_free_consistent(NULL, STAR_NIC_MAX_RFD_NUM * sizeof(STAR_NIC_RXDESC), rxring.vir_addr, rxring.phy_addr);
		memset((void *)&rxring, 0, STAR_NIC_MAX_RFD_NUM * sizeof(STAR_NIC_RXDESC));
	}

	if (txring.vir_addr) {
		pci_free_consistent(NULL, STAR_NIC_MAX_TFD_NUM * sizeof(STAR_NIC_TXDESC), txring.vir_addr, txring.phy_addr);
		memset((void *)&txring, 0, STAR_NIC_MAX_TFD_NUM * sizeof(STAR_NIC_TXDESC));
	}
}

static int __init star_nic_buffer_alloc(void)
{
	STAR_NIC_RXDESC	volatile *rxdesc_ptr;
	STAR_NIC_TXDESC	volatile *txdesc_ptr;
	struct sk_buff	*skb_ptr;
	int err;
	int i;

	rxring.vir_addr = pci_alloc_consistent(NULL, STAR_NIC_MAX_RFD_NUM * sizeof(STAR_NIC_RXDESC), &rxring.phy_addr);
	if (!rxring.vir_addr) {
		printk("\n ERROR: Allocate RFD Failed\n");
		err = -ENOMEM;
		goto err_out;
	}

	txring.vir_addr = pci_alloc_consistent(NULL, STAR_NIC_MAX_TFD_NUM * sizeof(STAR_NIC_TXDESC), &txring.phy_addr);
	if (!txring.vir_addr) {
		printk("\n ERROR: Allocate TFD Failed\n");
		err = -ENOMEM;
		goto err_out;
	}

	// Clean RX Memory
	memset((void *)rxring.vir_addr, 0, STAR_NIC_MAX_RFD_NUM * sizeof(STAR_NIC_RXDESC));
	DBG_PRINT("    rxring.vir_addr=0x%08X rxring.phy_addr=0x%08X\n", (u32)rxring.vir_addr, (u32)rxring.phy_addr);
	rxring.cur_index = 0;	// Set cur_index Point to Zero
	rxdesc_ptr = rxring.vir_addr;
	for (i = 0; i < STAR_NIC_MAX_RFD_NUM; i++, rxdesc_ptr++) {
		if (i == (STAR_NIC_MAX_RFD_NUM - 1)) { 
			rxdesc_ptr->eor = 1;	// End bit == 0;
		}
		skb_ptr = star_nic_alloc_skb();
		if (!skb_ptr) {
			printk("ERROR: Allocate skb Failed!\n");
			err = -ENOMEM;
			goto err_out;
		}
		// Trans Packet from Virtual Memory to Physical Memory
		rxring.skb_ptr[i]	= skb_ptr;
		rxdesc_ptr->data_ptr	= (u32)virt_to_phys(skb_ptr->data);
		rxdesc_ptr->length	= MAX_PACKET_LEN;
	}

	// Clean TX Memory
	memset((void *)txring.vir_addr, 0, STAR_NIC_MAX_TFD_NUM * sizeof(STAR_NIC_TXDESC));
	DBG_PRINT("    txring.vir_addr=0x%08X txring.phy_addr=0x%08X\n", (u32)txring.vir_addr, (u32)txring.phy_addr);
	txring.cur_index = 0;	// Set cur_index Point to Zero
	txdesc_ptr = txring.vir_addr;
	for (i = 0; i < STAR_NIC_MAX_TFD_NUM; i++, txdesc_ptr++) {
		if (i == (STAR_NIC_MAX_TFD_NUM - 1)) { 
			txdesc_ptr->eor = 1;	// End of Ring ==1
		}
		txdesc_ptr->cown = 1;	// TX Ring , Cown == 1

#ifdef STAR_NIC_TX_HW_CHECKSUM
		// Enable Checksum
		txdesc_ptr->ico		= 1;
		txdesc_ptr->uco		= 1;
		txdesc_ptr->tco		= 1;
#else
		txdesc_ptr->ico		= 0;
		txdesc_ptr->uco		= 0;
		txdesc_ptr->tco		= 0;
#endif
		txring.skb_ptr[i] 	= NULL;	// clear txring.skb_ptr
	}

	return 0;

err_out:
	star_nic_buffer_free();
	return err;
}


#ifdef CONFIG_STAR_JUMBO
#define MINIMUM_ETHERNET_FRAME_SIZE	64
#define MAX_JUMBO_FRAME_SIZE		2036
#define ENET_HEADER_SIZE			14
#define ETHERNET_FCS_SIZE			4
#define ETHERNET_VLAN_SIZE			4
#define NET_IP_ALIGN				2
static int
str8100_change_mtu(struct net_device *netdev, int new_mtu)
{
printk("%s: new_mtu=%d\n",__FUNCTION__,new_mtu);
	int max_frame = new_mtu + ENET_HEADER_SIZE + ETHERNET_FCS_SIZE+ETHERNET_VLAN_SIZE;
	if ((max_frame < MINIMUM_ETHERNET_FRAME_SIZE) ||
	    (max_frame > MAX_JUMBO_FRAME_SIZE)) {
		printk("%s: Invalid MTU setting (%d)\n",__FUNCTION__,new_mtu);
		return -EINVAL;
	}
	netdev->mtu = new_mtu;

//	star_nic_buffer_free();
//	star_nic_buffer_alloc();

//	if (netif_running(netdev))
//		e1000_reinit_locked(adapter);

	return 0;

}
#endif

#ifdef CONFIG_STAR_NIC_NAPI
static int star_nic_poll(struct net_device *netdev, int *budget)
{
	int work_done = 0;
	int work_to_do = min(*budget, netdev->quota); // where is min define

	star_nic_receive_packet(0, &work_done, work_to_do);

	*budget -= work_done;
	netdev->quota -= work_done;

	/* if no Tx and not enough Rx work done, exit the polling mode */
	if (work_done) {
		if (test_bit(0, (unsigned long *)&is_qf) == 1) { // queue full
			clear_bit(0, (unsigned long *)&is_qf);
			HAL_NIC_RX_DMA_START();
			return 1;
		}
	} else {
		netif_rx_complete(CUR_NAPI_DEV);
#ifdef CONFIG_STAR_NIC_NAPI_MASK_IRQ
		enable_irq(INTC_NIC_RXRC_BIT_INDEX);
#endif
		return 0;
	}

	return 1;
}
#endif

static int star_nic_get_rfd_buff(int index)
{
	struct star_nic_private *priv;
	STAR_NIC_RXDESC volatile *rxdesc_ptr;
	struct sk_buff *skb_ptr;
	unsigned char *data;
	int len;

	//TODO: get rxdesc ptr
	rxdesc_ptr = rxring.vir_addr + index;
	skb_ptr = rxring.skb_ptr[index];
#ifdef CONFIG_STAR_JUMBO
	if (rxdesc_ptr->fs != 1 || rxdesc_ptr->ls != 1) {
		goto freepacket;
	}
#endif
	len = rxdesc_ptr->length;

	consistent_sync(skb_ptr->data, len, PCI_DMA_FROMDEVICE);

	data = skb_put(skb_ptr, len);

	skb_ptr->dev = STAR_NIC_LAN_DEV;

	priv = netdev_priv(skb_ptr->dev);

#ifdef STAR_NIC_RX_HW_CHECKSUM
	if (rxdesc_ptr->ipf == 1 || rxdesc_ptr->l4f == 1) {
		if (rxdesc_ptr->prot != 0x11) {
			skb_ptr->ip_summed = CHECKSUM_NONE;
		} else {
			// CheckSum Fail
			priv->stats.rx_errors++;
			goto freepacket;
		}
	} else {
			skb_ptr->ip_summed = CHECKSUM_UNNECESSARY;
	}
#else
	skb_ptr->ip_summed = CHECKSUM_NONE;
#endif

	// this line must, if no, packet will not send to network layer
	skb_ptr->protocol = eth_type_trans(skb_ptr, skb_ptr->dev);

	priv->stats.rx_packets++;
	priv->stats.rx_bytes += len;
	skb_ptr->dev->last_rx = jiffies;

	// if netif_rx any package, will let this driver core dump.
#ifdef CONFIG_STAR_NIC_NAPI
	netif_receive_skb(skb_ptr);
#else
	netif_rx(skb_ptr);
#endif

	return 0;

freepacket:
	dev_kfree_skb_any(skb_ptr);
	return 0;
}

#ifdef CONFIG_STAR_NIC_NAPI
void star_nic_receive_packet(int mode, int *work_done, int work_to_do)
#else
void star_nic_receive_packet(int mode)
#endif
{
	int rxsd_index;
	u32 rxsd_current;
	STAR_NIC_RXDESC volatile *rxdesc_ptr = rxring.vir_addr + rxring.cur_index;
	struct sk_buff *skb_ptr;
#ifndef CONFIG_STAR_NIC_NAPI
	int rxqf = 0; // Queue Full Mode =0
#endif
	int i, rxcount = 0;
	HAL_NIC_READ_RXSD(rxsd_current);
	rxsd_index = (rxsd_current - (u32)rxring.phy_addr) >> 4;

	if (rxsd_index > rxring.cur_index) {
		rxcount = rxsd_index - rxring.cur_index;
	} else if (rxsd_index < rxring.cur_index) {
		rxcount = (STAR_NIC_MAX_RFD_NUM - rxring.cur_index) + rxsd_index;
	} else {
		if (rxdesc_ptr->cown == 0) {
			goto receive_packet_exit;
		} else {
			// Queue Full
#ifndef CONFIG_STAR_NIC_NAPI
			rxqf = 1;
#endif
			rxcount = STAR_NIC_MAX_RFD_NUM;
		}
	}

#ifndef CONFIG_STAR_NIC_NAPI
	if (mode == 1) {
		rxqf = 1;
		rxcount = STAR_NIC_MAX_RFD_NUM;
	}
#endif

	for (i = 0; i < rxcount; i++) {
#ifdef CONFIG_STAR_NIC_NAPI
		if (*work_done >= work_to_do)
			break;
		++(*work_done);
#endif
		if (rxdesc_ptr->cown != 0) {
			// Alloc New skb_buff 
			skb_ptr = star_nic_alloc_skb();
			// Check skb_buff
			if (skb_ptr != NULL) {
				star_nic_get_rfd_buff(rxring.cur_index);
				rxring.skb_ptr[rxring.cur_index] = skb_ptr;
				rxdesc_ptr->data_ptr	= (u32)virt_to_phys(skb_ptr->data);
				rxdesc_ptr->length	= MAX_PACKET_LEN;	
				rxdesc_ptr->cown	= 0; // set cbit to 0 for CPU Transfer	
			} else {
				// TODO:
				// I will add dev->lp.stats->rx_dropped, it will effect the performance
				DBG_PRINT("%s: Alloc sk_buff fail, reuse the buffer\n", __FUNCTION__);
				rxdesc_ptr->cown	= 0; // set cbit to 0 for CPU Transfer	
				return;
			}
		} else {
			//printk("[KC_DEBUG] star_nic_receive_packet() encounter COWN==0 BUG\n");
		}

		if (rxring.cur_index == (STAR_NIC_MAX_RFD_NUM - 1)) {
			rxring.cur_index	= 0;
			rxdesc_ptr		= rxring.vir_addr;
		} else {
			rxring.cur_index++;
			rxdesc_ptr++;
		}
	}

#ifndef CONFIG_STAR_NIC_NAPI
	if (rxqf) {
		rxring.cur_index = rxsd_index;
		mb();
		HAL_NIC_RX_DMA_START();
	}
#endif

receive_packet_exit:
	return;
}

#ifdef FREE_TX_SKB_MULTI
#define FREE_TX_SKB_MULTI_MAX   16
#define MAX_TX_SKB_FREE_NUM     FREE_TX_SKB_MULTI_MAX + MAX_SKB_FRAGS
#endif

#define FIX_NFS
static int star_nic_send_packet(struct sk_buff *skb, struct net_device *dev)
{
#if defined(STAR_NIC_TX_HW_CHECKSUM) && defined(MAX_SKB_FRAGS) && defined(STAR_NIC_SG)
	/*********************************************************
	 * Scatter/gather sk_buff
	 *********************************************************/
	struct star_nic_private *priv = netdev_priv(dev);
	STAR_NIC_TXDESC volatile *txdesc_ptr;
	unsigned long flags;

#ifdef FREE_TX_SKB_MULTI
	int i;
	int tssd_index;
	int tssd_current;
	int skb_free_count = 0;
	struct sk_buff *skb_free[MAX_TX_SKB_FREE_NUM];
#endif

#if defined(STAR_NIC_TX_HW_CHECKSUM) && defined(MAX_SKB_FRAGS) && defined(STAR_NIC_SG)
	int org_index;
	int cur_index;
#ifdef FIX_NFS
	int padding_size;
#endif

	unsigned int f;
	unsigned int nr_frags = skb_shinfo(skb)->nr_frags;
	unsigned int len = skb->len - skb->data_len;
	unsigned int offset;

#ifndef FREE_TX_SKB_MULTI
	int skb_free_count = 0;
	struct sk_buff *skb_free[MAX_SKB_FRAGS];
#endif
#else
#ifndef FREE_TX_SKB_MULTI
	struct sk_buff *skb_free;
#endif
#endif

	spin_lock_irqsave(&star_nic_send_lock, flags);

#ifdef FREE_TX_SKB_MULTI
		int count = 0;

		for (i = 0; i < FREE_TX_SKB_MULTI_MAX; i++) {
			txdesc_ptr = txring.vir_addr + txring.to_free_index;
			if (txdesc_ptr->cown == 0) {
				break;
			}
			if (txring.skb_ptr[txring.to_free_index]) {
				skb_free[count++] = txring.skb_ptr[txring.to_free_index];
				txring.skb_ptr[txring.to_free_index] = NULL;
			}

			increase_cyclic(txring.to_free_index, STAR_NIC_MAX_TFD_NUM);

			if (count == FREE_TX_SKB_MULTI_MAX) {
				break;
			}
		}
		skb_free_count = count;
#endif

#if defined(STAR_NIC_TX_HW_CHECKSUM) && defined(MAX_SKB_FRAGS) && defined(STAR_NIC_SG)
	/****************************************************** 
	 *     Scan txring for enough free space to write new
	 * packet.
	 *
	 * The total number of fragments is (nr_grags + 1).
	 ******************************************************/
	org_index = txring.cur_index;
	cur_index = txring.cur_index;
	for (f = 0; f < (nr_frags + 1); f++) {
		txdesc_ptr = txring.vir_addr + cur_index;

		if (txdesc_ptr->cown == 0) {
			/* cown=0, txdesc is owned by DMA controller.
			 * CPU should not modify txring.                       */
			spin_unlock_irqrestore(&star_nic_send_lock, flags);
#ifdef FREE_TX_SKB_MULTI
			/* Free unused skb when tx queue is full to
			 * save CPU time.                                      */
			for (i = 0; i < skb_free_count; i++) {
				dev_kfree_skb(skb_free[i]);
			}
#endif
			/* Not enough tx buffer, re-queue the skb.             */
			return NETDEV_TX_BUSY;
		}

		if (txring.skb_ptr[cur_index]) {
			skb_free[skb_free_count++] = txring.skb_ptr[cur_index];
		}

#if defined(FREE_TX_SKB_MULTI) || defined(STAR_NIC_TIMER)
		if(cur_index==txring.to_free_index)
			increase_cyclic(txring.to_free_index, STAR_NIC_MAX_TFD_NUM);
#endif

		if (f == 0) {
			txdesc_ptr->fs		= 1;
		} else {
			txdesc_ptr->fs		= 0;
		}
		if (f == nr_frags) {
			txdesc_ptr->ls		= 1;
		} else {
			txdesc_ptr->ls		= 0;
		}

		increase_cyclic(cur_index, STAR_NIC_MAX_TFD_NUM);
	}

	/*********************************************************
	 * ====== First data buffer of sk_buff ======
	 * Same as none scatter/gather sk_buff.
	 *********************************************************/
	txdesc_ptr = (txring.vir_addr + txring.cur_index);
	txdesc_ptr->data_ptr			= virt_to_phys(skb->data);
#ifdef FIX_NFS
	padding_size = ETH_ZLEN - len;
	txdesc_ptr->length = len;
#else
	if ((nr_frags == 0) && (len < PKT_MIN_SIZE)) {
		txdesc_ptr->length		= PKT_MIN_SIZE;
		memset(skb->data + len, 0x00, PKT_MIN_SIZE - len);
	} else {
		txdesc_ptr->length		= len;
	}
#endif // FIX_NFS

	/* Only the last fragment frees the skb, to prevent
	 * release buffer before send all data to switch buffer. */
	if (nr_frags) {
		txring.skb_ptr[txring.cur_index]	= NULL;
	} else {
		txring.skb_ptr[txring.cur_index]	= skb;
	}
	consistent_sync(skb->data, txdesc_ptr->length, PCI_DMA_TODEVICE);

	increase_cyclic(txring.cur_index, STAR_NIC_MAX_TFD_NUM);

	/*********************************************************
	 * ====== Fragmented pages ======
	 * skb_shinfo(skb)->frags[]
	 *   page
	 *   page_offset
	 *   size
	 *********************************************************/
	for (f = 0; f < nr_frags; f++) {
		struct skb_frag_struct *frag; 
		txdesc_ptr = txring.vir_addr + txring.cur_index;
		frag = &skb_shinfo(skb)->frags[f]; 
#ifdef FIX_NFS
		padding_size -= frag->size;

		txdesc_ptr->data_ptr = virt_to_phys(page_address(frag->page) + frag->page_offset);
		txdesc_ptr->length = frag->size;
#else
		len = frag->size; 
		offset = frag->page_offset; 

		txdesc_ptr->data_ptr = virt_to_phys(page_address(frag->page) + offset);
		txdesc_ptr->length = len;
#endif // FIX_NFS

		/* Only the last fragment frees the skb. */
		if (f == (nr_frags - 1)) {
			txring.skb_ptr[txring.cur_index] = skb;
		} else {
			txring.skb_ptr[txring.cur_index] = NULL;
		}
		consistent_sync(page_address(frag->page) + offset, txdesc_ptr->length, PCI_DMA_TODEVICE);

		increase_cyclic(txring.cur_index, STAR_NIC_MAX_TFD_NUM);
	}

	/*********************************************************
	 * Set proper information of TX descriptor.
	 *********************************************************/
	for (f = 0; f < (nr_frags + 1); f++) {
		txdesc_ptr = txring.vir_addr + org_index;
		txdesc_ptr->cown = 0;
		org_index++;
		if (org_index == STAR_NIC_MAX_TFD_NUM) {
			org_index = 0;
		}
	}

#ifdef FIX_NFS
	if (padding_size > 0)
		/* Padding zero to the end of packet to meet minimum 
		 * packet size requirement.                       */
		txdesc_ptr->length += padding_size;
#endif // FIX_NFS

#else
	txdesc_ptr = txring.vir_addr + txring.cur_index;

	if (txdesc_ptr->cown == 0) { // This TFD is busy
		spin_unlock_irqrestore(&star_nic_send_lock, flags);
		// re-queue the skb
		return 1;
	}

#ifndef FREE_TX_SKB_MULTI
	if (txring.skb_ptr[txring.cur_index]) {
		// MUST TODO: Free skbuff
		skb_free = txring.skb_ptr[txring.cur_index];
	}
#endif

	txdesc_ptr->fs		= 1;
	txdesc_ptr->ls		= 1;

	txring.skb_ptr[txring.cur_index]	= skb;
	txdesc_ptr->data_ptr			= virt_to_phys(skb->data);
	if (skb->len < PKT_MIN_SIZE) {
		txdesc_ptr->length		= PKT_MIN_SIZE;
		memset(skb->data + skb->len, 0x00, PKT_MIN_SIZE - skb->len);
	} else {
		txdesc_ptr->length		= skb->len;
	}

	consistent_sync(skb->data, txdesc_ptr->length, PCI_DMA_TODEVICE);

	increase_cyclic(txring.cur_index, STAR_NIC_MAX_TFD_NUM);

	txdesc_ptr->cown	= 0;
#endif

	mb();
	HAL_NIC_TX_DMA_START();

	priv->stats.tx_packets++;
	priv->stats.tx_bytes += skb->len;
	dev->trans_start = jiffies;

sendpacket_exit:
	spin_unlock_irqrestore(&star_nic_send_lock, flags);

#ifdef FREE_TX_SKB_MULTI
	for (i = 0; i < skb_free_count; i++) {
		dev_kfree_skb(skb_free[i]);
	}
#else
#if defined(STAR_NIC_TX_HW_CHECKSUM) && defined(MAX_SKB_FRAGS) && defined(STAR_NIC_SG)
	for (f = 0; f < skb_free_count; f++) {
		dev_kfree_skb(skb_free[f]);
	}
#else
	if (skb_free) {
		dev_kfree_skb(skb_free);
	}
#endif
#endif

	return NETDEV_TX_OK;

#else
//=============================================================================
	/*********************************************************
	 * None scatter/gather sk_buff
	 *********************************************************/
	struct star_nic_private *priv = netdev_priv(dev);
	STAR_NIC_TXDESC volatile *txdesc_ptr = (txring.vir_addr + txring.cur_index);
	struct sk_buff *skb_free = NULL;
	unsigned long flags;

	spin_lock_irqsave(&star_nic_send_lock, flags);

	if (txdesc_ptr->cown == 0) { // This TFD is busy
		spin_unlock_irqrestore(&star_nic_send_lock, flags);
		// re-queue the skb
		return NETDEV_TX_BUSY;
	}

	if (txdesc_ptr->data_ptr != 0) {
		// MUST TODO: Free skbuff
		skb_free = txring.skb_ptr[txring.cur_index];
#ifdef STAR_NIC_TIMER
		txring.to_free_index = txring.cur_index + 1;
		if (txring.to_free_index == STAR_NIC_MAX_TFD_NUM) {
			txring.to_free_index = 0;
		}
#endif
	}

#ifdef STAR_NIC_TX_HW_CHECKSUM
	if (skb->protocol == __constant_htons(ETH_P_IP)) {
		if (skb->nh.iph->protocol == IPPROTO_UDP) {
			txdesc_ptr->uco = 1;
			txdesc_ptr->tco = 0;
			//printk("[KC DEBUG] UDP PACKET\n");
		} else if (skb->nh.iph->protocol == IPPROTO_TCP) {
			txdesc_ptr->uco = 0;
			txdesc_ptr->tco = 1;
			//printk("[KC DEBUG] TCP PACKET\n");
		} else {
			txdesc_ptr->uco = 0;
			txdesc_ptr->tco = 0;
			//printk("[KC DEBUG] NOT TCP&UDP PACKET\n");
		}
	} else {
#if 0
		if (skb->protocol == __constant_htons(ETH_P_ARP)) {
			printk("[KC DEBUG] ARP PACKET\n");
		} else {
			printk("[KC DEBUG] NOT IP PACKET\n");
		}
#endif
		txdesc_ptr->ico = 0;
		txdesc_ptr->uco = 0;
		txdesc_ptr->tco = 0;
	}
#endif

	txring.skb_ptr[txring.cur_index]	= skb;
	txdesc_ptr->data_ptr			= virt_to_phys(skb->data);
	if (skb->len < PKT_MIN_SIZE) {
		txdesc_ptr->length		= PKT_MIN_SIZE;
		memset(skb->data + skb->len, 0x00, PKT_MIN_SIZE - skb->len);
	} else {
		txdesc_ptr->length		= skb->len;
	}

	consistent_sync(skb->data, txdesc_ptr->length, PCI_DMA_TODEVICE);

	txdesc_ptr->fs		= 1;
	txdesc_ptr->ls		= 1;
	// Wake interrupt
	txdesc_ptr->intr	= 0;
	txdesc_ptr->cown	= 0;

	mb();
	HAL_NIC_TX_DMA_START();

	priv->stats.tx_packets++;
	priv->stats.tx_bytes += skb->len;
	dev->trans_start = jiffies;

	if (txring.cur_index == (STAR_NIC_MAX_TFD_NUM - 1)) {
		txring.cur_index = 0;
	} else {
		txring.cur_index++;
	}

sendpacket_exit:
	spin_unlock_irqrestore(&star_nic_send_lock, flags);
	if (skb_free) {
		dev_kfree_skb(skb_free);
	}

#ifdef STAR_NIC_TIMER
	mod_timer(&star_nic_timer, jiffies + 10);
#endif
	return NETDEV_TX_OK;
#endif
}

static void star_nic_set_mac_addr(struct net_device *dev, const char *mac_addr)
{
	memcpy(dev->dev_addr, mac_addr, 6);

	NIC_MY_MAC_HIGH_BYTE_REG =
	       	(mac_addr[0] << 8) |
		 mac_addr[1];

	NIC_MY_MAC_LOW_BYTE_REG =
	       	(mac_addr[2] << 24) |
		(mac_addr[3] << 16) |
		(mac_addr[4] << 8)  |
		(mac_addr[5]);

	printk("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
		mac_addr[0],
		mac_addr[1],
		mac_addr[2],
		mac_addr[3],
		mac_addr[4],
		mac_addr[5]);
}

static int star_nic_set_lan_mac_addr(struct net_device *dev, void *addr)
{
	struct sockaddr *sock_addr = addr;
	struct star_nic_private *priv = netdev_priv(dev);

	spin_lock_irq(&priv->lock);
	star_nic_set_mac_addr(dev, sock_addr->sa_data);
	spin_unlock_irq(&priv->lock);

	return 0;
}


#ifdef CONFIG_PM
static int nic_suspended=0;
void inline str8100_wol_enter(){

	//stop tx dma
	HAL_NIC_TX_DMA_STOP();
	//Enable Internal Loopback mode
	NIC_TEST_1_REG|=(0x1<<18);

	//WoL
	NIC_MAC_CONTROL_REG |= (0x1 << 30); //assert WoL bit


	while (!((NIC_MAC_CONTROL_REG>>29)&0x1)){
		udelay(500);
		NIC_MAC_CONTROL_REG |= (0x1 << 30); //assert WoL bit
		NIC_MAC_CONTROL_REG &= ~(0x1 << 30); //de-assert WoL bit
		NIC_MAC_CONTROL_REG |= (0x1 << 30); //assert WoL bit
		udelay(500);

//		if((NIC_TEST_1_REG&(0x1fff))!=4096){
//printk("%.8x\n",NIC_TEST_1_REG);
			u32 test=0;
			test=test=NIC_TEST_1_REG;
//		}
	}
	//Disable Internal Loopback mode
	NIC_TEST_1_REG&=~(0x1<<18);

	//Enable Magic packet received of Nic status Int	
	HAL_NIC_ENABLE_INTERRUPT_STATUS_SOURCE_BIT(4);

	NIC_MAC_CONTROL_REG |= (0x1 <<29);//write "1" clear
	HAL_PWRMGT_DISABLE_NIC_CLOCK();
}

void inline str8100_wol_exit(){
	HAL_NIC_DISABLE_INTERRUPT_STATUS_SOURCE_BIT(4);
	HAL_PWRMGT_ENABLE_NIC_CLOCK();
	NIC_MAC_CONTROL_REG &= ~(0x1 << 30); //de-assert power down bit

	HAL_NIC_CLEAR_INTERRUPT_STATUS_SOURCES((0x1<<4));
	HAL_NIC_TX_DMA_START();
}

int inline str8100_nic_suspend(pm_message_t state)
{
	if(!netif_running(STAR_NIC_LAN_DEV)) return 0;
	nic_suspended=1;

#ifdef CONFIG_PM_DEBUG
	printk("%s:\n",__FUNCTION__);
#endif
	netif_device_detach(STAR_NIC_LAN_DEV);
/*
	if (netif_running(STAR_NIC_LAN_DEV)) {
		WARN_ON(test_bit(__E1000_RESETTING, &adapter->flags));
		e1000_down(adapter);
	}
*/
	str8100_wol_enter();
	return 0;
}
int inline str8100_nic_resume()
{
	//Waked
	if(!nic_suspended) return 0;
	nic_suspended=0;
#ifdef CONFIG_PM_DEBUG
	printk("%s:\n",__FUNCTION__);
#endif
/*	if (netif_running(STAR_NIC_LAN_DEV))
		e1000_up(adapter);
*/
	netif_device_attach(STAR_NIC_LAN_DEV);

	str8100_wol_exit();	

	return 0;
}
#endif

static int star_nic_init(struct net_device *dev)
{
#if 1
	// set hight
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << 15);
	// set low
	PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << 15);
	// set high
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << 15);
#endif

#if 1
	// set NIC clock to 67.5MHz
	PWRMGT_SYSTEM_CLOCK_CONTROL_REG |= (0x1 << 7);
#else
	// set NIC clock to 125MHz
	PWRMGT_SYSTEM_CLOCK_CONTROL_REG &= ~(0x1 << 7);
#endif

	// enable NIC clock
	HAL_PWRMGT_ENABLE_NIC_CLOCK();
	//NIC_MAC_CONTROL_REG = 0x00527C00;
	udelay(100);

	// Configure GPIO for NIC MDC/MDIO pins
	HAL_MISC_ENABLE_MDC_MDIO_PINS();
	HAL_MISC_ENABLE_NIC_COL_PINS();
#ifdef CONFIG_STAR_NIC_PHY_INTERNAL_PHY
	MISC_GPIOA_PIN_ENABLE_REG |= (0x7 << 22);
	MISC_FAST_ETHERNET_PHY_CONFIG_REG |=  (FE_PHY_LED_MODE >> 12) & 0x3;

	// set hight
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << 15);
	// set low
	PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << 15);
	// set high
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << 15);
#else
	//Enable GPIO for NIC LED
	HAL_MISC_ENABLE_LED012_PINS();
#endif

	// disable all interrupt status sources
	HAL_NIC_DISABLE_ALL_INTERRUPT_STATUS_SOURCES();

	// clear previous interrupt sources
	HAL_NIC_CLEAR_ALL_INTERRUPT_STATUS_SOURCES();

	// disable all DMA-related interrupt sources
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_TXTC_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXRC_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_TXQE_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_NIC_RXQF_BIT_INDEX);

	// clear previous interrupt sources
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_NIC_TXTC_BIT_INDEX);
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_NIC_RXRC_BIT_INDEX);
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_NIC_TXQE_BIT_INDEX);
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_NIC_RXQF_BIT_INDEX);

	HAL_NIC_TX_DMA_STOP();
	HAL_NIC_RX_DMA_STOP();

	if (star_nic_buffer_alloc() != 0) {
		return -1;
	}
	star_nic_mac_config(dev);
	star_nic_fc_config(dev);

	if (star_nic_phy_config(dev) != 0) {
		star_nic_buffer_free();
		return -1;
	}

	star_nic_vlan_config(dev);
	star_nic_arl_config(dev);
	star_nic_set_mac_addr(dev, default_mac_addr);
	star_nic_mib_reset();

	*(u32 volatile *)(SYSVA_MISC_BASE_ADDR+0x0c) = 0x00000125;    //0x00000105 pb0_nic

	HAL_NIC_WRITE_TXSD(txring.phy_addr);
	HAL_NIC_WRITE_TX_BASE(txring.phy_addr);
	HAL_NIC_WRITE_RXSD(rxring.phy_addr);
	HAL_NIC_WRITE_RX_BASE(rxring.phy_addr);

	star_nic_dma_config(dev);

	return 0;
}

static int __init star_nic_probe(int port_type)
{
	struct net_device *netdev;
	struct star_nic_private *priv;
	int err;

	netdev = alloc_etherdev(sizeof(struct star_nic_private));
	if (!netdev) {
		err = -ENOMEM;
		goto err_alloc_etherdev;
	}

	priv = netdev_priv(netdev);
	memset(priv, 0, sizeof(struct star_nic_private));
	spin_lock_init(&priv->lock);

	netdev->base_addr		= SYSVA_NIC_BASE_ADDR;
	netdev->stop			= star_nic_close;
	netdev->hard_start_xmit		= star_nic_send_packet;
	netdev->tx_timeout		= star_nic_timeout;
	netdev->get_stats		= star_nic_get_stats;

#if defined(STAR_NIC_TX_HW_CHECKSUM) && defined(MAX_SKB_FRAGS) && defined(STAR_NIC_SG)
	netdev->features                = NETIF_F_IP_CSUM | NETIF_F_SG;
#elif defined(STAR_NIC_TX_HW_CHECKSUM)
	netdev->features                = NETIF_F_IP_CSUM;
#endif
#ifdef CONFIG_STAR_NIC_NAPI
	netdev->poll			= star_nic_poll;
	netdev->weight			= 64;
#endif
	netdev->open			= star_nic_lan_open;
	netdev->set_mac_address		= star_nic_set_lan_mac_addr;
	priv->dev_index			= LAN_PORT;

#ifdef CONFIG_STAR_JUMBO
	netdev->change_mtu = &str8100_change_mtu;
#endif

	err = register_netdev(netdev);
	if (err) {
		goto err_register_netdev;
	}

	SET_MODULE_OWNER(netdev);

	STAR_NIC_LAN_DEV = netdev;

	if ((err = star_nic_init(netdev))) {
		goto err_nic_init;
	}

	return 0;

err_register_netdev:
	free_netdev(netdev);
	return err;

err_nic_init:
	unregister_netdev(netdev);
	return err;

err_alloc_etherdev:
	return err;
}

static int __init star_nic_lan_init(void)
{
	return star_nic_probe(LAN_PORT);
}

static int __init star_nic_proc_init(void)
{
	star_nic_proc_entry = create_proc_entry("str8100/nic", S_IFREG | S_IRUGO, NULL);
	if (star_nic_proc_entry) {
		star_nic_proc_entry->read_proc = star_nic_read_proc;
		star_nic_proc_entry->write_proc = star_nic_write_proc;
	}

	return 0;
}

static int __init star_nic_init_module(void)
{
	int err = 0;

	printk(KERN_INFO "%s", star_nic_driver_version);
	spin_lock_init(&star_nic_send_lock);
	err = star_nic_lan_init();
	if (err != 0) {
		return err;
	}
	star_nic_proc_init();

	printk("\n");
#ifdef CONFIG_INTERNEL_PHY_PATCH
	printk("%s: internal phy patch included.\n",__FUNCTION__);
	//str813x_internal_phy_proc_init();
#elif defined(CONFIG_STAR_NIC_PHY_INTERNAL_PHY)
	printk("%s: internal phy used but no patch included.\n",__FUNCTION__);
#endif
#if defined(STAR_NIC_TX_HW_CHECKSUM) && defined(MAX_SKB_FRAGS) && defined(STAR_NIC_SG)
	printk("%s: scatter/gather enabled.\n",__FUNCTION__);
#else
	printk("%s: scatter/gather disabled.\n",__FUNCTION__);
#endif
	printk("\n");

#ifdef STAR_NIC_TIMER
	init_timer(&star_nic_timer);
	star_nic_timer.function = &star_nic_timer_func;
	star_nic_timer.data = (unsigned long)NULL;
#endif

	return 0;
}

module_init(star_nic_init_module);

//========================================================
#ifdef CONFIG_INTERNEL_PHY_PATCH

static void (*phy_statemachine)(int, int, int);

#define ETH3220_PHY_MON_PERIOD INTERNAL_PHY_PATCH_CHECK_PERIOD

/*===================================================================================*/
/*  phy monitor state  */
#define NUM_PHY 1
#define PHY_STATE_INIT					0
#define LINK_DOWN_POSITIVE				1
#define WAIT_LINK_UP_POSITIVE				2
#define LINK_UP_POSITIVE				3
#define WAIT_BYPASS_LINK_UP_POSITIVE			4
#define BYPASS_AND_LINK_UP_POSITIVE			5
#define LINK_UP_8101_POSITIVE				6
#define WAIT_8101_LINK_UP_POSITIVE			7

#define PHY_STATE_LAST					(WAIT_8101_LINK_UP_POSITIVE+1)
/*===================================================================================*/
/*  time setting  */
#define WAIT_BYPASS_LINK_UP_POSITIVE_TIMEOUT		5000	/*  5000 ms  */
#define WAIT_BYPASS_LINK_UP_NEGATIVE_TIMEOUT		5000	/*  5000 ms  */
#define LINK_DOWN_ABILITY_DETECT_TIMEOUT		5000	/*  5000 ms  */
#define DETECT_8101_PERIOD				7000	/*  7000 ms  */
#define WAIT_8101_LINK_UP_TIMEOUT			3000	/*  3000 ms  */

#define MAX_PHY_PORT					1
#define DEFAULT_AGC_TRAIN				16
#define MAX_AGC_TRAIN					16	//train 16 times
static int agc_train_num = DEFAULT_AGC_TRAIN;
u32 port_displaybuf[NUM_PHY][MAX_AGC_TRAIN + 1] = {{0}};

static int cuv[3][3] = {
	{1, 1, 4},
	{1, 1, 0},
	{1, 1, -4}};
static u32 link_status_old = 0;
//static int agc_th[2] = {0x18, 0x28}; /* To be deleted */
//static u32 phy_mon_timer; /* To be deleted */
//static u32 current_agc = 0;   //0:Not patch, 1:patch
/*===================================================================================*/

typedef struct eth3220_phy_s {
	u16 state;
	u16 linkdown_cnt;
	u32 state_time;
	u32 timer;
} eth3220_phy_t;

#define DEBUG_PHY_STATE_TRANSITION			1
#if DEBUG_PHY_STATE_TRANSITION
/*  show state transition of debug phy port.
 *  -1 for all ports
 *  -2 for disable all ports
 *  0 - 4 for each port  */
static int debug_phy_port = -2;
static char *phystate_name[] = {
	"init",			/*  PHY_STATE_INIT  */
	"ldp",			/*  LINK_DOWN_POSITIVE  */
	"wait_lup",		/*  WAIT_LINK_UP_POSITIVE  */
	"lup",			/*  LINK_UP_POSITIVE  */
	"wait_bp_lup",		/*  WAIT_BYPASS_LINK_UP_POSITIVE  */
	"bp_lup",		/*  BYPASS_AND_LINK_UP_POSITIVE  */
	"8101_lup",		/*  LINK_UP_8101_POSITIVE  */
	"wait_8101_lup",	/*  WAIT_8101_LINK_UP_POSITIVE  */
	"err",
};
#endif  /*  DEBUG_PHY_STATE_TRANSITION  */
static eth3220_phy_t phy[5] = { {PHY_STATE_INIT, 0, 0, 0},
				{PHY_STATE_INIT, 0, 0, 0},
				{PHY_STATE_INIT, 0, 0, 0},
				{PHY_STATE_INIT, 0, 0, 0},
				{PHY_STATE_INIT, 0, 0, 0}};

static u16 long_cable_global_reg[32]={
0x0000,0x19a0,0x1d00,0x0e80,0x0f60,0x07c0,0x07e0,0x03e0,
0x0000,0x0000,0x0000,0x2000,0x8250,0x1700,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x204b,0x01c2,0x0000,
0x0000,0x0000,0x0fff,0x4100,0x9319,0x0021,0x0034,0x270a|FE_PHY_LED_MODE
};

static u16 long_cable_local_reg[32]={
0x3100,0x786d,0x01c1,0xca51,0x05e1,0x45e1,0x0003,0x001c,
0x2000,0x9828,0xf3c4,0x400c,0xf8ff,0x6940,0xb906,0x503c,
0x8000,0x297a,0x1010,0x5010,0x6ae1,0x7c73,0x783c,0xfbdf,
0x2080,0x3244,0x1301,0x1a80,0x8e8f,0x8000,0x9c29,0xa70a|FE_PHY_LED_MODE
};

static void internal_phy_update(unsigned long data)
{
	internal_phy_patch_check(0);
	mod_timer(&internal_phy_timer, jiffies + INTERNAL_PHY_PATCH_CHECK_PERIOD / 10);
}

static struct timer_list internal_phy_timer =
	TIMER_INITIALIZER(internal_phy_update, 0, 0);

/*=============================================================*
 *  eth3220ac_rt8101_phy_setting
 *=============================================================*/
static  void eth3220ac_rt8101_phy_setting(int port)
{
	star_nic_write_phy(port, 12, 0x18ff);
	star_nic_write_phy(port, 18, 0x6400);
}

static void eth3220ac_release_bpf(int port)
{
	star_nic_write_phy(port, 18, 0x6210);
}

static  void eth3220ac_def_bpf(int port)
{
	star_nic_write_phy(port, 18, 0x6bff);
}

static  void eth3220ac_def_linkdown_setting(int port)
{
	star_nic_write_phy(port, 13, 0xe901);
	star_nic_write_phy(port, 14, 0xa3c6);
}

static  void eth3220ac_def_linkup_setting(int port)
{
	star_nic_write_phy(port, 13, 0x6901);
	star_nic_write_phy(port, 14, 0xa286);
}

/*=============================================================*
 *  eth3220ac_link_agc:
 *=============================================================*/
static int eth3220ac_link_agc(int port, int speed)
{
	u16 reg;
	u32 agc_data = 0;
	u32 short_cable;
	int i, jj;

	/* if speed = 100MHz, then continue */
	if (speed == 0)
		return 0;

	short_cable = 0;
	jj = 0;
	for (i=0; i < agc_train_num; i++) {
		star_nic_read_phy(port, 15, &reg);
		reg &= 0x7f;
		if (reg <= 0x12) {
			short_cable = 1;
			jj++;
			agc_data += (u32)reg;
		}
	}
	if (short_cable) {
		agc_data = (agc_data / jj) + 4;
	} else {
		agc_data = (cuv[2][0] * agc_data) / cuv[2][1] / agc_train_num - 4;
	}

	/*  Fix AGC  */
	agc_data = 0xd0 | (agc_data << 9);
	star_nic_write_phy(port, 15, agc_data);
	udelay(1000);
	star_nic_read_phy(port, 15, &reg);
	reg &= ~(0x1 << 7);
	star_nic_write_phy(port, 15, reg);

	return 0;
}

/*=============================================================*
 *  eth3220ac_unlink_agc:
 *=============================================================*/
static void eth3220ac_unlink_agc(int port)
{
	// start AGC adaptive
	star_nic_write_phy(port, 15, 0xa050);
}

/*=============================================================*
 *  eth3220ac_rt8100_check
 *=============================================================*/
static int eth3220ac_rt8100_check(int port)
{
	u16 reg, reg2;

	/* Read reg27 (error register) */
	star_nic_read_phy(port, 27, &reg);
	/* if error exists, set Bypass Filter enable */
	if ((reg & 0xfffc)) {
                star_nic_read_phy(port, 15, &reg);	
                star_nic_read_phy(port, 27, &reg2);	
		if (( reg2 & 0xfffc) && (((reg >> 9) & 0xff) < 0x1c)) {
			printk("8100 pos err\n");
			/* Bypass agcgain disable */
			star_nic_write_phy(port, 15, (reg & (~(0x1 << 7))));
			
			/* repeat counts when reaching threshold error */
			star_nic_write_phy(port, 13, 0x4940);
			
			/* Speed up AN speed && compensate threshold phase error */
			star_nic_write_phy(port, 14, 0xa306);
			
			/* Bypass Filter enable */
                        star_nic_read_phy(port, 18, &reg2);	

			star_nic_write_phy(port, 18, (reg | 0x400));
			
			/* restart AN */
			star_nic_write_phy(port, 0, 0x3300);
			return 1;
		}
	}
	return 0;
}


/*=============================================================*
 *  eth3220ac_rt8100_linkdown
 *=============================================================*/
static void eth3220ac_rt8100_linkdown(int port)
{
	u16 reg;
	
	/* Bypass Filter disable */
	star_nic_read_phy(port, 18, &reg);	
	star_nic_write_phy(port, 18, (reg & (~(0x1 << 10))));
	eth3220ac_def_linkdown_setting(port);
}

static void eth3220ac_normal_phy_setting(int port)
{
	star_nic_write_phy(port, 12, 0xd8ff);
	eth3220ac_def_bpf(port);
}

/*=============================================================*
 *  wp3220ac_phystate
 *=============================================================*/
static void wp3220ac_phystate(int port, int link, int speed)
{
	int next_state;
	u16 reg, reg2;	

	phy[port].timer += ETH3220_PHY_MON_PERIOD;
	
	if (link) {
		/*  Link up state  */
		switch(phy[port].state) {
		case LINK_UP_POSITIVE:
			next_state = eth3220ac_rt8100_check(port) ?
				WAIT_BYPASS_LINK_UP_POSITIVE :
				LINK_UP_POSITIVE;
			break;
			
		case PHY_STATE_INIT:
		case WAIT_LINK_UP_POSITIVE:
		case LINK_DOWN_POSITIVE:
			next_state = LINK_UP_POSITIVE;
			eth3220ac_def_linkup_setting(port);
			eth3220ac_link_agc(port, speed);
			eth3220ac_release_bpf(port);
			break;
			
		case WAIT_BYPASS_LINK_UP_POSITIVE:
		case BYPASS_AND_LINK_UP_POSITIVE:
			next_state = BYPASS_AND_LINK_UP_POSITIVE;
			break;
			
		case WAIT_8101_LINK_UP_POSITIVE:
			next_state = LINK_UP_8101_POSITIVE;
			eth3220ac_link_agc(port, speed);
			star_nic_write_phy(port, 12, 0x98ff);
			break;
			
		case LINK_UP_8101_POSITIVE:
			next_state = LINK_UP_8101_POSITIVE;
			break;
			
		default:
			next_state = LINK_UP_POSITIVE;
			eth3220ac_def_linkup_setting(port);
			eth3220ac_link_agc(port, speed);
		}
	} else {
		/*  Link down state  */
		switch(phy[port].state) {
		case LINK_DOWN_POSITIVE:
                        star_nic_read_phy(port, 5, &reg);	
                        star_nic_read_phy(port, 28, &reg2);	
			/* AN Link Partner Ability Register or NLP */
			if (reg || (reg2 & 0x100))
				next_state = WAIT_LINK_UP_POSITIVE;
			else
				next_state = LINK_DOWN_POSITIVE;
			break;
			
		case WAIT_LINK_UP_POSITIVE:
			if (phy[port].state_time > LINK_DOWN_ABILITY_DETECT_TIMEOUT)
				next_state = LINK_DOWN_POSITIVE;
			else
				next_state = WAIT_LINK_UP_POSITIVE;
			break;
			
		case WAIT_BYPASS_LINK_UP_POSITIVE:
			/* set timeout = 5 sec */
			if (phy[port].state_time > WAIT_BYPASS_LINK_UP_POSITIVE_TIMEOUT) {
				next_state = LINK_DOWN_POSITIVE;
				/* Bypass Filter disable */
				eth3220ac_rt8100_linkdown(port);
				eth3220ac_def_bpf(port);
			} else {
				next_state = WAIT_BYPASS_LINK_UP_POSITIVE;
			}
			break;
			
		case BYPASS_AND_LINK_UP_POSITIVE:
			next_state = LINK_DOWN_POSITIVE;
			eth3220ac_rt8100_linkdown(port);
			eth3220ac_def_bpf(port);
			break;
			
		case WAIT_8101_LINK_UP_POSITIVE:
			if (phy[port].state_time > WAIT_8101_LINK_UP_TIMEOUT) {
				next_state = LINK_DOWN_POSITIVE;
				eth3220ac_normal_phy_setting(port);
				eth3220ac_def_linkdown_setting(port);
			} else {
				next_state = WAIT_8101_LINK_UP_POSITIVE;
			}
			break;
			
		case LINK_UP_POSITIVE:
			eth3220ac_unlink_agc(port);
			eth3220ac_def_linkdown_setting(port);
			eth3220ac_def_bpf(port);
			if (phy[port].timer > DETECT_8101_PERIOD) {
				next_state = LINK_DOWN_POSITIVE;
				phy[port].timer = 0;
				phy[port].linkdown_cnt = 1;
			} else {
				if (++phy[port].linkdown_cnt > 2) {
					next_state = WAIT_8101_LINK_UP_POSITIVE;
					eth3220ac_rt8101_phy_setting(port);
				} else {
					next_state = LINK_DOWN_POSITIVE;
				}
			}
			break;
			
		case LINK_UP_8101_POSITIVE:
			eth3220ac_normal_phy_setting(port);
			/*  fall down to phy normal state  */
		case PHY_STATE_INIT:
			eth3220ac_def_linkdown_setting(port);
			eth3220ac_unlink_agc(port);
		default:
			next_state = LINK_DOWN_POSITIVE;
		}
	}
	
	if (phy[port].state != next_state) {
		phy[port].state_time = 0;
#if DEBUG_PHY_STATE_TRANSITION
		if (debug_phy_port == -1 || port == debug_phy_port)
		{
			if ((phy[port].state < PHY_STATE_LAST) && (next_state < PHY_STATE_LAST))
			{
				printk("p%d: %s->%s, %d, %d\n", port, phystate_name[phy[port].state],
					phystate_name[next_state], phy[port].timer, phy[port].linkdown_cnt);
			}
			else
			{
				printk("p%d: %d->%d\n", port, phy[port].state, next_state);
			}
		}
#endif   /*  DEBUG_PHY_STATE_TRANSITION  */
	} else {
		phy[port].state_time += ETH3220_PHY_MON_PERIOD;
	}
	phy[port].state = next_state;
}

/*=============================================================*
 *  eth3220_phyinit:
 *=============================================================*/
static void eth3220ac_10m_agc(void)
{
	/* Force 10M AGC = 2c globally */
	star_nic_write_phy(0, 31, 0x2f1a);
	star_nic_write_phy(0, 12, 0x112c);
	star_nic_write_phy(0, 13, 0x2e21);
	star_nic_write_phy(0, 31, 0xaf1a);
}

static void eth3220ac_dfe_init(void)
{
	int i;

	star_nic_write_phy(0, 31, 0x2f1a);
	for (i=0; i <= 7; i++)
		star_nic_write_phy(0, i, 0);
	star_nic_write_phy(0, 11, 0x0b50);
	star_nic_write_phy(0, 31, 0xaf1a);
}

static void eth3220ac_phy_cdr_training_init(void)
{
	int volatile i;

	/* Force all port in 10M FD mode */
	for (i=0; i < NUM_PHY; i++)
		star_nic_write_phy(i, 0, 0x100);
	
	/* Global setting */
	star_nic_write_phy(0, 31, 0x2f1a);
	star_nic_write_phy(0, 29, 0x5021);
        udelay(2000); //2ms, wait > 1 ms
	star_nic_write_phy(0, 29, 0x4021);
        udelay(2000); //2ms, wait > 1 ms
	star_nic_write_phy(0, 31, 0xaf1a);

	/* Enable phy AN */
	for (i=0; i < NUM_PHY; i++)
		star_nic_write_phy(i, 0, 0x3100);	
}

static void eth3220_phyinit(void)
{
	eth3220ac_10m_agc();
	eth3220ac_dfe_init();
	eth3220ac_phy_cdr_training_init();
}

static void eth3220_phycfg(int phyaddr)
{
	eth3220ac_def_linkdown_setting(phyaddr);
	eth3220ac_normal_phy_setting(phyaddr);
	star_nic_write_phy(phyaddr, 9, 0x7f);
}

static void internal_phy_patch_check(int init)
{
	u32 short_cable_agc_detect_count;
	u32 link_status = 0, link_speed;
	u32 ii, jj;
	u16 phy_data;
	u16 phy_data2;

	star_nic_read_phy(STAR_NIC_PHY_ADDR, 1, &phy_data);
	udelay(100);
	star_nic_read_phy(STAR_NIC_PHY_ADDR, 1, &phy_data2);
	if (((phy_data & 0x0004) != 0x0004) && ((phy_data2 & 0x0004) != 0x0004)) { // link down
		short_cable_agc_detect_count = 0;
		for (jj = 0; jj < INTERNAL_PHY_PATCH_CHECKCNT; jj++) {
			star_nic_read_phy(STAR_NIC_PHY_ADDR, 15, &phy_data);
			udelay(1000);
			if (((phy_data) & 0x7F) <= 0x12) { // short cable
				short_cable_agc_detect_count++;
				break;
			}
		}
		if (short_cable_agc_detect_count) { // short cable
			phy_statemachine = wp3220ac_phystate;
			eth3220_phyinit();
			star_nic_read_phy(STAR_NIC_PHY_ADDR, 1, &phy_data);
			if (phy_data & 0x0040) { // link up
				link_status = 1;
			}
			if ((NIC_MAC_CONTROL_REG & 0xC) == 0x4) { // 100Mbps
				link_speed = 1;
			} else {
				link_speed = 0;
			}
			link_status_old = link_status;
			for (ii = 0; ii < MAX_PHY_PORT; link_status >>= 1, ii++) {
				eth3220_phycfg(ii);
#if 0
				if (phy_statemachine != NULL)
					(*phy_statemachine)(ii, link_status & 1, link_speed & 1);
#endif
			}
		} else { // long cable
			// set to global domain
			star_nic_write_phy(NIC_PHY_ADDRESS, 31, 0x2f1a);
			for (ii = 0; ii < 32; ii++) {
				star_nic_write_phy(NIC_PHY_ADDRESS, ii, long_cable_global_reg[ii]);
			}
			// set to local domain
			star_nic_write_phy(NIC_PHY_ADDRESS, 31, 0xaf1a);
			for (ii = 0; ii < 32; ii++) {
				star_nic_write_phy(NIC_PHY_ADDRESS, ii, long_cable_local_reg[ii]);
			}
		}
	}
}

#endif
//========================================================
