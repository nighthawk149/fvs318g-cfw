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


#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/dma.h>
#include <asm/hardware.h>
#include <asm/arch/star_intc.h>
#include <asm/arch/star_spi.h>

//#define STR8100_SPI_DEBUG

struct str8100_spi {
	/* bitbang has to be first */
	struct spi_bitbang	 bitbang;
	struct completion	 done;

	int			len;
	int			count;

	/* data buffers */
	const unsigned char	*tx;
	unsigned char		*rx;

	struct platform_device  *pdev;
	struct spi_master	*master;
	struct spi_device	*spi_dev[4];

	int			board_count;
	struct spi_board_info	board_info[4];
};

extern u32 APB_clock;

static inline u8 str8100_spi_bus_idle(void)
{
	return ((SPI_SERVICE_STATUS_REG & 0x1) ? 0 : 1);
}

static inline u8 str8100_spi_tx_buffer_empty(void)
{
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 3)) ? 1 : 0);
}

static inline u8 str8100_spi_rx_buffer_full(void)
{
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 2)) ? 1 : 0);
}

static u8 str8100_spi_tx_rx(u8 tx_channel, u8 tx_eof_flag, u32 tx_data, u32 *rx_data)
{
	u8 rx_channel;
	u8 rx_eof_flag;

	while (!str8100_spi_bus_idle())
		; // do nothing

	while (!str8100_spi_tx_buffer_empty())
		; // do nothing

	SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
	SPI_TRANSMIT_CONTROL_REG |= (tx_channel & 0x3) | ((tx_eof_flag & 0x1) << 2);

	SPI_TRANSMIT_BUFFER_REG = tx_data;

	while (!str8100_spi_rx_buffer_full())
		; // do nothing

	rx_channel = (SPI_RECEIVE_CONTROL_REG & 0x3);

	rx_eof_flag = (SPI_RECEIVE_CONTROL_REG & (0x1 << 2)) ? 1 : 0;

	*rx_data = SPI_RECEIVE_BUFFER_REG;

	if ((tx_channel != rx_channel) || (tx_eof_flag != rx_eof_flag)) {
		return 0;
	} else {
		return 1;
	}
}

static inline struct str8100_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

static void str8100_spi_chipselect(struct spi_device *spi, int value)
{
	unsigned int spi_config;
	int i;

	switch (value) {
	case BITBANG_CS_INACTIVE:
		break;

	case BITBANG_CS_ACTIVE:
		spi_config = SPI_CONFIGURATION_REG;
		// enable SPI
		spi_config |= (0x1 << 31);
		if (spi->mode & SPI_CPHA)
			spi_config |= (0x1 << 13);
		else
			spi_config &= ~(0x1 << 13);

		if (spi->mode & SPI_CPOL)
			spi_config |= (0x1 << 14);
		else
			spi_config &= ~(0x1 << 14);

		if (spi->controller_data &&
			((struct str8100_spi_dev_attr *)spi->controller_data)->spi_serial_mode ==
			STR8100_SPI_SERIAL_MODE_MICROPROCESSOR) {
			spi_config |= (0x1 << 9);
#ifdef STR8100_SPI_DEBUG
			printk("[STR8100_SPI_DEBUG] Microprocessor mode\n");
#endif
		} else {
			spi_config &= ~(0x1 << 9);
#ifdef STR8100_SPI_DEBUG
			printk("[STR8100_SPI_DEBUG] General mode\n");
#endif
		}

		/* write new configration */
		SPI_CONFIGURATION_REG = spi_config;
#ifdef STR8100_SPI_DEBUG
			printk("[STR8100_SPI_DEBUG] TX chip select:%d\n", spi->chip_select);
#endif
		SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
		SPI_TRANSMIT_CONTROL_REG |= (spi->chip_select & 0x3);

		for (i = 0; i < 8; i++) {
			if (spi->max_speed_hz > (APB_clock >> i))
				break;
		}
		SPI_BIT_RATE_CONTROL_REG = i;
#ifdef STR8100_SPI_DEBUG
		printk("[STR8100_SPI_DEBUG] APB_clock:%u\n", APB_clock);
		printk("[STR8100_SPI_DEBUG] spi->max_speed_hz:%u\n", spi->max_speed_hz);
		printk("[STR8100_SPI_DEBUG] SPI bit rate control val:%d\n", i);
#endif
		break;
	}
}

static int str8100_spi_setup(struct spi_device *spi)
{
	if (!spi->bits_per_word)
		spi->bits_per_word = 8;

	return 0;
}

static int str8100_spi_txrx(struct spi_device *spi, struct spi_transfer *t)
{
	struct str8100_spi *hw = to_hw(spi);

	hw->tx = t->tx_buf;
	hw->rx = t->rx_buf;
	hw->len = t->len;
	hw->count = 0;

#ifdef STR8100_SPI_DEBUG
	printk("[STR8100_SPI_DEBUG] txrx: tx %p, rx %p, len %d\n", t->tx_buf, t->rx_buf, t->len);
	if (hw->tx) {
		int i;
		for (i = 0; i < t->len; i++) {
			printk("[STR8100_SPI_DEBUG] t->tx_buf[%02d]: 0x%02x\n", i, hw->tx[i]);
		}
	}
#endif
	if (hw->tx) {
		int i;
		u32 rx_data;
		for (i = 0; i < (hw->len - 1); i++) {
			str8100_spi_tx_rx(spi->chip_select, 0, hw->tx[i], &rx_data);
			if (hw->rx) {
				hw->rx[i] = rx_data;
#ifdef STR8100_SPI_DEBUG
				printk("[STR8100_SPI_DEBUG] hw->rx[%02d]:0x%02x\n", i, hw->rx[i]);
#endif
			}
		}
		if (t->last_in_message_list) {
			str8100_spi_tx_rx(spi->chip_select, 1, hw->tx[i], &rx_data);
			if (hw->rx) {
				hw->rx[i] = rx_data;
#ifdef STR8100_SPI_DEBUG
				printk("[STR8100_SPI_DEBUG] hw->rx[%02d]:0x%02x\n", i, hw->rx[i]);
#endif
			}
		} else {
			str8100_spi_tx_rx(spi->chip_select, 0, hw->tx[i], &rx_data);
		}
		goto done;
	}

	if (hw->rx) {
		int i;
		u32 rx_data;
		for (i = 0; i < (hw->len - 1); i++) {
			str8100_spi_tx_rx(spi->chip_select, 0, 0xff, &rx_data);
			hw->rx[i] = rx_data;
#ifdef STR8100_SPI_DEBUG
			printk("[STR8100_SPI_DEBUG] hw->rx[%02d]:0x%02x\n", i, hw->rx[i]);
#endif
		}
		if (t->last_in_message_list) {
			str8100_spi_tx_rx(spi->chip_select, 1, 0xff, &rx_data);
			hw->rx[i] = rx_data;
#ifdef STR8100_SPI_DEBUG
			printk("[STR8100_SPI_DEBUG] hw->rx[%02d]:0x%02x\n", i, hw->rx[i]);
#endif
		} else {
			str8100_spi_tx_rx(spi->chip_select, 0, 0xff, &rx_data);
			hw->rx[i] = rx_data;
#ifdef STR8100_SPI_DEBUG
			printk("[STR8100_SPI_DEBUG] hw->rx[%02d]:0x%02x\n", i, hw->rx[i]);
#endif
		}
	}

done:
	return t->len;
}

static int str8100_spi_probe(struct platform_device *pdev)
{
	struct str8100_spi *hw;
	struct spi_master *master;
	unsigned int receive_data;
	int err = 0;

	master = spi_alloc_master(&pdev->dev, sizeof(struct str8100_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "STR8100 SPI: no memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	master->bus_num = 1;
	master->num_chipselect = 4;

	hw = spi_master_get_devdata(master);
	memset(hw, 0, sizeof(struct str8100_spi));
	hw->pdev = pdev;
	hw->master = spi_master_get(master);
	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);

	/* setup the state for the bitbang driver */
	hw->bitbang.master         = hw->master;
	hw->bitbang.chipselect     = str8100_spi_chipselect;
	hw->bitbang.txrx_bufs      = str8100_spi_txrx;
	hw->bitbang.master->setup  = str8100_spi_setup;

	/* register our spi controller */
	err = spi_bitbang_start(&hw->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}

	// Clear spurious interrupt sources
	SPI_INTERRUPT_STATUS_REG = (0xF << 4);

	// Disable SPI interrupt
	SPI_INTERRUPT_ENABLE_REG = 0;

	receive_data = SPI_RECEIVE_BUFFER_REG;

	// Enable SPI
	SPI_CONFIGURATION_REG |= (0x1 << 31);

#ifdef STR8100_SPI_DEBUG
{
	int i;
	u32 rx_data1, rx_data2, rx_data3;

	str8100_spi_tx_rx(0, 0, 0x9f, &rx_data1);
	str8100_spi_tx_rx(0, 0, 0xff, &rx_data1);
	str8100_spi_tx_rx(0, 0, 0xff, &rx_data2);
	str8100_spi_tx_rx(0, 1, 0xff, &rx_data3);
	printk("[STR8100_SPI_DEBUG] manufacturer: %x\n", rx_data1);
	printk("[STR8100_SPI_DEBUG] device:       %x\n", ((rx_data2 & 0xff) << 8) | (u16) (rx_data3 & 0xff));

	str8100_spi_tx_rx(0, 0, 0x03, &rx_data1);
	str8100_spi_tx_rx(0, 0, 0x00, &rx_data1);
	str8100_spi_tx_rx(0, 0, 0x00, &rx_data1);
	str8100_spi_tx_rx(0, 0, 0x00, &rx_data1);
	for (i = 0; i < 15; i++) {
		str8100_spi_tx_rx(0, 0, 0xff, &rx_data1);
		printk("[STR8100_SPI_DEBUG] flash[%02d]:0x%02x\n", i, rx_data1 & 0xff);
	}
	str8100_spi_tx_rx(0, 1, 0xff, &rx_data1);
	printk("[STR8100_SPI_DEBUG] flash[%02d]:0x%02x\n", i, rx_data1 & 0xff);
}
#endif

	return 0;

err_register:
	spi_master_put(hw->master);;

err_nomem:
	return err;
}

static int str8100_spi_remove(struct platform_device *dev)
{
	struct str8100_spi *hw = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	spi_unregister_master(hw->master);

	//str8100_spi_clk_disable();

	spi_master_put(hw->master);

	return 0;
}


#ifdef CONFIG_PM
static int str8100_spi_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct str8100_spi *hw = platform_get_drvdata(pdev);

	//str8100_spi_clk_disable();
	return 0;
}

static int str8100_spi_resume(struct platform_device *pdev)
{
	struct str8100_spi *hw = platform_get_drvdata(pdev);

	//str8100_spi_clk_enable()
	return 0;
}
#else
#define str8100_spi_suspend	NULL
#define str8100_spi_resume	NULL
#endif

static struct platform_driver str8100_spi_driver = {
	.probe		= str8100_spi_probe,
	.remove		= __devexit_p(str8100_spi_remove),
	.suspend	= str8100_spi_suspend,
	.resume		= str8100_spi_resume,
	.driver		= {
		.name	= "str8100_spi",
		.owner	= THIS_MODULE,
	},
};

static void __init str8100_spi_hw_init(void)
{
	u32 receive_data;

	// Enable SPI pins
	HAL_MISC_ENABLE_SPI_PINS();

	// Enable SPI clock
	HAL_PWRMGT_ENABLE_SPI_CLOCK();

	// Disable SPI serial flash access through 0x30000000 region
	HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS();

	/*
	 * Note SPI is NOT enabled after this function is invoked!!
	 */
	SPI_CONFIGURATION_REG =
		(((0x0 & 0x3) << 0) | /* 8bits shift length */
		 (0x0 << 9) | /* general SPI mode */
		 (0x0 << 10) | /* disable FIFO */
		 (0x1 << 11) | /* SPI master mode */
		 (0x0 << 12) | /* disable SPI loopback mode */
		 (0x0 << 13) | /* clock phase */
		 (0x0 << 14) | /* clock polarity */
		 (0x0 << 24) | /* disable SPI Data Swap */
		 (0x0 << 30) | /* disable SPI High Speed Read for BootUp */
		 (0x0 << 31)); /* disable SPI */

	SPI_BIT_RATE_CONTROL_REG = 0x1; // PCLK/2

	// Configure SPI's Tx channel
	SPI_TRANSMIT_CONTROL_REG = 0;

	// Configure Tx FIFO Threshold
	SPI_FIFO_TRANSMIT_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_TRANSMIT_CONFIG_REG |= ((0x0 & 0x03) << 4);

	// Configure Rx FIFO Threshold
	SPI_FIFO_RECEIVE_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_RECEIVE_CONFIG_REG |= ((0x0 & 0x03) << 4);

	SPI_INTERRUPT_ENABLE_REG = 0;

	// Clear spurious interrupt sources
	SPI_INTERRUPT_STATUS_REG = (0xF << 4);

	receive_data = SPI_RECEIVE_BUFFER_REG;

	return;
}

static int __init str8100_spi_init(void)
{
	printk("STR8100 SPI: init\n");
	str8100_spi_hw_init();
	return platform_driver_register(&str8100_spi_driver);
}

static void __exit str8100_spi_exit(void)
{
	platform_driver_unregister(&str8100_spi_driver);
}

module_init(str8100_spi_init);
module_exit(str8100_spi_exit);

MODULE_DESCRIPTION("STR8100 SPI Driver");
MODULE_AUTHOR("STAR Semi Corp.");
MODULE_LICENSE("GPL");

