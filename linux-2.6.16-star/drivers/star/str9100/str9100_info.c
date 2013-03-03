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


#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,32)
#define LINUX24 1
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26 1
#endif

#include <linux/stddef.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <asm/arch/hardware.h>
#include <linux/kdev_t.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/system.h>
#include "../../net/str9100/star_gsw.h"

#ifdef LINUX24
#include <asm/arch/str9100_info.h>
#include <asm/arch/str9100/star_tool.h>
#include <asm/arch/str9100/star_gsw.h>
#endif

#ifdef LINUX26
#include <linux/str9100/str9100_tool.h>
#include <linux/str9100/str9100_info.h>
#endif








static const char *cpu_str[]={
"175",
"200",
"225",
"250"};

static const char *v18_str[] = {
"1.537",
"1.594",
"1.655",
"1.721",
"1.793",
"1.871",
"1.956",
"2.049"};

static const char *pciclk_str[] = {
"33",
"66",
"-"};

static const char *dram_str[] = {
"16",
"32",
"64",
"-"};

#ifdef LINUX24
struct proc_dir_entry *str9100_info_proc;
#endif


typedef struct STR9100_INFO_{
	u32 cpu;
	u32 v18regular;
	u32 pciclk;
	u32 dram;
}STR9100_INFO;

static STR9100_INFO str9100_info;

#ifdef LINUX26
static struct proc_dir_entry *str9100_info_proc_entry;
#endif


#ifdef LINUX24
int get_system_info(void){
	u32 volatile temp;
	temp = (((*(u32 volatile *)(IO_ADDRESS(0x77000014))) >> 6) & 0x3);
	str9100_info.cpu = temp;

	temp = (*(u32 volatile *)(IO_ADDRESS(0x77000018)));
	temp = ((temp >> 11)&0x7);
	str9100_info.v18regular = temp;

#if defined(CONFIG_STAR9100_PCI66M)
	str9100_info.pciclk = 1;
#elif defined(CONFIG_STAR9100_PCI33M)
	str9100_info.pciclk = 0;
#elif 
	str9100_info.pciclk = 2;
#endif

#if defined(CONFIG_STR9100_DRAM_64M)
	str9100_info.dram = 2;
#elif defined(CONFIG_STR9100_DRAM_32M)
	str9100_info.dram = 1;
#elif defined(CONFIG_STR9100_DRAM_16M)
	str9100_info.dram = 0;
#else
	str9100_info.dram = 3;
#endif


	return 0;
}
#endif

#ifdef LINUX26
static int get_system_info(void)
{
	u32 temp;
	temp = (PWRMGT_RESET_LATCH_CONFIGURATION_REG >> 6) & 0x3;
	str9100_info.cpu = temp;

	temp = (PWRMGT_REGULATOR_CONTROL_REG >> 11) & 0x7;
	temp = ((temp >> 11)&0x7);
	str9100_info.v18regular = temp;

#if defined(CONFIG_STR9100_PCI66M)
	str9100_info.pciclk = 1;
#elif defined(CONFIG_STR9100_PCI33M)
	str9100_info.pciclk = 0;
#else
	str9100_info.pciclk = 2;
#endif

#if defined(CONFIG_STR9100_DRAM_64M)
	str9100_info.dram = 2;
#elif defined(CONFIG_STR9100_DRAM_32M)
	str9100_info.dram = 1;
#elif defined(CONFIG_STR9100_DRAM_16M)
	str9100_info.dram = 0;
#else
	str9100_info.dram = 3;
#endif

	return 0;
}
#endif


#ifdef LINUX24
static int str9100_info_read_proc(char *page, char **start,  off_t off, int count, int *eof, void *data){
	int num = 0;

        volatile unsigned long remap = (unsigned long)ioremap(FLASH_BASE_ADDR, FLASH_SIZE);
        volatile u16 *addr = (u16 *)(remap + 0);

	// for human readable
	num += sprintf(page+num, "--- CPU \n");
	num += sprintf(page+num, "CPU Clock: Str9100 %sMhz\n",cpu_str[str9100_info.cpu]);
	num += sprintf(page+num, "1.8V Regulator Regulated vdd Output : %sv \n",v18_str[str9100_info.v18regular]);
	num += sprintf(page+num, "--- Device \n");
	num += sprintf(page+num, "PCI Clock: %sMhz\n",pciclk_str[str9100_info.pciclk]);
	num += sprintf(page+num, "--- Memory \n");
	num += sprintf(page+num, "DRAM Size: %sMBytes\n",dram_str[str9100_info.dram]);
#ifdef CONFIG_CPU_ISCRATCHPAD_ENABLE
	num += sprintf(page+num, "enable I-Scratchpad\n");
#else
	num += sprintf(page+num, "disable I-Scratchpad\n");
#endif
	num += sprintf(page+num, "flash type: %s\n", get_flash_type(addr) );


        iounmap(remap);
		
	return num;
}
#endif

#ifdef LINUX26
static int str9100_info_read_proc(char *page, char **start,  off_t off, int count, int *eof, void *data)
{
	int num = 0;

	// for human readable
	num += sprintf(page+num, "--- CPU \n");
	num += sprintf(page+num, "CPU Clock: STR9100 %sMhz\n", cpu_str[str9100_info.cpu]);
	num += sprintf(page+num, "1.8V Regulator Regulated vdd Output : %sv \n", v18_str[str9100_info.v18regular]);
	num += sprintf(page+num, "--- Device \n");
	num += sprintf(page+num, "PCI Clock: %sMhz\n", pciclk_str[str9100_info.pciclk]);
	num += sprintf(page+num, "--- Memory \n");
	num += sprintf(page+num, "DRAM Size: %sMBytes\n", dram_str[str9100_info.dram]);
#ifdef CONFIG_CPU_ISPAD_ENABLE
	num += sprintf(page+num, "I-Scratchpad enable\n");
#else
	num += sprintf(page+num, "I-Scratchpad disable\n");
#endif
	num += sprintf(page+num, "flash type: %s\n", get_flash_type());

	return num;
}
#endif

int str9100_info_write_proc(struct file *file, const char *buffer, unsigned long count, void *data){

	return 0;
}

// copy form drivers/net/str9100/star_gsw_phy.h
#ifdef LINUX26
#define GSW_VLAN_VID_0_1 GSW_VLAN_VID_0_1_REG
#define GSW_VLAN_VID_2_3 GSW_VLAN_VID_2_3_REG
#define GSW_VLAN_VID_4_5 GSW_VLAN_VID_4_5_REG
#define GSW_VLAN_VID_6_7 GSW_VLAN_VID_6_7_REG
#define GSW_HNAT_CONFIG GSW_HNAT_CONFIG_REG
#endif


//#define CONFIG_SWITCH_IOCTL
#ifdef CONFIG_SWITCH_IOCTL

#define GSW_SET_VLAN_0_VID(vid) \
{ \
	((GSW_VLAN_VID_0_1) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_0_1) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_1_VID(vid) \
{ \
	((GSW_VLAN_VID_0_1) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_0_1) |= (((vid) & 0xFFF) << 12)); \
}

#define GSW_SET_VLAN_2_VID(vid) \
{ \
	((GSW_VLAN_VID_2_3) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_2_3) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_3_VID(vid) \
{ \
	((GSW_VLAN_VID_2_3) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_2_3) |= (((vid) & 0xFFF) << 12)); \
}

#define GSW_SET_VLAN_4_VID(vid) \
{ \
	((GSW_VLAN_VID_4_5) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_4_5) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_5_VID(vid) \
{ \
	((GSW_VLAN_VID_4_5) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_4_5) |= (((vid) & 0xFFF) << 12)); \
}

#define GSW_SET_VLAN_6_VID(vid) \
{ \
	((GSW_VLAN_VID_6_7) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_6_7) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_7_VID(vid) \
{ \
	((GSW_VLAN_VID_6_7) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_6_7) |= (((vid) & 0xFFF) << 12)); \
}

void change_vid(u8 gid, u16 vid)
{
		switch (gid) 
		{
		    case 0:
		    {
			GSW_SET_VLAN_0_VID(vid);
			break;
		    }
		    case 1:
		    {
			GSW_SET_VLAN_1_VID(vid); 
			break;
		    }
		    case 2:
		    {
			GSW_SET_VLAN_2_VID(vid);
			break;
		    }
		    case 3:
		    {
			GSW_SET_VLAN_3_VID(vid); 
			break;
		    }
		    case 4:
		    {
			GSW_SET_VLAN_4_VID(vid);
			break;
		    }
		    case 5:
		    {
			GSW_SET_VLAN_5_VID(vid); 
			break;
		    }
		    case 6:
		    {
			GSW_SET_VLAN_6_VID(vid);
			break;
		    }
		    case 7:
		    {
			GSW_SET_VLAN_7_VID(vid); 
			break;
		    }
		}

}

int get_vid(u8 gid)
{
	switch (gid) 
	{
	    case 0:
	    {
		return (GSW_VLAN_VID_0_1 & 0x0fff);
	    }
	    case 1:
	    {
		return ((GSW_VLAN_VID_0_1 >> 12) & 0x0fff);
	    }
	    case 2:
	    {
		return (GSW_VLAN_VID_2_3 & 0x0fff);
	    }
	    case 3:
	    {
		return ((GSW_VLAN_VID_2_3 >> 12) & 0x0fff);
	    }
	    case 4:
	    {
		return (GSW_VLAN_VID_4_5 & 0x0fff);
	    }
	    case 5:
	    {
		return ((GSW_VLAN_VID_4_5 >> 12) & 0x0fff);
	    }
	    case 6:
	    {
		return (GSW_VLAN_VID_6_7 & 0x0fff);
	    }
	    case 7:
	    {
		return ((GSW_VLAN_VID_6_7 >> 12) & 0x0fff);
	    }
	}

	return -1;
}
#endif

static int str9100_info_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg){

#if 1
     int len;
     char temp[STR9100_INFO_SIZE];
#ifdef CONFIG_SWITCH_IOCTL
	int merge_int;
	u16 vvid_num;
	VVIDContent vvid_content;
	VGIDPair vgid_pair;
	VGIDMAC vgid_mac;
	u8 gid;
	u32 shnat_wangid;
	u32 hnat_cfg;

	extern gsw_info_t star_gsw_info;

#endif


     switch (cmd) {

#ifdef CONFIG_SWITCH_IOCTL
        case STR9100_GSW_GET_SHNAT_WANGID:
		copy_from_user(&shnat_wangid, (u32*)arg, sizeof(u32));
                GSW_READ_HNAT_CONFIGURATION(hnat_cfg);
		shnat_wangid = hnat_cfg >> 16;

  		copy_to_user((u32*)arg, &shnat_wangid, sizeof(u32));

		return 0;

        case STR9100_GSW_SET_SHNAT_WANGID:
		copy_from_user(&shnat_wangid, (u32 *)arg, sizeof(u32));


                GSW_READ_HNAT_CONFIGURATION(hnat_cfg);
                hnat_cfg &= (~(0xff << 16));
                //gid_bitmap=simple_strtol(buf_param1, NULL, 16);
                printk("[kernel mode] shnat_wangid : %x\n", shnat_wangid);
                hnat_cfg |= (shnat_wangid << 16);
                GSW_WRITE_HNAT_CONFIGURATION(hnat_cfg);
		return 0;
#if 1
        case STR9100_GSW_LOOKUP_ARL:
		copy_from_user(&vgid_mac, (VGIDMAC *)arg, sizeof(VGIDMAC));
		#if 0
		printk("[kernel mode] STR9100_GSW_LOOKUP_ARL\n");
		printk("[kernel mode] gid : %d\n", vgid_mac.gid_);
		printk("[kernel mode] mac # %x:%x:%x:%x:%x:%x\n", vgid_mac.mac_[0], vgid_mac.mac_[1],vgid_mac.mac_[2],vgid_mac.mac_[3],vgid_mac.mac_[4],vgid_mac.mac_[5]);
		#endif

		//memcpy(vgid_mac.mac_, star_gsw_info.vlan[vgid_mac.gid_].vlan_mac, 6);

	        vgid_mac.vid_ = star_gsw_info.vlan[vgid_mac.gid_].vlan_vid;

		printk("[kernel mode] gid : %d\n", vgid_mac.gid_);
		printk("[kernel mode] vid : %d\n", vgid_mac.vid_);
		//printk("[kernel mode] mac # %x:%x:%x:%x:%x:%x\n", vgid_mac.mac_[0], vgid_mac.mac_[1],vgid_mac.mac_[2],vgid_mac.mac_[3],vgid_mac.mac_[4],vgid_mac.mac_[5]);

		if (star_gsw_search_arl_table(vgid_mac.mac_, vgid_mac.gid_)) {
			return 1; // found
		}
		else {
			return 0; // found
		}
		return 0;

        //  unrelated variable star_gsw_info
        case STR9100_GSW_ADD_VID_MAC:
                // add a mac into arl table
                #if 0
                printk("[kernel mode] STR9100_GSW_ADD_VID_MAC\n");
                #endif
                copy_from_user(&vgid_mac, (VGIDMAC *)arg, sizeof(VGIDMAC));
                add_mac_into_arl(vgid_mac.gid_, vgid_mac.mac_);
                return 0;

        //  unrelated variable star_gsw_info
        //  unrelated hnat
        case STR9100_GSW_PURE_DEL_VID_MAC:
                copy_from_user(&vgid_mac, (VGIDMAC *)arg, sizeof(VGIDMAC));
		//star_gsw_del_arl_table(vgid_mac.mac_, vgid_mac.gid_);
		del_mac_from_arl(vgid_mac.gid_, vgid_mac.mac_);
                return 0;


	// related hnat
        case STR9100_GSW_DEL_VID_MAC:
		#if 0
		printk("[kernel mode] STR9100_GSW_DEL_VID_MAC\n");
		#endif
		copy_from_user(&vgid_mac, (VGIDMAC *)arg, sizeof(VGIDMAC));

		del_my_vlan_mac(vgid_mac.gid_);
		return 0;
#endif
        case STR9100_GSW_SET_VID_MAC:
		copy_from_user(&vgid_mac, (VGIDMAC *)arg, sizeof(VGIDMAC));
		#if 0
		printk("[kernel mode] STR9100_GSW_SET_VID_MAC\n");
		//printk("[kernel mode] net_device_index_ : %d\n", vgid_mac.net_device_index_);
		printk("[kernel mode] gid : %d\n", vgid_mac.gid_);
		printk("[kernel mode] vid : %d\n", vgid_mac.vid_);
		printk("[kernel mode] mac # %x:%x:%x:%x:%x:%x\n", vgid_mac.mac_[0], vgid_mac.mac_[1],vgid_mac.mac_[2],vgid_mac.mac_[3],vgid_mac.mac_[4],vgid_mac.mac_[5]);
		#endif

		change_vid(vgid_mac.gid_, vgid_mac.vid_);
		del_my_vlan_mac(vgid_mac.gid_);
		config_my_vlan_mac(vgid_mac.gid_, vgid_mac.vid_, vgid_mac.mac_);
		star_gsw_hnat_write_vlan_src_mac(vgid_mac.gid_, vgid_mac.mac_);
		//gid_map_ary[vgid_mac.net_device_index_]=vgid_mac.gid_;
		return 0;


        case STR9100_GSW_GET_VID_MAC:
		copy_from_user(&vgid_mac, (VGIDMAC *)arg, sizeof(VGIDMAC));
        	star_gsw_info.vlan[vgid_mac.gid_].vlan_gid;
	        vgid_mac.vid_ = star_gsw_info.vlan[vgid_mac.gid_].vlan_vid;
	        //vgid_mac.net_device_index_ = 
		memcpy(vgid_mac.mac_, star_gsw_info.vlan[vgid_mac.gid_].vlan_mac, 6);

		#if 0
		printk("[kernel mode] STR9100_GSW_GET_VID_MAC\n");
		printk("[kernel mode] gid : %d\n", vgid_mac.gid_);
		printk("[kernel mode] vid : %d\n", vgid_mac.vid_);
		printk("[kernel mode] mac # %x:%x:%x:%x:%x:%x\n", vgid_mac.mac_[0], vgid_mac.mac_[1],vgid_mac.mac_[2],vgid_mac.mac_[3],vgid_mac.mac_[4],vgid_mac.mac_[5]);
		#endif
	  	copy_to_user((VGIDMAC*)arg, &vgid_mac, sizeof(VGIDMAC));


		return 0;


        case STR9100_GSW_SET_VID:
		copy_from_user(&vgid_pair, (VGIDPair*)arg, sizeof(VGIDPair));

		//printk("SET_VID\n");
		//printk("vgid_pair.vid_", vgid_pair.vid_);
		//printk("vgid_pair.gid_", vgid_pair.gid_);
		if (0 <= vgid_pair.gid_ && vgid_pair.gid_ <=7)
		{
			change_vid(vgid_pair.gid_, vgid_pair.vid_);
			return 0;
		}
		else
		{
			printk("not valid gid: %d\n", vgid_pair.gid_);
           		return -ENOTTY;
		}

        case STR9100_GSW_GET_VID:
		copy_from_user(&vgid_pair, (VGIDPair*)arg, sizeof(VGIDPair));
		
		//printk("GET_VID\n");
		//printk("vgid_pair.vid_", vgid_pair.vid_);
		//printk("vgid_pair.gid_", vgid_pair.gid_);
		if (0 <= vgid_pair.gid_ && vgid_pair.gid_ <=7)
		{
			u16 vid=get_vid(vgid_pair.gid_);
			vgid_pair.vid_=vid;
			//printk("vid : %d\n", vid);
	  		copy_to_user((VGIDPair*)arg, &vgid_pair, sizeof (VGIDPair));
			return 0;
		}
		else
		{
			printk("not valid gid: %d\n", vgid_pair.gid_);
           		return -ENOTTY;
		}

#ifdef CONFIG_VVID
        case STR9100_GSW_SET_VVID:
		//copy_from_user(&vvid_data, (VVID *)arg, sizeof(VVID));
		copy_from_user(&vvid_content, (VVIDContent *)arg, sizeof(VVIDContent));
		//printk("vvid.pri_: %d\n", vvid_data.pri_);
		//printk("vvid.vid_: %d\n", vvid_data.vid_);
         	merge_int = (vvid_content.pri_ << 12 | vvid_content.vid_);
		if (vvid[merge_int]==0)
		{ // insert a vvid number (vvid_szie)
			++vvid_size;
			vvid[merge_int]=vvid_content.vvid_num_;
			vvid_ary[vvid_content.vvid_num_].pri_=vvid_content.pri_;
			vvid_ary[vvid_content.vvid_num_].vid_=vvid_content.vid_;
			//printk("in kernel vvid_ary[%d] => (%d, %d)\n", vvid_content.vvid_num_, vvid_ary[vvid_content.vvid_num_].pri_, vvid_ary[vvid_content.vvid_num_].vid_);
		}
		else
		{
			printk("vvid[%d] already exist\n", merge_int);
		}
		return vvid_size;


        case STR9100_GSW_GET_VVID:
		copy_from_user(&vvid_content, (VVIDContent *)arg, sizeof(VVIDContent));
		//printk("in kernel vvid_content.vvid_num_: %d\n", vvid_content.vvid_num_);
		vvid_content.pri_ = vvid_ary[vvid_content.vvid_num_].pri_;
		vvid_content.vid_ = vvid_ary[vvid_content.vvid_num_].vid_;
		printk("in get kernel vvid_ary[%d] => (%d, %d)\n", vvid_content.vvid_num_, vvid_content.pri_, vvid_content.vid_);
	  	copy_to_user((VVIDContent*)arg, &vvid_content, sizeof (VVIDContent));
		return 0;
	   	break;

#endif // CONFIG_VVID
#endif // end CONFIG_SWITCH_IOCTL


        case STR9100_INFO_IOCGETD:    
	   printk("STR9100_INFO_IOCGETD \n");
           copy_to_user((unsigned char *)arg, (unsigned char *)&str9100_info, sizeof(str9100_info));
	   goto ioctlexit;

        case STR9100_INFO_IOCSETD:
	   printk("STR9100_INFO_IOCSETD \n");
           //if (copy_from_user(temp, (unsigned char *)arg, count))     
           //    return -EFAULT;    
	   goto ioctlexit;

	case STR9100_INFO_IOCPUCLK:
	   len = strlen(cpu_str[str9100_info.cpu]);
	   strcpy(temp,cpu_str[str9100_info.cpu]);
	   break;

	case STR9100_INFO_IOV18:
	   len = strlen(v18_str[str9100_info.v18regular]);
	   strcpy(temp,v18_str[str9100_info.v18regular]);
	   break;

	case STR9100_INFO_IOPCICLK:
	   len = strlen(pciclk_str[str9100_info.pciclk]);
	   strcpy(temp,pciclk_str[str9100_info.pciclk]);
	   break;

	case STR9100_INFO_IODRAMSZ:
	   len = strlen(dram_str[str9100_info.dram]);
	   strcpy(temp,dram_str[str9100_info.dram]);
	   break;



        default:
           return -ENOTTY;
  /*         return -EINVAL; another return option */                      
     }


	  copy_to_user((unsigned char *)arg,temp,len+1);



ioctlexit:
#endif
	return 0;
}

static int str9100_info_open(struct inode *inode, struct file *file)
{
        unsigned int minor = MINOR(inode->i_rdev);
        if (minor != STR9100_INFO_MINOR)
                return -ENODEV;

#ifdef MODULE
        MOD_INC_USE_COUNT;
#endif

        return 0;
}


static int str9100_info_release(struct inode *inode, struct file *file)
{

#ifdef MODULE
        MOD_DEC_USE_COUNT;
#endif

        return 0;
}


/*
 * ioctl interface
 */
static struct file_operations str9100_info_fops =
{
        owner:          THIS_MODULE,
        ioctl:          str9100_info_ioctl,
        open:           str9100_info_open,
        release:        str9100_info_release,
};

/* STR9100_MINOR in include/linux/miscdevice.h */
static struct miscdevice str9100_info_miscdev =
{
        STR9100_INFO_MINOR,
        "str9100_info",
        &str9100_info_fops
};


#ifdef LINUX24
static int __init str9100_info_init(void){
	struct proc_dir_entry *procdir=0;


	get_system_info();

	misc_register(&str9100_info_miscdev);

	//proc_mkdir("str9100",0);
	procdir= create_proc_str9100(PROC_STR);
	if (procdir)
	{
		str9100_info_proc = create_proc_entry("info", S_IFREG | S_IRUGO, procdir);
                if (str9100_info_proc) {
                        str9100_info_proc->read_proc = str9100_info_read_proc;
                        str9100_info_proc->write_proc = str9100_info_write_proc;
                }
		printk("Str9100 Information inited \n");
                return 0;
        }
        else
                return -1;
	
	//str9100_info_proc=create_proc_read_entry( "str9100/info", 0, NULL, str9100_info_read_proc,NULL) ;
        //str9100_info_proc->write_proc=star9100_info_write_proc;


}
#endif

#ifdef LINUX26
static int __init str9100_info_init(void)
{
	get_system_info();

	str9100_info_proc_entry = create_proc_entry("str9100/info", S_IFREG | S_IRUGO, NULL);
	if (str9100_info_proc_entry) {
		str9100_info_proc_entry->read_proc = str9100_info_read_proc;
		str9100_info_proc_entry->write_proc = str9100_info_write_proc;
	}

	misc_register(&str9100_info_miscdev);

	return 1;
}
#endif


static void __exit str9100_info_exit(void){

	misc_deregister(&str9100_info_miscdev);

	remove_proc_entry("str9100/info",NULL);

	printk("Str9100 Information exit \n");
}

module_init(str9100_info_init);
module_exit(str9100_info_exit);

