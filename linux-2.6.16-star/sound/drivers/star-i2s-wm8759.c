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

//*******************
//Options
#define DEBUG
#define LEFT
#define RIGHT
#define I2S_CHANNEL_NUM 2
//*******************
#include <sound/driver.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <asm/delay.h>

#include <asm/arch/star_powermgt.h>
//#include <asm/arch/star_misc.h>
#include <asm/arch/star_i2s.h>
#include <asm/arch/star_dmac.h>
#include <asm/arch/star_demo_dma.h>

MODULE_AUTHOR("Mac Lin");
MODULE_DESCRIPTION("Star I2S + WM8759 ");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA, Star I2S + WM8759}}");

#define MAX_PCM_DEVICES		4
#define MAX_PCM_SUBSTREAMS	16
#define MAX_MIDI_DEVICES	2

/* defaults */
#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE		(64*1024)
#endif

#ifndef add_playback_constraints
#define add_playback_constraints(x) 0
#endif
#ifndef add_capture_constraints
#define add_capture_constraints(x) 0
#endif

static int snd_star_i2s_wm8759_index = 0;	/* Index 0-MAX */
static char *snd_star_i2s_wm8759_id = "[id 1]";	/* ID for this card */
static int pcm_devs = 1;
static int pcm_substreams = 1;
//static int midi_devs[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 2};

static int debug_i2s_src_clk_div=0;
module_param(debug_i2s_src_clk_div, int, 0);
MODULE_PARM_DESC(debug_i2s_src_clk_div, "i2s_src_clk_div 0:disabled 1:div1, 2:div2, 3:div4, 4:div8");


#ifdef DEBUG

static int debug=0;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "debug mode (1=on, 0=off, default=1");
#define LVL_INFO 	1
#define LVL_VERBOSE 	2
#define LVL_INT 	3
#define LVL_INT_VERBOSE	4
#define DEBUG_PRINT(lvl,parm...) if(debug>=lvl) printk(parm)
#else
#define DEBUG_PRINT(lvl,parm...) 
#endif

static struct platform_device *snd_devices;

#define MIXER_ADDR_MASTER	0
#define MIXER_ADDR_LINE		1
#define MIXER_ADDR_MIC		2
#define MIXER_ADDR_SYNTH	3
#define MIXER_ADDR_CD		4
#define MIXER_ADDR_LAST		4

struct snd_star_i2s_wm8759_priv {
	struct snd_card *card;
	struct snd_pcm *pcm;
	spinlock_t mixer_lock;
	int mixer_volume[MIXER_ADDR_LAST+1][2];
	int capture_source[MIXER_ADDR_LAST+1][2];
	struct device* dev;
};

struct snd_star_i2s_wm8759_pcm_priv {
	struct snd_star_i2s_wm8759_priv *card_priv;
	spinlock_t lock;
	struct timer_list timer;
	unsigned int buffer_size_in_bytes;
        unsigned int period_size_in_bytes;
	unsigned int pcm_bps;		/* bytes per second */
	unsigned int pcm_jiffie;	/* bytes per one jiffie */

	unsigned int pcm_irq_pos[I2S_CHANNEL_NUM];	/* IRQ position */
	unsigned int pcm_buf_pos[I2S_CHANNEL_NUM];	/* position in buffer */

	u32 period_count_in_buffer; 	//(buffer_size/period_size)
	u32 desc_count;		//period_count_in_buffer*channels

	u32 desc_size_in_bytes;	//desc_count*sizeof(DMAC_LLP_T);
	DMAC_LLP_T* desc;	//vir addr of desc buffer allocated
	DMAC_LLP_T* desc_p;	//phy addr of desc buffer allocated

	struct snd_pcm_substream *substream;
	
};

#ifdef DEBUG
static void debug_print_info(u32 lvl, const char* func_name,char* more,struct snd_pcm_substream *substream){
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_star_i2s_wm8759_pcm_priv *dpcm = runtime->private_data;

	//only print func name and little info in previous debug lvl
	//currently lvl would be LVL_VERBOSE or LVL_INT_VERBOSE
	//then, their previous level is LVL_INFO and LVL_INT
	if(debug>=(lvl-1))
		printk("%s\n%s: %sformat_width=%d\n",(debug<lvl)?"":"-",func_name,more,snd_pcm_format_width(runtime->format));
	if(debug<lvl) return;

printk("   runtime ");	
//#define MYPRINT(xx,fmt,args...) printk(#xx"="fmt" ")
#define MYPRINT(head,xx,fmt,args...) printk(#xx"="fmt" ",head->xx,args)
#define MYPRINT_STD(head, xx, fmt)   printk(#xx"="fmt" ", head->xx)
#define MYPRINT_HEX(head, xx, fmt)   printk(#xx"="fmt" ", head->xx, head->xx)

	MYPRINT_STD(runtime, rate,         "%d");
	MYPRINT_STD(runtime, channels,     "%d");
	MYPRINT_HEX(runtime, buffer_size,  "%ld(%#lx)");
	MYPRINT_HEX(runtime, period_size,  "%ld(%#lx)");
	MYPRINT_STD(runtime, frame_bits,   "%d");
	MYPRINT_STD(runtime, sample_bits,  "%d");
printk("\n           ");
	MYPRINT_STD(runtime, dma_area,     "%p");
	MYPRINT_STD(runtime, dma_addr,     "%#x");
	MYPRINT_HEX(runtime, dma_bytes,    "%d(%#x)");
printk("\n           ");
	MYPRINT_STD(runtime, dma_buffer_p, "%p");
	if(runtime->dma_buffer_p){
		MYPRINT_STD(runtime->dma_buffer_p, area,        "%p");
		MYPRINT_STD(runtime->dma_buffer_p, addr,        "%#x");
		MYPRINT_STD(runtime->dma_buffer_p, bytes,       "%d");
		MYPRINT_STD(runtime->dma_buffer_p, private_data,"%p");
	}
printk("\n           param: ");
	MYPRINT_STD(runtime, start_threshold,      "%ld");
	MYPRINT_STD(runtime, stop_threshold,       "%ld");
	MYPRINT_STD(runtime, xfer_align,           "%ld");
	MYPRINT_STD(runtime, boundary,             "%ld");
printk("\n                  ");
	MYPRINT_STD(runtime, silence_threshold,    "%ld");
	MYPRINT_STD(runtime, silence_size,         "%ld");
	MYPRINT_STD(runtime, silence_start,        "%ld");
	MYPRINT_STD(runtime, silence_filled,       "%ld");

printk("\n   dpcm ");
	MYPRINT_STD(dpcm,    buffer_size_in_bytes, "%d");
	MYPRINT_STD(dpcm,    period_size_in_bytes, "%d");
	MYPRINT_STD(dpcm,    pcm_bps,              "%d");
	MYPRINT_STD(dpcm,    pcm_jiffie,           "%d");
printk("\n        ");
	MYPRINT_STD(dpcm,    pcm_irq_pos[0],       "%d");
	MYPRINT_STD(dpcm,    pcm_buf_pos[0],       "%d");
	MYPRINT_STD(dpcm,    pcm_irq_pos[1],       "%d");
	MYPRINT_STD(dpcm,    pcm_buf_pos[1],       "%d");
//printk("    dma_area %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n\n",*((char*)runtime->dma_area+0x0),*((char*)runtime->dma_area+0x1),*((char*)runtime->dma_area+0x2),*((char*)runtime->dma_area+0x3),*((char*)runtime->dma_area+0x4),*((char*)runtime->dma_area+0x5),*((char*)runtime->dma_area+0x6),*((char*)runtime->dma_area+0x7) );
//	MYPRINT(,"%d",NULL);
printk("\n\n");
}
#else
#define debug_print_info(args...)
#endif

#define DMA_TRANSFER_MAX_BYTE(sample_size)    (0xfff<<(sample_size>>4)) 
//#define DMA_TRANSFER_MAX_BYTE(sample_size)    (0x7ff<<(sample_size>>4)) 
//static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8759_dma_handshake_left_tx;
//static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8759_dma_handshake_right_tx;
#define CHANNEL_CHUNK_SIZE (dpcm->period_size_in_bytes>>1)
static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8759_dma_handshake_tx[I2S_CHANNEL_NUM];

static void I2s_WM8759_Configure_DMA_Hardware_Handshake(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	//struct snd_star_i2s_wm8759_pcm_priv *dpcm = runtime->private_data;
	//unsigned int bps;
	u32 i;

	debug_print_info(LVL_VERBOSE,__FUNCTION__," ",substream);

	for(i=0;i<I2S_CHANNEL_NUM;i++){
		i2s_wm8759_dma_handshake_tx[i].target_select = (i==1)?DMAC_HW_HAND_SHAKE_I2S_TX_LEFT_ID:DMAC_HW_HAND_SHAKE_I2S_TX_RIGHT_ID;
		i2s_wm8759_dma_handshake_tx[i].private_data=substream;
		i2s_wm8759_dma_handshake_tx[i].channel_num = i;
		i2s_wm8759_dma_handshake_tx[i].channel_id = DMAC_CH_ID(i2s_wm8759_dma_handshake_tx[i].channel_num);
		
		i2s_wm8759_dma_handshake_tx[i].src_addr = i2s_wm8759_dma_handshake_tx[i].llp_head->SrcAddr;
		i2s_wm8759_dma_handshake_tx[i].dst_addr = i2s_wm8759_dma_handshake_tx[i].llp_head->DstAddr;

		i2s_wm8759_dma_handshake_tx[i].src_master = i2s_wm8759_dma_handshake_tx[i].llp_head->Ctrl_TotSize.src_sel;
		i2s_wm8759_dma_handshake_tx[i].dst_master = i2s_wm8759_dma_handshake_tx[i].llp_head->Ctrl_TotSize.dst_sel;
		i2s_wm8759_dma_handshake_tx[i].srcad_ctl = i2s_wm8759_dma_handshake_tx[i].llp_head->Ctrl_TotSize.srcad_ctl;
		i2s_wm8759_dma_handshake_tx[i].dstad_ctl = i2s_wm8759_dma_handshake_tx[i].llp_head->Ctrl_TotSize.dstad_ctl;

		i2s_wm8759_dma_handshake_tx[i].src_width = i2s_wm8759_dma_handshake_tx[i].llp_head->Ctrl_TotSize.src_width;
		i2s_wm8759_dma_handshake_tx[i].dst_width = i2s_wm8759_dma_handshake_tx[i].llp_head->Ctrl_TotSize.dst_width;

		i2s_wm8759_dma_handshake_tx[i].transfer_bytes = i2s_wm8759_dma_handshake_tx[i].llp_head->Ctrl_TotSize.tot_size<<(snd_pcm_format_width(runtime->format)>>3);

		i2s_wm8759_dma_handshake_tx[i].src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;   
		i2s_wm8759_dma_handshake_tx[i].llp_addr = (u32)i2s_wm8759_dma_handshake_tx[i].llp_head->LLP;
		
		//current struct can't set TC_MASK of first desc,
		//    but currently the default is set to disabled, just what we want.
		//    So fix it some other day.
		//llp->Ctrl_TotSize.tc_status_mask = (DMAC_CH_TC_MASK_DISABLE);
		
		Hal_Dmac_Configure_DMA_Handshake(&i2s_wm8759_dma_handshake_tx[i]);
	}

    return;
}

extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);
static irqreturn_t str8100_dma_tc_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	u32 dma_tc_status;
	u32 i;
	DEBUG_PRINT(LVL_INT,"%s: this_irq=%d\n",__FUNCTION__,this_irq);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	//todo:

	HAL_DMAC_READ_TERMINAL_COUNT_INTERRUPT_STATUS(dma_tc_status);

	for(i=0;i<I2S_CHANNEL_NUM;i++){
		if (dma_tc_status & DMAC_CH_ID(i2s_wm8759_dma_handshake_tx[i].channel_num)){
			struct snd_pcm_substream *substream=(struct snd_pcm_substream *)i2s_wm8759_dma_handshake_tx[i].private_data;
			struct snd_pcm_runtime *runtime = substream->runtime;
			struct snd_star_i2s_wm8759_pcm_priv *dpcm = (struct snd_star_i2s_wm8759_pcm_priv *)runtime->private_data;
			HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i2s_wm8759_dma_handshake_tx[i].channel_num));
			dma_tc_status &= ~(DMAC_CH_ID(i2s_wm8759_dma_handshake_tx[i].channel_num));
			
			//currently assume period size (in frames) is the DMA totsize
			dpcm->pcm_irq_pos[i] += dpcm->period_size_in_bytes/runtime->channels;
			dpcm->pcm_buf_pos[i] += dpcm->period_size_in_bytes/runtime->channels;
			dpcm->pcm_buf_pos[i] %= dpcm->buffer_size_in_bytes/runtime->channels;

                        if (dpcm->pcm_irq_pos[i] >= (dpcm->period_size_in_bytes/runtime->channels)) {
                                dpcm->pcm_irq_pos[i] %= (dpcm->period_size_in_bytes/runtime->channels);
				DEBUG_PRINT(LVL_INT,"%s: -> snd_pcm_period_elapsed\n",__FUNCTION__);
				snd_pcm_period_elapsed(dpcm->substream);
			}
//printk(" SIZE_ADDR_REG=%#x",DMAC_CH_SIZE_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
//printk(" %d LLP_REG=%#x\n",i2s_wm8759_dma_handshake_tx[i].channel_num,DMAC_CH_LLP_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
//printk(" SRC_ADDR_REG=%#x",DMAC_CH_SRC_ADDR_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
#ifdef DEBUG
			if(debug>=4){
				printk("%s: ch %d",__FUNCTION__,i2s_wm8759_dma_handshake_tx[i].channel_num);
				printk(" SRC_ADDR_REG=%#x",DMAC_CH_SRC_ADDR_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
				printk(" DST_ADDR_REG=%#x",DMAC_CH_DST_ADDR_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
				printk("\n    ");
				printk(" CFG_REG=%#x",DMAC_CH_CFG_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
				printk(" LLP_REG=%#x",DMAC_CH_LLP_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
				printk(" SIZE_ADDR_REG=%#x",DMAC_CH_SIZE_REG(i2s_wm8759_dma_handshake_tx[i].channel_num));
				printk("\n");
			}
#endif//#ifdef DEBUG
		}
	}

//printk("-\n");
	//error handling
	if(dma_tc_status) 
		for(i=0;i<8;i++)
			if (dma_tc_status & DMAC_CH_ID(i)){                      
				HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i));
				printk("%s: channel %d: Unhandled DMA TC interrupt\n\n",__FUNCTION__,i);
			}

	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}

static char *i2s_status_txt[]={
				"Right channel RxBuf Full",
				"Right channel TxBuf Empty",
				"Left channel RxBuf Full",
				"Left channel TxBuf Empty",
				"Right channel RxBuf Overrun",
				"Right channel TxBuf Underrun",
				"Left channel RxBuf Overrun",
				"Left channel TxBuf Underrun"
				};
static u32 i2s_err=0;
static irqreturn_t str8100_i2s_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	u32 interrupt_status,i;
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	DEBUG_PRINT(LVL_INT,"%s: this_irq=%d, I2S_INTERRUPT_STATUS_REG=0x%.8x\n",__FUNCTION__,this_irq,I2S_INTERRUPT_STATUS_REG);

#ifdef DEBUG
	if(debug>=2){
		//todo:
		interrupt_status = I2S_INTERRUPT_STATUS_REG;
		
		if(debug>=3){
			for(i=0;i<4;i++){
				if(interrupt_status&(0x1<<i))
					printk("%s: %s\n",__FUNCTION__,i2s_status_txt[i]);
			}
		}
	
		for(i=4;i<8;i++){
			if(interrupt_status&(0x1<<i))
				printk("[Error] %s: %s\n",__FUNCTION__,i2s_status_txt[i]);
		}
	}
#endif

//interrupt_status = I2S_INTERRUPT_STATUS_REG;if(interrupt_status&(0xf0)) printk(".\n");

	I2S_INTERRUPT_STATUS_REG &= 0xf0;
i2s_err++;
	
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}


static int snd_card_star_i2s_wm8759_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_star_i2s_wm8759_pcm_priv *dpcm = runtime->private_data;
	int err = 0;

	spin_lock(&dpcm->lock);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_RESUME:
		DEBUG_PRINT(LVL_INFO,"%s: (resume)\n",__FUNCTION__);
		break;
	case SNDRV_PCM_TRIGGER_START:
		debug_print_info(LVL_VERBOSE,__FUNCTION__,"(start) ",substream);

		i2s_err=0;
		I2S_INTERRUPT_ENABLE_REG = (
#ifdef RIGHT
		I2S_TXBF_R_UR_FLAG | 
#endif
#ifdef LEFT
		I2S_TXBF_L_UR_FLAG |
#endif
		0);
//		I2S_INTERRUPT_ENABLE_REG = (I2S_TXBF_L_UR_FLAG);
//		I2S_INTERRUPT_ENABLE_REG = (I2S_TXBF_R_UR_FLAG);
//		I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG | I2S_TXBF_R_EMPTY_FLAG | I2S_TXBF_L_EMPTY_FLAG);
		HAL_DMAC_ENABLE();
		HAL_I2S_ENABLE_I2S();
		snd_pcm_period_elapsed(dpcm->substream);
		
//		snd_card_star_i2s_wm8759_pcm_timer_start(dpcm);
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
		DEBUG_PRINT(LVL_INFO,"%s: (suspend)\n",__FUNCTION__);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		DEBUG_PRINT(LVL_INFO,"%s: (stop)\n",__FUNCTION__);
#ifdef DEBUG
		DEBUG_PRINT(LVL_INFO,"    i2s_err=%d\n\n",i2s_err);
		else 
#endif
		if (i2s_err>0) printk("%s: Error: i2s_err=%d !!!\n\n",__FUNCTION__,i2s_err);

		HAL_I2S_DISABLE_I2S();
		HAL_DMAC_DISABLE();
		I2S_INTERRUPT_ENABLE_REG = 0x0;

//		snd_card_star_i2s_wm8759_pcm_timer_stop(dpcm);
		break;
	default:
		DEBUG_PRINT(LVL_INFO,"%s unknown cmd\n",__FUNCTION__);
		err = -EINVAL;
		break;
	}
	spin_unlock(&dpcm->lock);
	return 0;
}

static int snd_card_star_i2s_wm8759_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_star_i2s_wm8759_pcm_priv *dpcm = runtime->private_data;
	unsigned int bps;
	u32 tmp,ch,i;
	DMAC_LLP_T* llp;
	u32 ch_dst_addr=0, addr_shift=0, src_width=0, dst_width=0,tot_size=0, ch_period_size_in_bytes=0;

//DEBUG_PRINT(LVL_INFO,"%s:before: runtime start_threshold=%lu\n", __FUNCTION__,runtime->start_threshold);
//#define MIN_START (1024*14)
//if(runtime->start_threshold<MIN_START) runtime->start_threshold=MIN_START;

	debug_print_info(LVL_VERBOSE,__FUNCTION__," ",substream);

/*	if(runtime->rate!=32000&&runtime->rate!=44100&&runtime->rate!=48000){
		printk("%s: invalid sampling rate(%d)\n",__FUNCTION__,runtime->rate);
		return -EFAULT;
	}
*/
	if(snd_pcm_format_width(runtime->format)!=16&&snd_pcm_format_width(runtime->format)!=32){
		printk("%s: invalid sample size(%d)\n",__FUNCTION__,snd_pcm_format_width(runtime->format));
		return -EFAULT;
	}
	if(runtime->channels>2||runtime->channels<1){
		printk("%s: invalid channel number(%d)\n",__FUNCTION__,runtime->channels);
	}

	bps = runtime->rate * runtime->channels;
	bps *= snd_pcm_format_width(runtime->format);
	bps /= 8;
	if (bps <= 0)
		return -EINVAL;
	dpcm->pcm_bps = bps;
	dpcm->pcm_jiffie = bps / HZ;
	dpcm->buffer_size_in_bytes = snd_pcm_lib_buffer_bytes(substream);
        dpcm->period_size_in_bytes = snd_pcm_lib_period_bytes(substream);
	dpcm->pcm_irq_pos[0] = 0;
	dpcm->pcm_buf_pos[0] = 0;
	dpcm->pcm_irq_pos[1] = 0;
	dpcm->pcm_buf_pos[1] = 0;

	if(str8100_i2s_init(runtime->rate,I2S_DATA_32_BIT, I2S_MASTER_MODE, I2S_I2S_MODE, I2S_CLOCK_256S_MODE)){

		printk("%s: str8100_i2s_init failed(sampling_rate=%d)\n",__FUNCTION__,runtime->rate);
		return -EFAULT;
	}
	if(debug_i2s_src_clk_div>0){
		I2S_CONFIGURATION_REG&=(~0x3);
		I2S_CONFIGURATION_REG|=(((debug_i2s_src_clk_div-1)&0x3)<<4);
		DEBUG_PRINT(LVL_INFO,"%s: debug_i2s_src_clk_div=%d,I2S_CONFIGURATION_REG=%#.8x\n",__FUNCTION__,debug_i2s_src_clk_div,I2S_CONFIGURATION_REG);
	}

if(runtime->buffer_size%runtime->period_size)
	printk("%s: warning! buffer_size(%ld) is not integer times  period_size(%ld)!!\n"
	       "   the last link-list descriptor totsize is wrong. fix it!\n\n",
	       __FUNCTION__, runtime->buffer_size, runtime->period_size);
if(runtime->period_size>0x1000)
	printk("%s: period_size(%ld) (in frames) is larger than max size that dma can process(4096) in one time\n"
	       "    this is not implemented. fix it!\n\n",
	       __FUNCTION__, runtime->period_size);
	dpcm->period_count_in_buffer=((runtime->buffer_size/runtime->period_size)+runtime->buffer_size%runtime->period_size);
	dpcm->desc_count=dpcm->period_count_in_buffer*runtime->channels;
	
	tmp=dpcm->desc_count*sizeof(DMAC_LLP_T);
	//if the desc buffer is already allocated and is exactly the size we need,
	//then we don't have to allocate it again.
	if((!dpcm->desc||!dpcm->desc_size_in_bytes||!(dpcm->desc_size_in_bytes==tmp))){
		//check if the desc buffer is already allocated.
		if(dpcm->desc) {
			pci_free_consistent(NULL, dpcm->desc_size_in_bytes, dpcm->desc, (dma_addr_t)dpcm->desc_p);
			dpcm->desc=dpcm->desc_p=NULL;
			dpcm->desc_size_in_bytes=0;
		}
		dpcm->desc_size_in_bytes=tmp;
		dpcm->desc = pci_alloc_consistent(NULL, dpcm->desc_size_in_bytes, (dma_addr_t *)(&dpcm->desc_p));
		if(!dpcm->desc){
			printk("%s: alloc buffer for desc failed.\n",__FUNCTION__);
			return -ENOMEM;
		}
	}

	memset ((void *)dpcm->desc, 0x0, dpcm->desc_size_in_bytes);

	switch(snd_pcm_format_width(runtime->format)){
	case 16:
		addr_shift=2;
		src_width = DMAC_CH_SRC_WIDTH_16_BITS;
		dst_width = DMAC_CH_DST_WIDTH_16_BITS;
		break;
	case 32:
		addr_shift=0;
		src_width = DMAC_CH_SRC_WIDTH_32_BITS;
		dst_width = DMAC_CH_DST_WIDTH_32_BITS;
		break;
	}
	ch_period_size_in_bytes=dpcm->period_size_in_bytes/runtime->channels;
	tot_size = Hal_Dmac_Get_Channel_Transfer_Unit_Number(ch_period_size_in_bytes, src_width);
	for(ch=0;ch<runtime->channels;ch++){
		i2s_wm8759_dma_handshake_tx[ch].llp_head=&dpcm->desc[ch*dpcm->period_count_in_buffer];
		//ch0 right, ch1 left
		if(ch==0) ch_dst_addr=(SYSPA_I2S_BASE_ADDR + 0xc4);
		else ch_dst_addr=(SYSPA_I2S_BASE_ADDR + 0xC8);
		ch_dst_addr+=addr_shift;
#define CUR_IDX (i+ch*dpcm->period_count_in_buffer)
		for(i=0;i<dpcm->period_count_in_buffer;i++){
			llp=&dpcm->desc[CUR_IDX];
		
			llp->SrcAddr = (u32)runtime->dma_addr+CUR_IDX*ch_period_size_in_bytes;
			llp->DstAddr = ch_dst_addr;

			//make it a ring llp
			llp->LLP = (i != (dpcm->period_count_in_buffer - 1)) ? 
				(DMAC_LLP_T *)&dpcm->desc_p[CUR_IDX+1] :
				(DMAC_LLP_T *)&dpcm->desc_p[ch*dpcm->period_count_in_buffer];

			llp->Ctrl_TotSize.tot_size = tot_size;  /* TOT_SIZE */
			llp->Ctrl_TotSize.dst_sel = DMAC_CH_DST_SEL_M0;  /* DST_SEL */
			llp->Ctrl_TotSize.src_sel = DMAC_CH_SRC_SEL_M1;  /* SRC_SEL */
			llp->Ctrl_TotSize.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;  /* DSTAD_CTL */
			llp->Ctrl_TotSize.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;  /* SRCAD_CTL */
			llp->Ctrl_TotSize.dst_width = dst_width;  /* DST_WIDTH */
			llp->Ctrl_TotSize.src_width = src_width;  /* SRC_WIDTH */       
			
			//interrupt whenever a descriptor finished
			llp->Ctrl_TotSize.tc_status_mask = (DMAC_CH_TC_MASK_DISABLE);
			//llp->Ctrl_TotSize.tc_status_mask = (DMAC_CH_TC_MASK_ENABLE);
			//llp->Ctrl_TotSize.tc_status_mask = ((ii != (I2S_WM8772_DMAC_LLP_NUM - 1)) ? (DMAC_CH_TC_MASK_ENABLE) : (DMAC_CH_TC_MASK_DISABLE)); 
		}
	}

	
	I2s_WM8759_Configure_DMA_Hardware_Handshake(substream);  

	I2S_RIGHT_TRANSMIT_DATA_REG=0;
	I2S_LEFT_TRANSMIT_DATA_REG=0;
	tmp=I2S_RIGHT_RECEIVE_DATA_REG;
	tmp=I2S_LEFT_RECEIVE_DATA_REG;

	DEBUG_PRINT(LVL_INFO,"%s: exit\n",__FUNCTION__);
	return 0;
}


static snd_pcm_uframes_t snd_card_star_i2s_wm8759_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_star_i2s_wm8759_pcm_priv *dpcm = runtime->private_data;
	debug_print_info(LVL_INT_VERBOSE,__FUNCTION__," ",substream);
	
#ifdef LEFT
	return bytes_to_frames(runtime, dpcm->pcm_buf_pos[1]);
#else //#ifdef LEFT

#ifdef RIGHT
	return bytes_to_frames(runtime, dpcm->pcm_buf_pos[0]);
#else //#ifdef RIGHT
this is error condition...
#endif//#ifdef RIGHT

#endif//#ifdef LEFT

}
static struct snd_pcm_hardware snd_card_star_i2s_wm8759_playback = {
	.info =			(SNDRV_PCM_INFO_MMAP | 
				 /*SNDRV_PCM_INFO_INTERLEAVED |*/SNDRV_PCM_INFO_NONINTERLEAVED |
				 SNDRV_PCM_INFO_RESUME | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = 		SNDRV_PCM_FMTBIT_S16_LE  /*| SNDRV_PCM_FMTBIT_S24_LE*/ | SNDRV_PCM_FMTBIT_S32_LE,
	.rates = 		SNDRV_PCM_RATE_8000_48000,//SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000,
	.rate_min = 		/*32000*/8000,
	.rate_max = 		48000,
	.channels_min = 	I2S_CHANNEL_NUM,
	.channels_max = 	I2S_CHANNEL_NUM,
	.buffer_bytes_max = 	MAX_BUFFER_SIZE,
	.period_bytes_min = 	64,
	.period_bytes_max = 	MAX_BUFFER_SIZE,
	.periods_min = 		1,
//mkl071008: setting periods_min to 2048 would cause madplay failed.
//	     output: ioctl(SNDCTL_DSP_SYNC): Invalid argument
//	.periods_min = 		2048,

//	.periods_max = 		1024,
	.periods_max = 		4096,
	.fifo_size =		0,
};

static void snd_card_star_i2s_wm8759_runtime_free(struct snd_pcm_runtime *runtime)
{
//	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_star_i2s_wm8759_pcm_priv *dpcm = runtime->private_data;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	if(dpcm->desc) {
		pci_free_consistent(NULL, dpcm->desc_size_in_bytes, dpcm->desc, (dma_addr_t)dpcm->desc_p);
		dpcm->desc=dpcm->desc_p=NULL;
		dpcm->desc_size_in_bytes=0;
	}

	kfree(runtime->private_data);

}

static int snd_card_star_i2s_wm8759_hw_params(struct snd_pcm_substream *substream,
				    struct snd_pcm_hw_params *hw_params)
{
	//struct snd_pcm_runtime *runtime = substream->runtime;
	//struct snd_star_i2s_wm8759_pcm_priv *dpcm = runtime->private_data;
	int ret;

	debug_print_info(LVL_VERBOSE,__FUNCTION__," ",substream);
	ret=snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(hw_params));
	DEBUG_PRINT(LVL_INFO,"%s: ret=%#x\n",__FUNCTION__,ret);
	return ret;
}

static int snd_card_star_i2s_wm8759_hw_free(struct snd_pcm_substream *substream)
{
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	
	return snd_pcm_lib_free_pages(substream);
}

static struct snd_star_i2s_wm8759_pcm_priv *new_pcm_stream(struct snd_pcm_substream *substream)
{
	struct snd_star_i2s_wm8759_pcm_priv *dpcm;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	dpcm = kzalloc(sizeof(*dpcm), GFP_KERNEL);
	memset((void*) dpcm, 0, sizeof(*dpcm));
	if (! dpcm)
		return dpcm;
	spin_lock_init(&dpcm->lock);
	dpcm->substream = substream;
	return dpcm;
}

static int snd_card_star_i2s_wm8759_playback_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_star_i2s_wm8759_pcm_priv *dpcm;
	int err;
	DEBUG_PRINT(LVL_INFO,"%s: \n",__FUNCTION__);
	
	if ((dpcm = new_pcm_stream(substream)) == NULL)
		return -ENOMEM;
	runtime->private_data = dpcm;
	runtime->private_free = snd_card_star_i2s_wm8759_runtime_free;
	runtime->hw = snd_card_star_i2s_wm8759_playback;
	if (substream->pcm->device & 1) {
		runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
		runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
	}
	if (substream->pcm->device & 2)
		runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP|SNDRV_PCM_INFO_MMAP_VALID);
	if ((err = add_playback_constraints(runtime)) < 0) {
		kfree(dpcm);
		return err;
	}

	return 0;
}


static int snd_card_star_i2s_wm8759_playback_close(struct snd_pcm_substream *substream)
{
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	return 0;
}


static struct snd_pcm_ops snd_card_star_i2s_wm8759_playback_ops = {
	.open =			snd_card_star_i2s_wm8759_playback_open,
	.close =		snd_card_star_i2s_wm8759_playback_close,
	.ioctl =		snd_pcm_lib_ioctl,
	.hw_params =		snd_card_star_i2s_wm8759_hw_params,
	.hw_free =		snd_card_star_i2s_wm8759_hw_free,
	.prepare =		snd_card_star_i2s_wm8759_pcm_prepare,
	.trigger =		snd_card_star_i2s_wm8759_pcm_trigger,
	.pointer =		snd_card_star_i2s_wm8759_pcm_pointer,
};


static int __init snd_card_star_i2s_wm8759_new_pcm(struct snd_star_i2s_wm8759_priv *card_priv, int pcm_device, int substreams)
{
	struct snd_pcm *pcm;
	int err;
DEBUG_PRINT(LVL_INFO,"%s: \n",__FUNCTION__);

	if ((err = snd_pcm_new(card_priv->card, "[PCM id 2]", pcm_device, substreams, 0, &pcm)) < 0)
		return err;
	card_priv->pcm = pcm;
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_card_star_i2s_wm8759_playback_ops);
	pcm->private_data = card_priv;
	pcm->info_flags = 0;
	strcpy(pcm->name, "[PCM id 3]");
//	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS, snd_dma_continuous_data(GFP_KERNEL), 0, 64*1024);

//	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV, card_priv->dev, 0, 64*1024);
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV, snd_dma_isa_data(), 64*1024, 64*1024);
	return 0;
}

#define STAR_I2S_WM8759_VOLUME(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, .index = xindex, \
  .info = snd_star_i2s_wm8759_volume_info, \
  .get = snd_star_i2s_wm8759_volume_get, .put = snd_star_i2s_wm8759_volume_put, \
  .private_value = addr }

#if 0
static int snd_star_i2s_wm8759_volume_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = -50;
	uinfo->value.integer.max = 100;
	return 0;
}
 
static int snd_star_i2s_wm8759_volume_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_star_i2s_wm8759_priv *card_priv = snd_kcontrol_chip(kcontrol);
	int addr = kcontrol->private_value;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	spin_lock_irq(&card_priv->mixer_lock);
	ucontrol->value.integer.value[0] = card_priv->mixer_volume[addr][0];
	ucontrol->value.integer.value[1] = card_priv->mixer_volume[addr][1];
	spin_unlock_irq(&card_priv->mixer_lock);
	return 0;
}

static int snd_star_i2s_wm8759_volume_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_star_i2s_wm8759_priv *card_priv = snd_kcontrol_chip(kcontrol);
	int change, addr = kcontrol->private_value;
	int left, right;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	left = ucontrol->value.integer.value[0];
	if (left < -50)
		left = -50;
	if (left > 100)
		left = 100;
	right = ucontrol->value.integer.value[1];
	if (right < -50)
		right = -50;
	if (right > 100)
		right = 100;
	spin_lock_irq(&card_priv->mixer_lock);
	change = card_priv->mixer_volume[addr][0] != left ||
	         card_priv->mixer_volume[addr][1] != right;
	card_priv->mixer_volume[addr][0] = left;
	card_priv->mixer_volume[addr][1] = right;
	spin_unlock_irq(&card_priv->mixer_lock);
	return change;
}

#define STAR_I2S_WM8759_CAPSRC(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, .index = xindex, \
  .info = snd_star_i2s_wm8759_capsrc_info, \
  .get = snd_star_i2s_wm8759_capsrc_get, .put = snd_star_i2s_wm8759_capsrc_put, \
  .private_value = addr }

static int snd_star_i2s_wm8759_capsrc_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
 
static int snd_star_i2s_wm8759_capsrc_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_star_i2s_wm8759_priv *card_priv = snd_kcontrol_chip(kcontrol);
	int addr = kcontrol->private_value;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	spin_lock_irq(&card_priv->mixer_lock);
	ucontrol->value.integer.value[0] = card_priv->capture_source[addr][0];
	ucontrol->value.integer.value[1] = card_priv->capture_source[addr][1];
	spin_unlock_irq(&card_priv->mixer_lock);
	return 0;
}

static int snd_star_i2s_wm8759_capsrc_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_star_i2s_wm8759_priv *card_priv = snd_kcontrol_chip(kcontrol);
	int change, addr = kcontrol->private_value;
	int left, right;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	left = ucontrol->value.integer.value[0] & 1;
	right = ucontrol->value.integer.value[1] & 1;
	spin_lock_irq(&card_priv->mixer_lock);
	change = card_priv->capture_source[addr][0] != left &&
	         card_priv->capture_source[addr][1] != right;
	card_priv->capture_source[addr][0] = left;
	card_priv->capture_source[addr][1] = right;
	spin_unlock_irq(&card_priv->mixer_lock);
	return change;
}

static struct snd_kcontrol_new snd_star_i2s_wm8759_controls[] = {
STAR_I2S_WM8759_VOLUME("Master Volume", 0, MIXER_ADDR_MASTER),
STAR_I2S_WM8759_CAPSRC("Master Capture Switch", 0, MIXER_ADDR_MASTER),
STAR_I2S_WM8759_VOLUME("Synth Volume", 0, MIXER_ADDR_SYNTH),
STAR_I2S_WM8759_CAPSRC("Synth Capture Switch", 0, MIXER_ADDR_MASTER),
STAR_I2S_WM8759_VOLUME("Line Volume", 0, MIXER_ADDR_LINE),
STAR_I2S_WM8759_CAPSRC("Line Capture Switch", 0, MIXER_ADDR_MASTER),
STAR_I2S_WM8759_VOLUME("Mic Volume", 0, MIXER_ADDR_MIC),
STAR_I2S_WM8759_CAPSRC("Mic Capture Switch", 0, MIXER_ADDR_MASTER),
STAR_I2S_WM8759_VOLUME("CD Volume", 0, MIXER_ADDR_CD),
STAR_I2S_WM8759_CAPSRC("CD Capture Switch", 0, MIXER_ADDR_MASTER)
};

/* This function is no longer used in the initial function. */
static int __init snd_card_star_i2s_wm8759_new_mixer(struct snd_star_i2s_wm8759_priv *card_priv)
{
	struct snd_card *card = card_priv->card;
	unsigned int idx;
	int err;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	snd_assert(card_priv != NULL, return -EINVAL);
	spin_lock_init(&card_priv->mixer_lock);
	strcpy(card->mixername, "[Mixer id 4]");

	for (idx = 0; idx < ARRAY_SIZE(snd_star_i2s_wm8759_controls); idx++) {
		if ((err = snd_ctl_add(card, snd_ctl_new1(&snd_star_i2s_wm8759_controls[idx], card_priv))) < 0)
			return err;
	}
	return 0;
}
#endif

static int __init snd_star_i2s_wm8759_probe(struct platform_device *devptr)
{
	struct snd_card *card;
	struct snd_star_i2s_wm8759_priv *card_priv;
	int idx, err;
	int dev = devptr->id;
	DEBUG_PRINT(LVL_INFO,"%s devptr->dev=%p\n",__FUNCTION__,&devptr->dev);

	card = snd_card_new(snd_star_i2s_wm8759_index, snd_star_i2s_wm8759_id, THIS_MODULE,
			    sizeof(struct snd_star_i2s_wm8759_priv));
	if (card == NULL) return -ENOMEM;
	
	card_priv = card->private_data;
	card_priv->card = card;
//	card_priv->dev=&devptr->dev;
	card_priv->dev=NULL;
	for (idx = 0; idx < MAX_PCM_DEVICES && idx < pcm_devs; idx++) {
		if (pcm_substreams < 1) pcm_substreams = 1;
		if (pcm_substreams > MAX_PCM_SUBSTREAMS) pcm_substreams = MAX_PCM_SUBSTREAMS;
		if ((err = snd_card_star_i2s_wm8759_new_pcm(card_priv, idx, pcm_substreams)) < 0)
			goto __nodev;
	}
#if 0
	if ((err = snd_card_star_i2s_wm8759_new_mixer(card_priv)) < 0)
		goto __nodev;
#endif
	strcpy(card->driver, "[id 5]");
	strcpy(card->shortname, "[id 6]");
	sprintf(card->longname, "[id 7 %i]", dev + 1);

	snd_card_set_dev(card, &devptr->dev);

	if ((err = snd_card_register(card)) == 0) {
		platform_set_drvdata(devptr, card);
		return 0;
	}
      __nodev:
	snd_card_free(card);
	return err;
}

static int snd_star_i2s_wm8759_remove(struct platform_device *devptr)
{
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	snd_card_free(platform_get_drvdata(devptr));
	platform_set_drvdata(devptr, NULL);
	return 0;
}

#ifdef CONFIG_PM
static int snd_star_i2s_wm8759_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_card *card = platform_get_drvdata(pdev);
	struct snd_star_i2s_wm8759_priv *card_priv = card->private_data;
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	snd_power_change_state(card, SNDRV_CTL_POWER_D3hot);
	snd_pcm_suspend_all(card_priv->pcm);
	return 0;
}
	
static int snd_star_i2s_wm8759_resume(struct platform_device *pdev)
{
	struct snd_card *card = platform_get_drvdata(pdev);
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);

	snd_power_change_state(card, SNDRV_CTL_POWER_D0);
	return 0;
}
#endif

#define SND_STAR_I2S_WM8759_DRIVER	"[id 8]"

static struct platform_driver snd_star_i2s_wm8759_driver = {
	.probe		= snd_star_i2s_wm8759_probe,
	.remove		= snd_star_i2s_wm8759_remove,
#ifdef CONFIG_PM
	.suspend	= snd_star_i2s_wm8759_suspend,
	.resume		= snd_star_i2s_wm8759_resume,
#endif
	.driver		= {
		.name	= SND_STAR_I2S_WM8759_DRIVER
	},
};

static void __init_or_module snd_star_i2s_wm8759_unregister_all(void)
{
DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	platform_device_unregister(snd_devices);
	platform_driver_unregister(&snd_star_i2s_wm8759_driver);
}

static int __init alsa_card_star_i2s_wm8759_init(void)
{
	int err, ret;
	DEBUG_PRINT(LVL_INFO,"%s: \n",__FUNCTION__);
	PWRMGT_PLL250_CONTROL_REG=0x1;

	if ((err = platform_driver_register(&snd_star_i2s_wm8759_driver)) < 0)
		return err;

		snd_devices = platform_device_register_simple(SND_STAR_I2S_WM8759_DRIVER, /*i*/0, NULL, 0);
		if (IS_ERR(snd_devices)) {
			err = PTR_ERR(snd_devices);
			goto errout;
		}
		
	str8100_set_interrupt_trigger (INTC_I2S_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);
	if ((ret=request_irq(INTC_I2S_BIT_INDEX, str8100_i2s_irq_handler, 0, "i2s", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_I2S_BIT_INDEX,ret,-EBUSY);
		goto errout;
	}

	str8100_set_interrupt_trigger (INTC_GDMAC_TC_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_GDMAC_TC_BIT_INDEX, str8100_dma_tc_irq_handler, 0, "dma tc", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_GDMAC_TC_BIT_INDEX,ret,-EBUSY);
		goto errout;
	}
	str8100_set_interrupt_trigger (INTC_GDMAC_ERROR_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_GDMAC_ERROR_BIT_INDEX, str8100_dma_err_irq_handler, 0, "dma error", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_GDMAC_ERROR_BIT_INDEX,ret,-EBUSY);
		goto errout;
	}

	DEBUG_PRINT(LVL_INFO,"%s: exit\n",__FUNCTION__);
	return 0;

 errout:
	DEBUG_PRINT(LVL_INFO,"%s: errorout\n",__FUNCTION__);
	snd_star_i2s_wm8759_unregister_all();
	free_irq(INTC_I2S_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_TC_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_ERROR_BIT_INDEX, NULL);
	return err;
}

static void __exit alsa_card_star_i2s_wm8759_exit(void)
{
	DEBUG_PRINT(LVL_INFO,"%s\n",__FUNCTION__);
	snd_star_i2s_wm8759_unregister_all();
	free_irq(INTC_I2S_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_TC_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_ERROR_BIT_INDEX, NULL);
}

module_init(alsa_card_star_i2s_wm8759_init)
module_exit(alsa_card_star_i2s_wm8759_exit)
