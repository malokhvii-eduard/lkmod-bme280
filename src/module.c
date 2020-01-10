#include <linux/module.h>
#include <linux/i2c.h>

#include <module.h>

static int bme280_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	return 0;
}

static int bme280_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static const unsigned short bme280_i2c_addr[] = { I2C_CLIENT_END };
static const struct i2c_device_id bme280_i2c_id[] = { {} };
MODULE_DEVICE_TABLE(i2c, bme280_i2c_id);

static struct i2c_driver bme280_i2c_driver = {
	.driver = {
		.name	= "bme280",
	},
	.probe = bme280_i2c_probe,
	.remove = bme280_i2c_remove,
	.id_table = bme280_i2c_id,
	.address_list = bme280_i2c_addr,
};
module_i2c_driver(bme280_i2c_driver);

MODULE_AUTHOR("Eduard Malokhvii <malohvii.ee@gmail.com>");
MODULE_DESCRIPTION("Driver for Bosch Sensortec BME280 combined "
		   "temperature, pressure, humidity sensor");
MODULE_VERSION("1.0");
