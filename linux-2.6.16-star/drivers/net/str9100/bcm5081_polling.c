#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/netdevice.h>
#include "star_gsw_phy.h"
#include "star_gsw.h"
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Polling bcm5081 Module");
MODULE_AUTHOR("Inter You");

extern int star_gsw_read_phy(u8 phy_addr, u8 phy_reg, u16 volatile *read_data);
extern int star_gsw_write_phy(u8 phy_addr, u8 phy_reg, u16 write_data);

extern struct net_device *STAR_GSW_WAN_DEV;
static struct timer_list wan_polling_timer;
static unsigned short int polling_status = 0;

/* polling every 1 sec */
#define SCAN_RATE                   HZ

/*BCM5081 phy addr*/
#define PHY_BCM5081                 0x09
#define PHY_REG_MII_CONTROL         0x00
#define PHY_REG_AUTO_ADVERTISE      0x04
#define PHY_REG_1000BASE_CONTROL    0x09
#define PHY_REG_AUX_SUM             0x19
#define PHY_REG_MII_STATUS          0x01

/*star mac1 addr*/
#define STAR_REG_MAC1               0x7000000c

/*bcm5081 speed*/
#define FULL_1000M                  0x07
#define HALF_1000M                  0x06
#define FULL_100M                   0x05
#define T4_100BASE                  0x04
#define HALF_100M                   0x03
#define FULL_10M                    0x02
#define HALF_10M                    0x01

/* r/w data length */
#define LEN_8BIT        1
#define LEN_16BIT       2
#define LEN_32BIT       4
#define LEN_48BIT       6
#define LEN_64BIT       8

static void get_set_speed(void)
{
    unsigned short int readdata,pspd;
    u32 mac_port_config;
    u32 mac_port_base = 0;

    star_gsw_read_phy(PHY_BCM5081, PHY_REG_AUX_SUM, &readdata);

    pspd = (readdata >>8) & 0x7;

    mac_port_base = GSW_PORT1_CFG_REG;
    mac_port_config = __REG(mac_port_base);

    switch (pspd)
    {
        case FULL_1000M:
            {
                /*clear duplex/speed/auto-negotiation*/
                mac_port_config &= ~(0xf<<7);
                /*set full duplex*/
                mac_port_config |= (1<<10);
                /*set speed 1000Mbps*/
                mac_port_config |= (2<<8);
            }
            break;
        case HALF_1000M:
            {
                /*clear duplex/speed/auto-negotiation*/
                mac_port_config &= ~(0xf<<7);
                /*set speed 1000Mbps*/
                mac_port_config |= (2<<8);
            }
            break;
        case T4_100BASE:
        case FULL_100M:
            {
                /*clear duplex/speed/auto-negotiation*/
                mac_port_config &= ~(0xf<<7);
                /*set full duplex*/
                mac_port_config |= (1<<10);
                /*set speed 100Mbps*/
                mac_port_config |= (1<<8);
            }
            break;
        case HALF_100M:
            {
                /*clear duplex/speed/auto-negotiation*/
                mac_port_config &= ~(0xf<<7);
                /*set speed 100Mbps*/
                mac_port_config |= (1<<8);
            }
            break;
        case FULL_10M:
            {
                /*clear duplex/speed/auto-negotiation*/
                mac_port_config &= ~(0xf<<7);
                /*set full duplex*/
                mac_port_config |= (1<<10);
            }
            break;
        case HALF_10M:
            {
                /*clear duplex/speed/auto-negotiation*/
                mac_port_config &= ~(0xf<<7);
            }
            break;
    }

    __REG(mac_port_base) = mac_port_config;

}

static void wan_polling(unsigned long count)
{
	unsigned int interval = SCAN_RATE;
	unsigned short int data;

    star_gsw_read_phy(PHY_BCM5081, PHY_REG_AUX_SUM, &data);

    /*speed and link status*/
    data &= ((7<<8) | (1<<2) | (1<<15));
    if (unlikely(polling_status != data))
    {
        /*link status change*/
        if ((polling_status^data) & (1<<2))
        {
            if (data & (1<<2))
            {
                netif_carrier_on(STAR_GSW_WAN_DEV);
            }
            else
            {
                polling_status = data;
                netif_carrier_off(STAR_GSW_WAN_DEV);
                goto polling_out;
            }

        }

        /*auto-negotiation complete and link pass*/
        if ((data & (1<<15)) && (data & (1<<2)))
        {
            get_set_speed();
        }

        polling_status = data;
    }

polling_out:
    wan_polling_timer.expires  = jiffies + interval;
	wan_polling_timer.function = wan_polling;
	add_timer(&wan_polling_timer);
}

static int __init init_wan_polling_module(void)
{
    int ret = 0;

	init_timer(&wan_polling_timer);

	/* start polling */
	wan_polling_timer.data     = 0;
    wan_polling_timer.expires  = jiffies + SCAN_RATE;
	wan_polling_timer.function = wan_polling;
    add_timer(&wan_polling_timer);	

    printk(KERN_INFO "bcm5081_polling : Module loaded.\n");

    return ret;
}

module_init(init_wan_polling_module);

