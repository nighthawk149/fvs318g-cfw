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

#ifndef __STR9100_INFO_H_
#define __STR9100_INFO_H_
#include <asm/ioctl.h>                                                   
#include <linux/types.h>

                                                                           
/* magic number for STR9100_INFO IOCTL operations */                             
#define STR9100_INFO_MAGIC 'S'                                                   
                                                                           
#define STR9100_INFO_IOCGETD  _IOR(STR9100_INFO_MAGIC, 1 , char *) //Get struct
#define STR9100_INFO_IOCSETD  _IOW(STR9100_INFO_MAGIC, 2 , char *) //Reversed Function   
#define STR9100_INFO_IOCPUCLK _IOR(STR9100_INFO_MAGIC, 10 , char *) //CPUCLK
#define STR9100_INFO_IOV18    _IOR(STR9100_INFO_MAGIC, 11 , char *) //Regulator 1.8V output
#define STR9100_INFO_IOPCICLK _IOR(STR9100_INFO_MAGIC, 12 , char *) //PCI Clock 
#define STR9100_INFO_IODRAMSZ _IOR(STR9100_INFO_MAGIC, 13 , char *) //DRAM Size 



//#define CONFIG_SWITCH_IOCTL
#ifdef CONFIG_SWITCH_IOCTL


#define STR9100_GSW_GET_VVID  _IOWR(STR9100_INFO_MAGIC, 31 , char *) // get VVID
#define STR9100_GSW_SET_VVID  _IOW(STR9100_INFO_MAGIC, 32 , char *) // set VVID
#define STR9100_GSW_GET_VID  _IOWR(STR9100_INFO_MAGIC, 33 , char *) // get VID from GID
#define STR9100_GSW_SET_VID  _IOW(STR9100_INFO_MAGIC, 34 , char *) // set VID


#define STR9100_GSW_SET_VID_MAC  _IOW(STR9100_INFO_MAGIC, 35 , char *) // set VID
#define STR9100_GSW_GET_VID_MAC  _IOW(STR9100_INFO_MAGIC, 36 , char *) // set VID

#define STR9100_GSW_DEL_VID_MAC  _IOW(STR9100_INFO_MAGIC, 37 , char *) // set VID
#define STR9100_GSW_LOOKUP_ARL  _IOW(STR9100_INFO_MAGIC, 38 , char *) // set VID
#define STR9100_GSW_ADD_VID_MAC  _IOW(STR9100_INFO_MAGIC, 39 , char *) // set VID
#define STR9100_GSW_PURE_DEL_VID_MAC  _IOW(STR9100_INFO_MAGIC, 40 , char *) // set VID


#define STR9100_GSW_GET_SHNAT_WANGID _IOR(STR9100_INFO_MAGIC, 21 , char *) //DRAM Size 
#define STR9100_GSW_SET_SHNAT_WANGID _IOR(STR9100_INFO_MAGIC, 22 , char *) //DRAM Size 


#define VVID_SIZE (32*1024)


typedef struct VVID_
{
        __u16 pri_;
        __u16 vid_;
}VVID;

typedef struct VVIDContent_
{
        //VVID vvid_;
        __u16 pri_;
        __u16 vid_;
        __u16 vvid_num_;
}VVIDContent;

typedef struct VGIDPair_
{
        __u16 vid_;
        __u16 gid_;

}VGIDPair;

typedef struct VGIDMAC_
{
        __u16 vid_;
        __u16 gid_;
        //int net_device_index_;
        __u8 mac_[6];

}VGIDMAC;

extern int vvid_size;
extern __u16 *vvid;
extern VVID *vvid_ary;

#define NETDEV_NUM 10
extern unsigned char gid_map_ary[NETDEV_NUM]; // map net device and gid, ex: eth0 -> gid 7




#endif // CONFIG_SWITCH_IOCTL

// Max Data Struct Size
#define STR9100_INFO_SIZE 1024

#endif
