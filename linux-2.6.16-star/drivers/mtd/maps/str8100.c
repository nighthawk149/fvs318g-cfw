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
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#ifdef CONFIG_MTD_PARTITIONS
#include <linux/mtd/partitions.h>
#endif

#define WINDOW_ADDR	0x10000000
#define WINDOW_SIZE	0x00800000
#define BUSWIDTH	1

static struct map_info str8100_map = {
	.name = "STR8100 NOR Flash",
	.size = WINDOW_SIZE,
	.bankwidth = BUSWIDTH,
	.phys = WINDOW_ADDR
};

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition str8100_partitions[] = {
	{
		.name =		"ARMBOOT",
		.offset =	CONFIG_ARMBOOT_OFFSET,
		.size =		CONFIG_KERNEL_OFFSET-CONFIG_ARMBOOT_OFFSET,
	},{
		.name =		"Linux Kernel",
		.offset =	CONFIG_KERNEL_OFFSET,
		.size =		CONFIG_ROOTFS_OFFSET-CONFIG_KERNEL_OFFSET,
	},{
		.name =		"MTD Disk1",
		.offset =	CONFIG_ROOTFS_OFFSET,
		.size =		CONFIG_CFG_OFFSET-CONFIG_ROOTFS_OFFSET,
	},{
		.name =		"MTD Disk2",
		.offset =	CONFIG_CFG_OFFSET,
		.size =		0x800000-CONFIG_CFG_OFFSET,
	}
};
#endif

static struct mtd_info *mymtd;

static int __init init_str8100_mtd(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;

	str8100_map.virt = ioremap(WINDOW_ADDR, WINDOW_SIZE);
	if (!str8100_map.virt) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	simple_map_init(&str8100_map);

	mymtd = do_map_probe("cfi_probe", &str8100_map);
	if (!mymtd) {
		iounmap((void *)str8100_map.virt);
		return -ENXIO;
	}

	mymtd->owner = THIS_MODULE;
	add_mtd_device(mymtd);

#ifdef CONFIG_MTD_PARTITIONS
	parts = str8100_partitions;
	nb_parts = ARRAY_SIZE(str8100_partitions);
	add_mtd_partitions(mymtd, parts, nb_parts);
#endif

	return 0;
}

static void __exit cleanup_str8100_mtd(void)
{
	if (mymtd) {
#ifdef CONFIG_MTD_PARTITIONS
		del_mtd_partitions(mymtd);
#endif
		map_destroy(mymtd);
	}
	if (str8100_map.virt)
		iounmap((void *)str8100_map.virt);
}

module_init(init_str8100_mtd);
module_exit(cleanup_str8100_mtd);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("STAR Semiconductor Corp");
MODULE_DESCRIPTION("MTD map driver for Star Semi STR8100 SOC");

