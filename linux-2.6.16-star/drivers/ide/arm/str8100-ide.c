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

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_ide.h>

#define IDE_MAX(a,b)	((a)>(b) ? (a):(b))

#define STR8100_IDE_DATA_OFFSET		0x20
#define STR8100_IDE_ERROR_OFFSET	0x24
#define STR8100_IDE_NSECTOR_OFFSET	0x28
#define STR8100_IDE_SECTOR_OFFSET	0x2C
#define STR8100_IDE_LCYL_OFFSET		0x30
#define STR8100_IDE_HCYL_OFFSET		0x34
#define STR8100_IDE_SELECT_OFFSET	0x38
#define STR8100_IDE_STATUS_OFFSET	0x3C
#define STR8100_IDE_CONTROL_OFFSET	0x40

#define STR8100_IDE_FEATURE_OFFSET	STR8100_IDE_ERROR_OFFSET
#define STR8100_IDE_COMMAND_OFFSET	STR8100_IDE_STATUS_OFFSET
#define STR8100_IDE_ALTSTATUS_OFFSET	STR8100_IDE_CONTROL_OFFSET

struct pio_timing_t
{
	u32 T2;
	u32 T1;
	u32 T2i;
	u32 Ta;
	u32 T0;
} pio_mode_timing[5] = { 
	/* T2  T1  T2i  Ta   T0 */
	{ 290, 70, 600, 35, 600 },
	{ 290, 50, 383, 35, 383 },
	{ 290, 30, 240, 35, 240 },
	{  80, 30,  70, 35, 180 },
	{  70, 25,  50, 35, 120 }
};

struct dma_timing_t
{
	u32 Tm;
	u32 Td;
	u32 Tk;
	u32 T0;
} dma_mode_timing[3] = {
	/* Tm   Td   Tk  T0 */
	{  50, 215, 215, 480 },
	{  30,  80,  50, 150 },
	{  25,  70,  25, 120 }
};

struct udma_timing_t
{
	u32 Tmli;
	u32 Tenv;
	u32 Tcyc;
} udma_mode_timing[6] = {
	{ 20, 20, 115 },
	{ 20, 20,  80 },
	{ 20, 20,  67 },
	{ 20, 20,  56 },
#if 0
	{ 20, 20,  41 },
	{ 20, 20,  31 }
#elif 0
	{ 20, 20,  48 },
	{ 20, 20,  48 }
#else
	{ 20, 20,  41 },
	{ 20, 20,  31 }
#endif
};

extern u32 AHB_clock;

static inline int str8100_ide_is_device_idle(void)
{
	u32 status = _IDE_STATUS_REG;

	if (!(status & IDE_BSY || status & IDE_DRQ))
		return 1;
	else
		return 0;
}

static inline int str8100_ide_is_host_active(void)
{
	return (IDE_STATUS_CONTROL_REG & (0x1 << 4) ? 1 : 0);
}

static inline int str8100_ide_is_device_ready(void)
{
	return ((_IDE_STATUS_REG & IDE_BSY) ? 0 : 1);
}

static int str8100_ide_soft_reset(void)
{
	int i;

	for (i = 1000; i > 0; i--) {
		if (str8100_ide_is_device_ready())
			break;
		udelay(1000);
	}
	if (i == 0) return 0;

	IDE_DEVICE_CONTROL_REG = IDE_SRST;
	udelay(10);
	IDE_DEVICE_CONTROL_REG = 0;
	udelay(2000);

	for (i = 1000; i > 0; i--) {
		if (str8100_ide_is_device_ready())
			break;
		udelay(1000);
	}

	return ((i == 0) ? 0 : 1);
}

void str8100_ide_adjust_pio_timing(ide_drive_t *drive, u8 pio_mode)
{
	u32 pio_T2;
	u32 pio_T1;
	u32 pio_T2i;
	u32 pio_Ta;
	u32 pio_T0;
	u32 pio_timing_config;
	u32 T2, T2i;
	u32 T1, Ta;
	u32 pio_clksel = 1, pio_clock, pio_clock_cycle;

	struct hd_driveid *id = drive->id;

	pio_clock = AHB_clock / (pio_clksel + 1);
	pio_clock_cycle = (u32)1000000000 / pio_clock;
	pio_T2 = pio_mode_timing[pio_mode].T2;
	pio_T1 = pio_mode_timing[pio_mode].T1;
	pio_T2i = pio_mode_timing[pio_mode].T2i;
	pio_Ta = pio_mode_timing[pio_mode].Ta;
	pio_T0 = IDE_MAX(pio_mode_timing[pio_mode].T0, IDE_MAX(id->eide_pio, id->eide_pio_iordy));
	while ((pio_T2 + pio_T2i) < pio_T0) {
		pio_T2 += (pio_clock_cycle >> 1);
		pio_T2i += (pio_clock_cycle >> 1);
	}
	T2 = ((pio_T2 / pio_clock_cycle) + 0) & 0xF;
	T1 = ((pio_T1 / pio_clock_cycle) + 0) & 0xF;
	T2i = ((pio_T2i / pio_clock_cycle) + 0) & 0xF;
	Ta = ((pio_Ta / pio_clock_cycle) + 0) & 0xF;
	pio_timing_config = (T2 << 0) | (T1 << 4) | (T2i << 8) | (Ta << 12);
	if (drive->select.b.unit == 0)
		IDE_DRIVE0_PIO_TIMING_CONFIG_REG = pio_timing_config;
	else
		IDE_DRIVE1_PIO_TIMING_CONFIG_REG = pio_timing_config;
}

void str8100_ide_adjust_mwdma_timing(ide_drive_t *drive, u8 dma_mode)
{
	u32 dma_Tk;
	u32 dma_Td;
	u32 dma_Tm;
	u32 dma_T0;
	u32 dma_timing_config;
	u32 Tk, Td, Tm;
	u32 dma_udma_clock, dma_udma_clock_cycle, dma_udma_clksel = 0;

	struct hd_driveid *id = drive->id;

	dma_udma_clock = AHB_clock / (dma_udma_clksel + 1);
	dma_udma_clock_cycle = (u32)1000000000 / dma_udma_clock;
	dma_Tk = dma_mode_timing[dma_mode].Tk;
	dma_Td = dma_mode_timing[dma_mode].Td;
	dma_Tm = dma_mode_timing[dma_mode].Tm;
	dma_T0 = IDE_MAX(dma_mode_timing[dma_mode].T0, id->eide_dma_min);
	while ((dma_Td + dma_Tk) < dma_T0) {
		dma_Td += (dma_udma_clock_cycle >> 1);
		dma_Tk += (dma_udma_clock_cycle >> 1);
	}
	Tk = ((dma_Tk / dma_udma_clock_cycle) + 0) & 0xF;
	Td = ((dma_Td / dma_udma_clock_cycle) + 0) & 0xF;
	Tm = ((dma_Tm / dma_udma_clock_cycle) + 0) & 0xF;
	dma_timing_config = (Tk << 0) | (Td << 4) | (Tm << 8);
	if (drive->select.b.unit == 0)
		IDE_DRIVE0_DMA_TIMING_CONFIG_REG = dma_timing_config;
	else
		IDE_DRIVE1_DMA_TIMING_CONFIG_REG = dma_timing_config;
}

void str8100_ide_adjust_udma_timing(ide_drive_t *drive, u8 udma_mode)
{
	u32 udma_Tmli; /* in unit of ns */
	u32 udma_Tenv;
	u32 udma_Tcyc;
	u32 udma_timing_config;
	u32 Tmli, Tenv, Tcyc; /* in unit of UDMA clock */
	u32 dma_udma_clksel = 0, dma_udma_clock, dma_udma_clock_cycle;

	dma_udma_clock = AHB_clock / (dma_udma_clksel + 1);
	dma_udma_clock_cycle = (u32)1000000000 / dma_udma_clock;
	udma_Tmli = udma_mode_timing[udma_mode].Tmli;
	udma_Tenv = udma_mode_timing[udma_mode].Tenv;
	udma_Tcyc = udma_mode_timing[udma_mode].Tcyc;
	Tmli = ((udma_Tmli / dma_udma_clock_cycle) + 0) & 0xF;
	Tenv = ((udma_Tenv / dma_udma_clock_cycle) + 0) & 0xF;    
	Tcyc = ((udma_Tcyc / dma_udma_clock_cycle) + 0) & 0xF;
	if (drive->select.b.unit == 0) {
		udma_timing_config = IDE_UDMA_TIMING_CONFIG_REG;
		udma_timing_config &= ~((0xF << 8) | (0xF << 16) | (0xF << 24));
		udma_timing_config |= (Tmli << 8) | (Tenv << 16) | (Tcyc << 24);
		IDE_UDMA_TIMING_CONFIG_REG = udma_timing_config;
	} else {
		udma_timing_config = IDE_UDMA_TIMING_CONFIG_REG;
		udma_timing_config &= ~((0xF << 12) | (0xF << 20) | ((unsigned int)0xF << 28));
		udma_timing_config |= (Tmli << 12) | (Tenv << 20) | (Tcyc << 28);
		IDE_UDMA_TIMING_CONFIG_REG = udma_timing_config;
	}
}

static u8 str8100_ide_ratemask(ide_drive_t *drive)
{
	/* UDMA 100 capable */
	u8 mode = 3;

#if 0
	/*
	 * If we are UDMA66 capable fall back to UDMA33
	 * if the drive cannot see an 80pin cable.
	 */
	if (!eighty_ninty_three(drive))
		mode = min(mode, (u8)1);
#endif

	return mode;
}

static u8 str8100_ide_dma_2_pio(u8 xfer_rate)
{
	switch(xfer_rate) {
	case XFER_UDMA_6:
	case XFER_UDMA_5:
	case XFER_UDMA_4:
	case XFER_UDMA_3:
	case XFER_UDMA_2:
	case XFER_UDMA_1:
	case XFER_UDMA_0:
	case XFER_MW_DMA_2:
	case XFER_PIO_4:
		return 4;
	case XFER_MW_DMA_1:
	case XFER_PIO_3:
		return 3;
	case XFER_SW_DMA_2:
	case XFER_PIO_2:
		return 2;
	case XFER_MW_DMA_0:
	case XFER_SW_DMA_1:
	case XFER_SW_DMA_0:
	case XFER_PIO_1:
	case XFER_PIO_0:
	case XFER_PIO_SLOW:
	default:
		return 0;
	}
}

static void str8100_ide_tuneproc(ide_drive_t *drive, u8 pio)
{
	unsigned long flags;

	/* pio will be in 0, 1, 2, 3, 4, 5 */
	pio = ide_get_best_pio_mode(drive, pio, 5, NULL);

	spin_lock_irqsave(&ide_lock, flags);
	str8100_ide_adjust_pio_timing(drive, pio);
	spin_unlock_irqrestore(&ide_lock, flags);
}

static int str8100_ide_speedproc(ide_drive_t *drive, u8 xferspeed)
{
	unsigned long flags;
	u8 speed, pio;
	u32 dma_config;

	speed = ide_rate_filter(str8100_ide_ratemask(drive), xferspeed);

	spin_lock_irqsave(&ide_lock, flags);
	if (speed >= XFER_UDMA_0) {
		str8100_ide_adjust_udma_timing(drive, speed - XFER_UDMA_0);
		dma_config = IDE_DMA_UDMA_CONTROL_REG;
		if (drive->select.b.unit == 0) {
			dma_config |= (0x1 << 0);
			dma_config &= ~(0x1 << 2);
		} else {
			dma_config |= (0x1 << 1);
			dma_config &= ~(0x1 << 3);
		}
		IDE_DMA_UDMA_CONTROL_REG = dma_config;
	} else {
		if ((speed >= XFER_MW_DMA_0) && (speed <= XFER_MW_DMA_2)) {
			str8100_ide_adjust_mwdma_timing(drive, speed - XFER_MW_DMA_0);
			dma_config = IDE_DMA_UDMA_CONTROL_REG;
			if (drive->select.b.unit == 0) {
				dma_config |= (0x1 << 2);
				dma_config &= ~(0x1 << 0);
			} else {
				dma_config |= (0x1 << 3);
				dma_config &= ~(0x1 << 1);
			}
			IDE_DMA_UDMA_CONTROL_REG = dma_config;
		}
	}
	spin_unlock_irqrestore(&ide_lock, flags);

	if (speed >= XFER_SW_DMA_0)
		pio = str8100_ide_dma_2_pio(speed);
        else
                pio = speed - XFER_PIO_0;

	str8100_ide_tuneproc(drive, pio);

	return ide_config_drive_speed(drive, speed);
}

static int str8100_ide_config_drive_for_dma(ide_drive_t *drive)
{
	u8 speed = ide_dma_speed(drive, str8100_ide_ratemask(drive));

	/* If no DMA speed was available then disable DMA and use PIO. */
	if (!speed) {
		u8 tspeed = ide_get_best_pio_mode(drive, 255, 5, NULL);
		speed = str8100_ide_dma_2_pio(XFER_PIO_0 + tspeed) + XFER_PIO_0;
	}

	(void) str8100_ide_speedproc(drive, speed);
	return ide_dma_enable(drive);
}

static int str8100_ide_dma_check(ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct hd_driveid *id = drive->id;
	u8 tspeed, speed;

	drive->init_speed = 0;

	if ((id->capability & 1) && drive->autodma) {
		if (ide_use_dma(drive)) {
			if (str8100_ide_config_drive_for_dma(drive)) {
				return hwif->ide_dma_on(drive);
			}
		}
		goto fast_ata_pio;
	} else if ((id->capability & 8) || (id->field_valid & 2)) {
fast_ata_pio:
		tspeed = ide_get_best_pio_mode(drive, 255, 5, NULL);
		speed = str8100_ide_dma_2_pio(XFER_PIO_0 + tspeed) + XFER_PIO_0;
		hwif->speedproc(drive, speed);
		return hwif->ide_dma_off_quietly(drive);
	}
	/* IORDY not supported */
	return 0;
}

static int str8100_ide_dma_setup(ide_drive_t *drive)
{
	ide_hwif_t *hwif = drive->hwif;
	struct request *rq = HWGROUP(drive)->rq;
	u32 dma_config;

	/* fall back to pio! */
	if (!ide_build_dmatable(drive, rq)) {
		ide_map_sg(drive, rq);
		return 1;
	}

#if 0
	{
		int i;
		u32 *prd = hwif->dmatable_cpu;
		for (i = 0; i < 8; i++) {
			printk("IDE PRD[%i] D0: 0x%08x\n", i, prd[i * 2]);
			printk("IDE PRD[%i] D1: 0x%08x\n", i, prd[i * 2 + 1]);
		}
	}
#endif

	/* PRD table */
	IDE_BUS_MASTER_DTP_REG = hwif->dmatable_dma;

	dma_config = IDE_STATUS_CONTROL_REG;

	/* specify r/w */
	if (rq_data_dir(rq))
		dma_config |= (0x1 << 3);
	else
		dma_config &= ~(0x1 << 3);

	/* clear ERROR flags */
	dma_config |= (0x1 << 5);

	/* clear PRD INTR flags */
	dma_config |= (0x1 << 2);

	/* clear DEV INTR flags */
	dma_config |= (0x1 << 1);

	/* PRD interrupt disable */
	dma_config |= (0x1 << 6);

	IDE_STATUS_CONTROL_REG = dma_config;

	drive->waiting_for_dma = 1;

	return 0;
}

static int str8100_ide_dma_timer_expiry(ide_drive_t *drive)
{
	u8 dma_stat = IDE_STATUS_CONTROL_REG;

	printk(KERN_WARNING "%s: str8100_ide_dma_timer_expiry: dma status == 0x%02x\n",
		drive->name, dma_stat);

#if 0
	if ((dma_stat & 0x18) == 0x18)	/* BUSY Stupid Early Timer !! */
		return WAIT_CMD;
#endif

	HWGROUP(drive)->expiry = NULL;	/* one free ride for now */

	if (dma_stat & (0x1 << 5))	/* ERROR */
		return -1;

	if (dma_stat & (0x1 << 4))	/* DMAing */
		return WAIT_CMD;

	if (dma_stat & (0x1 << 1))	/* Got an Interrupt */
		return WAIT_CMD;

	return 0;	/* Status is unknown -- reset the bus */
}

static void str8100_ide_dma_exec_cmd(ide_drive_t *drive, u8 command)
{
	/* issue cmd to drive */
	ide_execute_command(drive, command, &ide_dma_intr, 2*WAIT_CMD, str8100_ide_dma_timer_expiry);
}

static void str8100_ide_dma_start(ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);

	/* Note that this is done *after* the cmd has
	 * been issued to the drive, as per the BM-IDE spec.
	 * The Promise Ultra33 doesn't work correctly when
	 * we do this part before issuing the drive cmd.
	 */
	/* start DMA */
	IDE_STATUS_CONTROL_REG |= (0x1 << 0);
	hwif->dma = 1;
	wmb();
}

static int str8100_ide_dma_end(ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);
	/* get DMA status */
	u8 dma_stat = IDE_STATUS_CONTROL_REG;
	u8 dma_config = dma_stat;

	drive->waiting_for_dma = 0;

	/* stop DMA */
	dma_config &= ~(0x1 << 0);

	/* clear the PRD INTR, DEV INTR & ERROR bits */
	dma_config |= (0x1 << 5) | (0x1 << 2) | (0x1 << 1);

	IDE_STATUS_CONTROL_REG = dma_config;

	/* purge DMA mappings */
	ide_destroy_dmatable(drive);

	/* verify good DMA status */
	hwif->dma = 0;
	wmb();
	dma_stat = (0x1 << 6) | (0x1 << 5) | ((0x1) << 2) |
		(((dma_stat >> 5) & 0x1) << 1) |
		(((dma_stat >> 4) & 0x1) << 0);
	return (dma_stat & 7) != 4 ? (0x10 | dma_stat) : 0;
}

/* returns 1 if dma irq issued, 0 otherwise */
static int str8100_ide_dma_test_irq(ide_drive_t *drive)
{
	/* return 1 if INTR asserted */
	if (IDE_STATUS_CONTROL_REG & 0x2) {
		return 1;
	}
	if (!drive->waiting_for_dma)
		printk(KERN_WARNING "%s: (%s) called while not waiting\n",
			drive->name, __FUNCTION__);
	return 0;
}

static int str8100_ide_dma_host_off(ide_drive_t *drive)
{
	u32 dma_config = IDE_DMA_UDMA_CONTROL_REG;

	if (drive->select.b.unit == 0) {
		dma_config &= ~(0x1 << 0);
		dma_config &= ~(0x1 << 2);
	} else {
		dma_config &= ~(0x1 << 1);
		dma_config &= ~(0x1 << 3);
	}

	IDE_DMA_UDMA_CONTROL_REG = dma_config;

	return 0;
}

static int str8100_ide_dma_host_on(ide_drive_t *drive)
{
	u8 speed = drive->current_speed;
	if (drive->using_dma) {
		u32 dma_config;
		if (speed >= XFER_UDMA_0) {
			str8100_ide_adjust_udma_timing(drive, speed - XFER_UDMA_0);
			dma_config = IDE_DMA_UDMA_CONTROL_REG;
			if (drive->select.b.unit == 0) {
				dma_config |= (0x1 << 0);
				dma_config &= ~(0x1 << 2);
			} else {
				dma_config |= (0x1 << 1);
				dma_config &= ~(0x1 << 3);
			}
			IDE_DMA_UDMA_CONTROL_REG = dma_config;
		} else {
			if ((drive->current_speed >= XFER_MW_DMA_0) && (speed <= XFER_MW_DMA_2)) {
				str8100_ide_adjust_mwdma_timing(drive, speed - XFER_MW_DMA_0);
				dma_config = IDE_DMA_UDMA_CONTROL_REG;
				if (drive->select.b.unit == 0) {
					dma_config |= (0x1 << 2);
					dma_config &= ~(0x1 << 0);
				} else {
					dma_config |= (0x1 << 3);
					dma_config &= ~(0x1 << 1);
				}
				IDE_DMA_UDMA_CONTROL_REG = dma_config;
			}
		}
		return 0;
	}
	return 1;
}

static int str8100_ide_dma_lostirq(ide_drive_t *drive)
{
	printk("%s: DMA interrupt recovery\n", drive->name);
	return 1;
}

int __init str8100_ide_init(void)
{
	hw_regs_t hw;
	ide_hwif_t *hwif = NULL;
	int hwif_idx;

	u32 pio_clksel = 1, dma_udma_clksel = 0, dma_udma_clock_cycle, pio_clock, pio_clock_cycle;
	u32 pio_Ta, pio_T2i, pio_T1, pio_T2, dma_udma_clock;
	u32 dma_Tm, dma_Td, dma_Tk, udma_Tcyc, udma_Tenv, udma_Tmli;

	pio_clock = AHB_clock / (pio_clksel + 1);
	pio_clock_cycle = (u32)1000000000 / pio_clock;
	dma_udma_clock = AHB_clock / (dma_udma_clksel + 1);
	dma_udma_clock_cycle = (u32)1000000000 / dma_udma_clock;
	pio_Ta = ((pio_mode_timing[0].Ta / pio_clock_cycle) + 0) & 0xF;
	pio_T2i = ((pio_mode_timing[0].T2i / pio_clock_cycle) + 0) & 0xF;
	pio_T1 = ((pio_mode_timing[0].T1 / pio_clock_cycle) + 0) & 0xF;
	pio_T2 = ((pio_mode_timing[0].T2 / pio_clock_cycle) + 0) & 0xF;

	dma_Tm = ((dma_mode_timing[0].Tm / dma_udma_clock_cycle) + 0) & 0xF;
	dma_Td = ((dma_mode_timing[0].Td / dma_udma_clock_cycle) + 0) & 0xF;
	dma_Tk = ((dma_mode_timing[0].Tk / dma_udma_clock_cycle) + 0) & 0xF;

	udma_Tcyc = ((udma_mode_timing[0].Tcyc / dma_udma_clock_cycle) + 0) & 0xF;
	udma_Tenv = ((udma_mode_timing[0].Tenv / dma_udma_clock_cycle) + 0) & 0xF;
	udma_Tmli = ((udma_mode_timing[0].Tmli / dma_udma_clock_cycle) + 0) & 0xF;

	HAL_MISC_ENABLE_IDE_PINS();
	HAL_PWRMGT_ENABLE_IDE_CLOCK();
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << PWRMGT_IDE_SOFTWARE_RESET_BIT_INDEX);
	PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << PWRMGT_IDE_SOFTWARE_RESET_BIT_INDEX);
	PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << PWRMGT_IDE_SOFTWARE_RESET_BIT_INDEX);

	IDE_PIO_CONTROL_REG = (pio_clksel << 6) | (dma_udma_clksel << 4) | (0x1 << 2);
	IDE_DRIVE0_PIO_TIMING_CONFIG_REG = (pio_Ta << 12) | (pio_T2i << 8) | (pio_T1 << 4) | (pio_T2);
	IDE_DRIVE1_PIO_TIMING_CONFIG_REG = (pio_Ta << 12) | (pio_T2i << 8) | (pio_T1 << 4) | (pio_T2);
	IDE_DRIVE0_DMA_TIMING_CONFIG_REG = (dma_Tm << 8) | (dma_Td << 4) | (dma_Tk);
	IDE_DRIVE1_DMA_TIMING_CONFIG_REG = (dma_Tm << 8) | (dma_Td << 4) | (dma_Tk);
	IDE_UDMA_TIMING_CONFIG_REG = (udma_Tcyc << 28) | (udma_Tcyc << 24) |
		(udma_Tenv << 20) | (udma_Tenv << 16) |
		(udma_Tmli << 12) | (udma_Tmli << 8);
	IDE_DMA_UDMA_CONTROL_REG = 0;
	IDE_STATUS_CONTROL_REG |= ((0x1 << 6) | (0x1 << 5) | (0x1 << 2) | (0x1 << 1));
	HAL_IDE_DMA_UDMA_STOP();
	if (!str8100_ide_soft_reset()) {
		printk("STR8100 IDE: Software Reset Failed!!\n");
		return -EFAULT;
	}

	memset(&hw, 0x0, sizeof(hw_regs_t));
	hw.io_ports[IDE_DATA_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_DATA_OFFSET;
	hw.io_ports[IDE_ERROR_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_ERROR_OFFSET;
	hw.io_ports[IDE_NSECTOR_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_NSECTOR_OFFSET;
	hw.io_ports[IDE_SECTOR_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_SECTOR_OFFSET;
	hw.io_ports[IDE_LCYL_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_LCYL_OFFSET;
	hw.io_ports[IDE_HCYL_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_HCYL_OFFSET;
	hw.io_ports[IDE_SELECT_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_SELECT_OFFSET;
	hw.io_ports[IDE_STATUS_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_STATUS_OFFSET;
	hw.io_ports[IDE_CONTROL_OFFSET]	= SYSVA_IDE_DEVICE_BASE_ADDR + STR8100_IDE_CONTROL_OFFSET;
	hw.irq = INTC_IDE_BIT_INDEX;
	hw.dma = 1;
	hw.chipset = ide_str8100;

	hwif_idx = ide_register_hw(&hw, &hwif);
	if (hwif_idx == -1) {
		printk("STR8100 IDE: Register IDE fail!!\n");
		return -EFAULT;
	} else {
		printk("STR8100 IDE: Register IDE success\n");
	}

	hwif->tuneproc = &str8100_ide_tuneproc;
	hwif->speedproc = &str8100_ide_speedproc;
	hwif->drives[0].autotune = IDE_TUNE_AUTO;
	hwif->drives[1].autotune = IDE_TUNE_AUTO;

	hwif->dmatable_cpu = pci_alloc_consistent(NULL,
		PRD_ENTRIES * PRD_BYTES,
		&hwif->dmatable_dma);

	if (!hwif->dmatable_cpu) {
		printk("STR8100 IDE: Alloc PRD memory failed!!\n");
		return -EFAULT;
	}

	hwif->ide_dma_off_quietly =	&__ide_dma_off_quietly;
	hwif->ide_dma_on =		&__ide_dma_on;
	hwif->ide_dma_check =		&str8100_ide_dma_check;
	hwif->dma_setup =		&str8100_ide_dma_setup;
	hwif->dma_exec_cmd =		&str8100_ide_dma_exec_cmd;
	hwif->dma_start =		&str8100_ide_dma_start;
	hwif->ide_dma_end =		&str8100_ide_dma_end;
	hwif->ide_dma_test_irq =	&str8100_ide_dma_test_irq;
	hwif->ide_dma_host_off =	&str8100_ide_dma_host_off;
	hwif->ide_dma_host_on =		&str8100_ide_dma_host_on;
	hwif->ide_dma_timeout =		&__ide_dma_timeout;
	hwif->ide_dma_lostirq =		&str8100_ide_dma_lostirq;

	hwif->autodma = 1;
	hwif->atapi_dma = 1;
	hwif->drives[1].autodma = hwif->autodma;
	hwif->drives[0].autodma = hwif->autodma;
	/* FIXME: proper cable detection needed */
	hwif->udma_four = 1;
	hwif->ultra_mask = 0x3f;
	hwif->mwdma_mask = 0x07;
	hwif->swdma_mask = 0x00;

//	hwif->rqsize = 128;

	printk("STR8100 IDE: Register IDE DMA success\n");

	return 0;
}

