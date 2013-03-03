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
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/bcd.h>
#include <linux/list.h>
#include <linux/delay.h>

#define DRV_VERSION "0.0.1"


// must define slave addr to normal_i2c, nt7651_probe() will be called
// addr_data will in I2C_CLIENT_INSMOD defined, so don't define addr_data variable
static unsigned short normal_i2c[] = { 0x3a, I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD;
static int nt7651_attach(struct i2c_adapter *adapter);
static int nt7651_detach(struct i2c_client *client);
static int nt7651_probe(struct i2c_adapter *adapter, int address, int kind);
static struct i2c_driver nt7651_driver = {
	.driver = {
		.name	= "nt7651",
	},
	.attach_adapter = &nt7651_attach,
	.detach_client	= &nt7651_detach,
};
struct nt7651_data {
	struct i2c_client client;
	//struct list_head list;
	//unsigned int epoch;
};

static int nt7651_attach(struct i2c_adapter *adapter)
{
	dev_dbg(&adapter->dev, "%s\n", __FUNCTION__);

	return i2c_probe(adapter, &addr_data, nt7651_probe);
}



#define LCM_SD1602X_INSTRUCTION_REG  (0)
#define LCM_SD1602X_DATA_REG         (0x40)

struct i2c_client *client_;

struct i2c_client *get_client()
{
	return client_;
}

int i2c_NT7651_lcm(u8 reg, u8 cmd)
{
	u8 buf[3];
	buf[1] = cmd;
	buf[0] = reg;
        return i2c_master_send(get_client(), buf,2); 
}


int nt7651_lcm_goto_x(u8 x)
{
  //u32 command_byte = (x & 0x7F);
  //printf("x: %d\n", x);
  u8 command_byte = (0x1 << 7) | (x & 0x7F);

  return i2c_NT7651_lcm(LCM_SD1602X_INSTRUCTION_REG, command_byte);
}

int i2c_NT7651_lcm_putchar(u8 achar)
{
        //return I2c_Lcm_Sd1602x_Write_Data_Command(achar);
        const u8 char_offset='1'- 0xb1;

        return i2c_NT7651_lcm(LCM_SD1602X_DATA_REG, achar+char_offset);
}

int i2c_NT7651_lcm_print(const char *str)
{
  int i=0;

  for (i=0 ; str[i] ; ++i)
  {
        i2c_NT7651_lcm_putchar(str[i]);
  }
  return 0;
}


int i2c_NT7651_lcm_clear()
{
  u32 command_byte = 0x1;
  return i2c_NT7651_lcm(LCM_SD1602X_INSTRUCTION_REG, command_byte);
}




static int init_nt7651_lcm(struct i2c_client *client)
{
	int xfer;
	u8 buf[3];

	//  Clear display command
	buf[1] = 0x1;
	buf[0] = LCM_SD1602X_INSTRUCTION_REG;  

	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

#if 0
	buf[1] = (0x1 << 5) | (0x1 << 4) | (0x0 << 0); 
	buf[0] = LCM_SD1602X_INSTRUCTION_REG;  

	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);
	printk("xfer: %d\n", xfer);

	buf[1] = (0x1 << 5) | (0x1 << 4) | (0x0 << 0); 
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = (0x1 << 5) | (0x1 << 4) | (0x0 << 0); 
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);


	buf[1] = (0x1 << 5) | (0x1 << 4);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);


	buf[1] = (0x1 << 3);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = 0x1;
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);


	buf[1] = (0x1 << 2) | (0x1 << 1);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = (0x1 << 3) | (0x1 << 2) | (0x1 << 1);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = (0x1 << 5) | (0x1 << 4) | (0x1 << 0);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = (0x1 << 2);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = (0x1 << 4) | (0x1 << 1) | (0x1 << 0);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = 0x9b;
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);

	buf[1] = (0x1 << 5) | (0x1 << 4) | (0x1 << 0);
	mdelay(10);
	xfer = i2c_master_send(client, buf, 2);
#endif

	mdelay(10);
	//nt7651_lcm_goto_x(12);

	i2c_NT7651_lcm_print("in kernel");

	return 0;
}


static int nt7651_probe(struct i2c_adapter *adapter, int address, int kind)
{
	struct nt7651_data *data;
	struct i2c_client *client;
	int err = 0;

        data = kzalloc(sizeof(struct nt7651_data), GFP_KERNEL);
        if (!data) {
                err = -ENOMEM;
                goto exit;
        }


	printk("nt7651_probe\n");
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		printk("123\n");
		goto exit;
	}

	client = &data->client; // the line must, or will get kernel panic
	client->addr = address;
	client->driver = &nt7651_driver;
	client->adapter	= adapter;
	strlcpy(client->name, "nt7651", I2C_NAME_SIZE);
	i2c_set_clientdata(client, data);
	/* Inform the i2c layer */
	if ((err = i2c_attach_client(client))) {
		printk("qwe\n");
		goto exit_kfree;
	}
	client_ = client;
	init_nt7651_lcm(client);

exit_kfree:
	kfree(data);

exit:
	return err;
}

static int nt7651_detach(struct i2c_client *client)
{

       int err;
        struct x1205_data *data = i2c_get_clientdata(client);

        dev_dbg(&client->dev, "%s\n", __FUNCTION__);

        if ((err = i2c_detach_client(client)))
                return err;


        kfree(data);

	return 0;
}


static int __init nt7651_init(void)
{
	printk("in nt7651_init\n");
	return i2c_add_driver(&nt7651_driver);
}

static void __exit nt7651_exit(void)
{
	i2c_del_driver(&nt7651_driver);
}

MODULE_AUTHOR("descent");
MODULE_DESCRIPTION("nt7651 lcm driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

module_init(nt7651_init);
module_exit(nt7651_exit);
