#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <asm-arm/arch-str9100/star_gpio.h>
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test board leds Module");
MODULE_AUTHOR("Inter You");
static struct proc_dir_entry * proc_entry;

static char buffer[300];
static char opt[10];
static char led[10];

static int get_argv(char * p, unsigned long len)
{
    char *j;
    int i=0,m=0;
    unsigned long length;
    char tmp[300];

    memset(opt,0,10);
    memset(led,0,10);
    memset(tmp,0,300);

    while ((p[i] == ' ' ) && i<len)
        i++;

    length = len - i;
    strcpy(tmp,&p[i]);

    j = strstr(tmp," ");

    i = 0;

    if (j != NULL)
    {

        while ((tmp[i] != ' ') && i<length)
        {
            led[m] = tmp[i];
            i++,m++;
        }

        led[m]='\0';
        m=0;

        while ((tmp[i] == ' ' ) && i<length)
            i++;

        while ((tmp[i] != '\0') && (tmp[i] != ' ') && isalpha(tmp[i]) && i<=length)
        {
            opt[m] = tmp[i];
            m++,i++;
        }
        opt[m]='\0';
    }
    else
    {
        return 1;
    }

    return 0;

}

#define LEDCFG_TEST    13
#define LEDCFG_DMZ     14
#define REG_GPIO_DIR    0xFFF0B008
#define REG_GPIO_OUTPUT 0xFFF0B000

void ledcfg_led_init(unsigned long int gpio, unsigned char status)
{
    u32 data;
    data = *((u32 volatile *)(REG_GPIO_DIR));
    data |= (1<<gpio);
    *((u32 volatile *)(REG_GPIO_DIR)) = data;

    if (status)
    {
        data = *((u32 volatile *)(REG_GPIO_OUTPUT));
        data &= ~(1<<gpio);
        *((u32 volatile *)(REG_GPIO_OUTPUT)) = data;
    }
    else
    {
        data = *((u32 volatile *)(REG_GPIO_OUTPUT));
        data |= (1<<gpio);
        *((u32 volatile *)(REG_GPIO_OUTPUT)) = data;
    }
}

ssize_t ledconfig_write(struct file * filp, const char __user * buff, unsigned long len, void * data)
{
    int i;

    memset(buffer,0,sizeof(buffer));

    if (copy_from_user(buffer,buff,len))
        return -EFAULT;

    i = get_argv(buffer,len);
    printk("led : %s\topt : %s\n",led,opt);

    if (!i)
    {
        if (strnicmp(led,"test",sizeof("test"))==0)
        {
            if ((i=strnicmp(opt,"on",sizeof("on")))==0)
            {
                ledcfg_led_init(LEDCFG_TEST,1);
            }
            else if (strnicmp(opt,"off",sizeof("off"))==0)
            {
                ledcfg_led_init(LEDCFG_TEST,0);
            }
        }
        else if (strnicmp(led,"dmz",sizeof("dmz"))==0)
        {
            if (strnicmp(opt,"on",sizeof("on"))==0)
            {
                ledcfg_led_init(LEDCFG_DMZ,1);
            }
            else if (strnicmp(opt,"off",sizeof("off"))==0)
            {
                ledcfg_led_init(LEDCFG_DMZ,0);
            }
        }
    }

    return len;
}

int ledconfig_read(char * page, char **start, off_t off, int count, int *eof, void *data)
{
    int len;

    len = sprintf(page,"%s\n","test leds by command echo \"test/dmz on/off\">/proc/ledconfig");
    return len;
}

static int __init init_ledconfig_module(void)
{
    int ret = 0;

    proc_entry = create_proc_entry("ledconfig",0644,NULL);
    if (proc_entry == NULL)
    {
        printk("Error : Cann't create ledconfig proc file\n");
        ret = -ENOMEM;
    }

    proc_entry->read_proc = ledconfig_read;
    proc_entry->write_proc = ledconfig_write;
    proc_entry->owner = THIS_MODULE;
    printk(KERN_INFO "ledconfig : Module loaded.\n");

    return ret;
}
/* TF1 Change
 * This routine is used in main.c to glow test LED on bootup
 */
EXPORT_SYMBOL (ledcfg_led_init);

module_init(init_ledconfig_module);
