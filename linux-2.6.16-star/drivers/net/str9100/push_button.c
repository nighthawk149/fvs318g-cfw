/* TF1 Change
 * This driver is extensively hacked by me, So its no longer in its original
 * shape as provided by Sercomm. This is done in order to provide same
 * functionality of hard factory reset, as seen in other Netgear boxes - anp
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>
#include <linux/reboot.h>

/* polling every 0.3 sec
 * HZ = 100
 */
#define SCAN_RATE HZ/3
#define FACTORY_RESET_THRESHOLD 10 /* seconds */
#define CUSTOM_FACTORY_THRESHOLD 2 /* seconds */
#define FACTORY_RESET_FILE "/tmp/fReset"
#define CUSTOM_FACTORY_RESET_FILE "/tmp/fCustomReset"
#define REG_GPIO_INPUT 0xFFF0B004
#define TEST_LED    13
#define LED_ON      1
#define LED_OFF     0 

static struct timer_list push_button_timer, blink_led_timer;
static unsigned int counter = 0;
static int button_pressed = 0;
static unsigned int led_toggle_flag = LED_ON;

void ledcfg_led_init (unsigned long int, unsigned char);

MODULE_DESCRIPTION ("Factory Reset Driver");
MODULE_AUTHOR ("Inter You");

int check_pushbutton (void)
    {
    unsigned long int tmp;
    tmp = *((int volatile *)(REG_GPIO_INPUT));
    if (tmp & (1 << 15))
        return 0;
    else
        return 1;
    }

static void test_led_blink 
    (
    unsigned long count
    )
    {
    led_toggle_flag = !led_toggle_flag;
    ledcfg_led_init (TEST_LED, led_toggle_flag);
    mod_timer (&blink_led_timer, jiffies + HZ / 20);
    }

static void push_button_polling 
    (
    unsigned long count
    )
    {
    struct file * pFp;
    unsigned int interval = SCAN_RATE;

    if (check_pushbutton())
        {
        /* button is pressed, indicate it by glowing test LED */
        ledcfg_led_init (TEST_LED, LED_ON);
        button_pressed = 1;
        if (counter == FACTORY_RESET_THRESHOLD)
            {
            /* 
             * Userspace has to do factory reset. We will indicate user space by
             * creating FACTORY_RESET_FILE. Also blink the Test LED for user
             * indication.
             */
            add_timer (&blink_led_timer); 
            pFp = filp_open (FACTORY_RESET_FILE, O_CREAT | O_RDWR, 0);
            filp_close (pFp, 0);
            del_timer (&push_button_timer);
            return;
            }
        interval = HZ;
        counter++;
        }
    else
        {
        /* 
         * user have pressed and released the button before 
         * FACTORY_RESET_THRESHOLD 
         */
        if (counter >= CUSTOM_FACTORY_THRESHOLD && counter < FACTORY_RESET_THRESHOLD)
	    {
            /* 
             * Userspace has to do factory reset. We will indicate user space by
             * creating FACTORY_RESET_FILE. Also blink the Test LED for user
             * indication.
             */
            add_timer (&blink_led_timer); 
            pFp = filp_open (CUSTOM_FACTORY_RESET_FILE, O_CREAT | O_RDWR, 0);
            filp_close (pFp, 0);
            del_timer (&push_button_timer);
            return;

	    }
        else if (button_pressed)
             machine_restart (NULL);
        }

    mod_timer (&push_button_timer, jiffies + interval);
    }

static int __init push_module_init (void)
    {
    /* timer for factory reset button */
    init_timer (&push_button_timer);
    push_button_timer.data = 0;
    push_button_timer.expires = jiffies + SCAN_RATE;
    push_button_timer.function = push_button_polling;
    add_timer (&push_button_timer); 

    /* timer for test LED blinking */
    init_timer (&blink_led_timer);
    blink_led_timer.data = 0;
    blink_led_timer.expires = jiffies + HZ / 10;
    blink_led_timer.function = test_led_blink;

    printk (KERN_INFO"push_button: Module loaded\n");

    return 0;
    }

static void __exit push_module_deinit (void)
    {
    del_timer (&push_button_timer);
    }

module_init (push_module_init);
module_exit (push_module_deinit);
