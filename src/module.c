#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/list.h>

#include <module.h>
#include <bme280.h>
#include <bme280_regs_mapp.h>
#include <bme280_info_mapp.h>

/** Pointer to last selected device, all operations perfoms with this device */
struct bme280 *bme280_device = NULL;

/** Linked list to hold registered devices */
LIST_HEAD(bme280_devices);

/** Used for guaranteed sole access to registered devices for this driver */
struct mutex bme280_devices_lock;

static ssize_t bme280_i2c_register_device(struct i2c_client *client)
{
	ssize_t ret;

	struct bme280 *device = kzalloc(sizeof(*device), GFP_KERNEL);
	if (device == NULL) {
		pr_err(THIS_MODULE_NAME
		       ": failed to allocate memory for device at "
		       " %s-%d 0x%x\n",
		       client->adapter->dev.of_node->name, client->adapter->nr,
		       client->addr);

		ret = -EFAULT;
		goto err;
	}

	ret = bme280_init(device, client);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to initialize device at %s-%d 0x%x\n",
		       client->adapter->dev.of_node->name, client->adapter->nr,
		       client->addr);

		goto cleanup_device;
	}

	/** Default settings for new device */
	device->settings.osrs_p = BME280_INDOOR_PRESS_OVERSAMPLING;
	device->settings.osrs_t = BME280_INDOOR_TEMP_OVERSAMPLING;
	device->settings.osrs_h = BME280_INDOOR_HUM_OVERSAMPLING;
	device->settings.filter = BME280_INDOOR_FILTER_COEFF;
	device->settings.standby_time = BME280_INDOOR_STANDBY_TIME;

	ret = bme280_set_sensor_settings(device, BME280_ALL_SETTINGS_SEL);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to set device settings at %s-%d 0x%x\n",
		       client->adapter->dev.of_node->name, client->adapter->nr,
		       client->addr);

		goto cleanup_device;
	}

	mutex_lock(&bme280_devices_lock);

	if (list_empty(&bme280_devices)) {
		ret = bme280_create_regs_mapp();
		if (ret) {
			goto unlock_devices;
		}

		ret = bme280_create_info_mapp();
		if (ret) {
			goto unlock_devices;
		}
	}

	list_add(&device->registered, &bme280_devices);

	if (list_is_singular(&bme280_devices)) {
		bme280_device = device;
	}

	mutex_unlock(&bme280_devices_lock);

	return 0;

unlock_devices:
	mutex_unlock(&bme280_devices_lock);
cleanup_device:
	kfree(device);
err:
	return ret;
}

static ssize_t bme280_i2c_unregister_device(struct i2c_client *client)
{
	ssize_t ret;

	struct list_head *iter = NULL;
	struct bme280 *device = NULL;

	u8 contains;

	mutex_lock(&bme280_devices_lock);

	list_for_each (iter, &bme280_devices) {
		device = list_entry(iter, struct bme280, registered);
		if (device->client == client) {
			contains = 1;
			break;
		}
	}

	if (contains) {
		if (device == bme280_device) {
			bme280_device = NULL;
		}

		list_del(iter);
		kfree(device);
	} else {
		pr_err(THIS_MODULE_NAME
		       ": couldn't found device for deinitialization,"
		       " illegal device at %s-%d 0x%x\n",
		       client->adapter->dev.of_node->name, client->adapter->nr,
		       client->addr);

		ret = -ENODEV;
		goto err;
	}

	if (!list_empty(&bme280_devices)) {
		bme280_device = list_first_entry(&bme280_devices, struct bme280,
						 registered);
	} else {
		bme280_remove_regs_mapp();
		bme280_remove_info_mapp();
	}

	ret = 0;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static int bme280_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	int ret;

	switch (id->driver_data) {
	case BME280_CHIP_ID: /** Verify that I2C client is bme280 */
		ret = bme280_i2c_register_device(client);
		if (ret) {
			pr_err(THIS_MODULE_NAME
			       ": failed to register device at %s-%d 0x%x\n",
			       client->adapter->dev.of_node->name,
			       client->adapter->nr, client->addr);

			goto err;
		}

		pr_info(THIS_MODULE_NAME ": register device at %s-%d 0x%x\n",
			client->adapter->dev.of_node->name, client->adapter->nr,
			client->addr);

		break;

	default:
		ret = -ENODEV;
		goto err;
	}

	return 0;

err:
	return ret;
}

static int bme280_i2c_remove(struct i2c_client *client)
{
	int ret;

	ret = bme280_i2c_unregister_device(client);
	if (ret) {
		pr_err(THIS_MODULE_NAME
		       ": failed to unregister device at %s-%d 0x%x\n",
		       client->adapter->dev.of_node->name, client->adapter->nr,
		       client->addr);
	} else {
		pr_info(THIS_MODULE_NAME ": unregister device at %s-%d 0x%x\n",
			client->adapter->dev.of_node->name, client->adapter->nr,
			client->addr);
	}

	return ret;
}

static const unsigned short bme280_i2c_addr[] = { BME280_I2C_ADDR_PRIM,
						  BME280_I2C_ADDR_SEC,
						  I2C_CLIENT_END };
static const struct i2c_device_id bme280_i2c_id[] = { { "bme280",
							BME280_CHIP_ID },
						      {} };
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
