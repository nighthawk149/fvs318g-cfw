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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/slab.h>

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_i2s.h>

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;

struct str8100_i2s_priv {
	struct snd_card *card;
	struct snd_pcm  *pcm;
};

//=====================================================================
static struct snd_pcm_hardware snd_str8100_i2s_pcm_playback_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
			SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_BLOCK_TRANSFER |
			SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
	.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000,
	.rate_min = 32000,
	.rate_max = 48000,
	.channels_min = 2,
	.channels_max = 2,
	.buffer_bytes_max = 32768,
	.period_bytes_min = 4096,
	.period_bytes_max = 32768,
	.periods_min = 1,
	.periods_max = 1024,
};

/* open callback */
static int snd_str8100_i2s_playback_open(struct snd_pcm_substream *substream)
{
	struct str8100_i2s_priv *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	runtime->hw = snd_str8100_i2s_pcm_playback_hw;
	// more hardware-initialization will be done here
	return 0;
}
/* close callback */
static int snd_str8100_i2s_playback_close(struct snd_pcm_substream *substream)
{
	struct str8100_i2s_priv *chip = snd_pcm_substream_chip(substream);
	// the hardware-specific codes will be here
	return 0;
}
/* hw_params callback */
static int snd_str8100_i2s_pcm_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *hw_params)
{
	return snd_pcm_lib_malloc_pages(substream,
			params_buffer_bytes(hw_params));
}
/* hw_free callback */
static int snd_str8100_i2s_pcm_hw_free(struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_pages(substream);
}
/* prepare callback */
static int snd_str8100_i2s_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct str8100_i2s_priv *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	/* set up the hardware with the current configuration
	 * * for example...
	 * */
#if 0
	str8100_i2s_set_sample_format(chip, runtime->format);
	str8100_i2s_set_sample_rate(chip, runtime->rate);
	str8100_i2s_set_channels(chip, runtime->channels);
	str8100_i2_set_dma_setup(chip, runtime->dma_addr,
			chip->buffer_size,
			chip->period_size);
#endif
	return 0;
}
/* trigger callback */
static int snd_str8100_i2s_pcm_trigger(struct snd_pcm_substream *substream,
		int cmd)
{
	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
			// do something to start the PCM engine
			//I2S_LEFT_TRANSMIT_DATA_REG=xxx;
			//I2S_RIGHT_TRANSMIT_DATA_REG=xxx;
			
			I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG | I2S_TXBF_R_EMPTY_FLAG | I2S_TXBF_L_EMPTY_FLAG);
			HAL_I2S_ENABLE_I2S();
			break;
		case SNDRV_PCM_TRIGGER_STOP:
			// do something to stop the PCM engine
			HAL_I2S_DISABLE_I2S();
			I2S_INTERRUPT_ENABLE_REG = 0x0;
			break;
		default:
			return -EINVAL;
	}
}
/* pointer callback */
static snd_pcm_uframes_t
snd_str8100_i2s_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct str8100_i2s_priv *chip = snd_pcm_substream_chip(substream);
	unsigned int current_ptr;
	/* get the current hardware pointer */
//	current_ptr = str8100_i2s_get_hw_pointer(chip);
	return current_ptr;
}
/* operators */
static struct snd_pcm_ops snd_str8100_i2s_playback_ops = {
	.open = snd_str8100_i2s_playback_open,
	.close = snd_str8100_i2s_playback_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = snd_str8100_i2s_pcm_hw_params,
	.hw_free = snd_str8100_i2s_pcm_hw_free,
	.prepare = snd_str8100_i2s_pcm_prepare,
	.trigger = snd_str8100_i2s_pcm_trigger,
	.pointer = snd_str8100_i2s_pcm_pointer,
};


static int __devinit snd_str8100_i2s_new_pcm(struct str8100_i2s_priv *chip)
{
	struct snd_pcm *pcm;
	int err;
	if ((err = snd_pcm_new(chip->card, "STR8100 I2S PCM", 0, 1, 1,&pcm)) < 0)
		return err;
	pcm->private_data = chip;
	strcpy(pcm->name, "STR8100 I2S PCM");
	chip->pcm = pcm;
	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&snd_str8100_i2s_playback_ops);
	/* pre-allocation of buffers */
	/* NOTE: this may fail */
//	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV, NULL,64*1024, 64*1024);
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS, snd_dma_continuous_data(GFP_KERNEL),64*1024, 64*1024);
	return 0;
}





//=====================================================================
static int snd_str8100_i2s_dev_free(struct snd_device *device)
{
	return 0;
}
//=====================================================================
static struct snd_card *STR8100_I2S_CARD=NULL;
static int __devinit snd_str8100_i2s_probe(void){
	static int dev;
	struct snd_card *card;
	struct str8100_i2s_priv *chip;
	int err;
	static struct snd_device_ops ops = {
		.dev_free = snd_str8100_i2s_dev_free,
	};
	
	if (dev >= SNDRV_CARDS)
		return -ENODEV;
	if (!enable[dev]) {
		dev++;
		return -ENOENT;
	}
	card = snd_card_new(index[dev], id[dev], THIS_MODULE, sizeof(struct str8100_i2s_priv));
	if (card == NULL)
		return -ENOMEM;

	STR8100_I2S_CARD=card;
	chip=card->private_data;
	chip->card = card;

	if ((err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops)) < 0) {
		return err;
	}
	snd_card_set_dev(card, NULL);

	strcpy(card->driver, "STR8100 I2S Audio driver");
	strcpy(card->shortname, "STR8100 I2S Audio driver");
	sprintf(card->longname, "%s",
			card->shortname );

	if ((err = snd_card_register(card)) < 0) {
		snd_card_free(card);
		return err;
	}	
	dev++;

	snd_str8100_i2s_new_pcm(chip);
	return 0;
}
extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);
static irqreturn_t str8100_i2s_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	printk("%s: this_irq=%d, I2S_INTERRUPT_STATUS_REG=0x%.8x\n",__FUNCTION__,this_irq,I2S_INTERRUPT_STATUS_REG);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	//todo:
	
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}

static void __devexit str8100_i2s_exit(void){
	printk("%s: \n",__FUNCTION__);
	if(STR8100_I2S_CARD) snd_card_free(STR8100_I2S_CARD);
	
	return;
}

static int __devinit str8100_i2s_init(void){
	printk("%s: \n",__FUNCTION__);
	PWRMGT_PLL250_CONTROL_REG=0x1;
	int ret;
	if(snd_str8100_i2s_probe()) {
		printk("%s: Probing failed.\n",__FUNCTION__);
		goto exit1;
	}

	str8100_set_interrupt_trigger (INTC_I2S_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);
	if ((ret=request_irq(INTC_I2S_BIT_INDEX, str8100_i2s_irq_handler, 0, "i2s", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_I2S_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}

    // Enable I2S pins
    HAL_MISC_ENABLE_I2S_PINS();

    // Enable I2S clock
    HAL_PWRMGT_ENABLE_I2S_CLOCK(); 

	//Hal_Pwrmgt_Software_Reset(PWRMGT_P2S_SOFTWARE_RESET_BIT_INDEX);

    /*
     * Configure I2S to be Master & Transmitter
     * Note the I2S's WS Clock is derived from Clock & Power Management Functional
     * Block!!
     */
	I2S_CONFIGURATION_REG = (I2S_DATA_16_BIT << 0) |
    			/*(I2S_DATA_32_BIT << 0) |*/
                        (0x0 << 4) |
                        (0x0 << 12) |   /* Disable loopback */
                        (0x0 << 15) |   /* Disable clock phase invert */
                        (0x0 << 24) |   /* Disable I2S data swap */
                        (I2S_CLOCK_256S_MODE << 25) |
                        (I2S_I2S_MODE << 26) |
                        (0x0 << 29) |   /* Enable I2S Transmitter */
			(I2S_MASTER_MODE << 30) |
			(0x0 << 31);	/* Disable I2S */

	//Enable none while initializing
	I2S_INTERRUPT_ENABLE_REG = 0x0;
//	I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG | I2S_TXBF_R_EMPTY_FLAG | I2S_TXBF_L_EMPTY_FLAG);

    // Clear spurious interrupt sources
    I2S_INTERRUPT_STATUS_REG = 0xF0;
    // Disable I2S
    HAL_I2S_DISABLE_I2S();

	return 0;
	
exit1:
	str8100_i2s_exit();
	return -EBUSY ;

}

module_init(str8100_i2s_init);
module_exit(str8100_i2s_exit);

