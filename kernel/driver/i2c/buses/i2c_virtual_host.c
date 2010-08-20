#include <linux/slab.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/link.h>

#define I2C_VHOST_DEBUG

#ifdef I2C_VHOST_DEBUG
#define I2C_VHOST_PRINTK(format, args...) do { printk("[SYSFS_INPUT_KBD][%s]" format, __func__, ##args); } while (0)
#else
#define I2C_VHOST_PRINTK(format, args...) do { } while (0)
#endif

#define DEV_DBG(dev, format, args...)	I2C_VHOST_PRINTK(format, ##args)

#define I2C_VHOST_DRIVER_DESC	"i2c virtual host driver"


MODULE_AUTHOR("William Su");
MODULE_DESCRIPTION(I2C_VHOST_DRIVER_DESC);
MODULE_LICENSE("GPL");

LIST_HEAD(device_list_head);

struct device_register_data{
	u8 *data;
	size_t size;
	struct list_head list;
}

static int i2c_vhost_xfer_byte(struct i2c_adapter * adap, u16 addr, unsigned short flags,
	char read_write, u8 command, int size, union i2c_smbus_data * data)
{

	if (read_write == I2C_SMBUS_WRITE) {
		DEV_DBG(&adap->dev, "smbus byte - addr 0x%02x, "
				"wrote 0x%02x.\n",
				addr, command);
		
	} else {
		DEV_DBG(&adap->dev, "smbus byte - addr 0x%02x, "
				"read  0x%02x.\n",
				addr, data->byte);
	}
	return 0;

}

static int i2c_vhost_xfer_byte_data(struct i2c_adapter * adap, u16 addr, unsigned short flags,
	char read_write, u8 command, int size, union i2c_smbus_data * data)
{
	if (read_write == I2C_SMBUS_WRITE) {
		DEV_DBG(&adap->dev, "smbus byte data - addr 0x%02x, "
				"wrote 0x%02x at 0x%02x.\n",
				addr, data->byte, command);
	} else {
		DEV_DBG(&adap->dev, "smbus byte data - addr 0x%02x, "
				"read  0x%02x at 0x%02x.\n",
				addr, data->byte, command);
	}
	return 0;
}

static int i2c_vhost_xfer_word_data(struct i2c_adapter * adap, u16 addr, unsigned short flags,
	char read_write, u8 command, int size, union i2c_smbus_data * data)
{
	if (read_write == I2C_SMBUS_WRITE) {
		DEV_DBG(&adap->dev, "smbus word data - addr 0x%02x, "
				"wrote 0x%04x at 0x%02x.\n",
				addr, data->word, command);
	} else {
		DEV_DBG(&adap->dev, "smbus word data - addr 0x%02x, "
				"read  0x%04x at 0x%02x.\n",
				addr, data->word, command);
	}

	return 0;
}

static int i2c_vhost_xfer(struct i2c_adapter * adap, u16 addr, unsigned short flags,
	char read_write, u8 command, int size, union i2c_smbus_data * data)
{
	int ret = 0;

	DEV_DBG(&adap->dev, "smbus - addr 0x%02x\n", addr);
	switch (size) {

	case I2C_SMBUS_QUICK:
		DEV_DBG(&adap->dev, "smbus quick - addr 0x%02x\n", addr);
		ret = 0;
		break;

	case I2C_SMBUS_BYTE:
		ret = i2c_vhost_xfer_byte(adap, addr, flags, read_write, command, size, data);
		break;

	case I2C_SMBUS_BYTE_DATA:
		ret = i2c_vhost_xfer_byte_data(adap, addr, flags, read_write, command, size, data);
		break;

#if 0
	case I2C_SMBUS_WORD_DATA:
		ret = i2c_vhost_xfer_word_data(adap, addr, flags, read_write, command, size, data);
		break;
#endif

	default:
		DEV_DBG(&adap->dev, "Unsupported I2C/SMBus command\n");
		ret = -EOPNOTSUPP;
		break;
	} /* switch (size) */

	return ret;
}

static u32 i2c_vhost_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE |
		I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA;
}

static const struct i2c_algorithm i2c_vhost_algorithm = {
	.functionality	= i2c_vhost_func,
	.smbus_xfer	= i2c_vhost_xfer,
};

static struct i2c_adapter i2c_vhost_adapter = {
	.owner		= THIS_MODULE,
	.class		= I2C_CLASS_HWMON | I2C_CLASS_SPD,
	.algo		= &i2c_vhost_algorithm,
	.name		= "SMBus i2c_vhost driver",
};


struct i2c_client *client;
static int __init i2c_vhost_init(void)
{
	int ret = 0;
	struct i2c_board_info info = {
		I2C_BOARD_INFO("i2c_test_device", 0xAA ),

	};

	I2C_VHOST_PRINTK("Module init\n");

	ret = i2c_add_adapter(&i2c_vhost_adapter);

	client = i2c_new_device(&i2c_vhost_adapter, &info);
	printk("client address:[%u]\n", (unsigned int)client);
	printk("adapter address:[%u]\n", (unsigned int)&i2c_vhost_adapter);

	return ret;
}

static void __exit i2c_vhost_exit(void)
{
	i2c_del_adapter(&i2c_vhost_adapter);
}

module_init(i2c_vhost_init);
module_exit(i2c_vhost_exit);
