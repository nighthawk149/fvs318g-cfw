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


#ifndef STAR_TOOL_H
#define STAR_TOOL_H

#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,32)
#define LINUX24 1
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26 1
#endif


#include <linux/types.h>




//extern int create_proc_dir;
//extern struct proc_dir_entry *str9100_gsw_procdir;


#define PROC_STR "str9100"



#ifdef LINUX24
struct proc_dir_entry *create_proc_str9100(const char* proc_str);
char *get_flash_env(const char *env_name);
const char *get_flash_type(volatile u16 *saddr);
#endif

#ifdef LINUX26
extern char *get_flash_env(const char *env_name);
extern const char *get_flash_type(void);
#endif


#endif



