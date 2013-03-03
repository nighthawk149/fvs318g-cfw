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
#define BUSWIDTH	2

static struct map_info str9100_map = {
	.name = "STR9100 NOR Flash",
	.size = WINDOW_SIZE,
	.bankwidth = BUSWIDTH,
	.phys = WINDOW_ADDR
};

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition str9100_partitions[] = {
	{
		.name =		"ARMBOOT",
		.offset =	CONFIG_ARMBOOT_OFFSET,
		.size =		CONFIG_KERNEL_OFFSET-CONFIG_ARMBOOT_OFFSET,
		.mask_flags =   MTD_WRITEABLE,
	},{
		.name =		"KERNEL",
		.offset =	0x40000,
		.size =		0x240000,
	},{
		.name =		"ROOTFS",
		.offset =	0x280000,
		.size =		0x7C0000-0x280000,
	},{
		.name =		"CONFIG",
		.offset =	0x7C0000,
		.size =		0x800000-0x7C0000,
	},{
		.name =		"FIRMIMAGE",
		.offset =	0x40000,
		.size =		0x780000,
	}
};
#endif

static struct mtd_info *mymtd;

static int __init init_str9100_mtd(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;

	str9100_map.virt = ioremap(WINDOW_ADDR, WINDOW_SIZE);
	if (!str9100_map.virt) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	simple_map_init(&str9100_map);

	mymtd = do_map_probe("cfi_probe", &str9100_map);
	if (!mymtd) {
		iounmap((void *)str9100_map.virt);
		return -ENXIO;
	}

	mymtd->owner = THIS_MODULE;
	add_mtd_device(mymtd);

#ifdef CONFIG_MTD_PARTITIONS
	parts = str9100_partitions;
	nb_parts = ARRAY_SIZE(str9100_partitions);
	add_mtd_partitions(mymtd, parts, nb_parts);
#endif

	return 0;
}

static void __exit cleanup_str9100_mtd(void)
{
	if (mymtd) {
#ifdef CONFIG_MTD_PARTITIONS
		del_mtd_partitions(mymtd);
#endif
		map_destroy(mymtd);
	}
	if (str9100_map.virt)
		iounmap((void *)str9100_map.virt);
}

module_init(init_str9100_mtd);
module_exit(cleanup_str9100_mtd);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("STAR Semiconductor Corp");
MODULE_DESCRIPTION("MTD map driver for Star Semi STR9100 SOC");

