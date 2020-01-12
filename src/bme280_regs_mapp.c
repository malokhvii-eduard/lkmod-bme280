#include <linux/cdev.h>
#include <linux/stat.h>
#include <linux/list.h>

#include <module.h>
#include <bme280.h>
#include <bme280_regs_mapp.h>

#ifdef DEBUG
#define ENABLE_CALIB_DATA_REGS_MAPP
#endif /* DEBUG */

#define CLASS_BME280 "bme280"

/***************************** Extern Variables *******************************/

extern struct bme280 *bme280_device;

extern struct list_head bme280_devices;
extern struct mutex bme280_devices_lock;

/***************************** Common Functions *******************************/

static inline ssize_t bme280_device_null_ptr_check(void)
{
	if (bme280_device == NULL) {
		pr_warn(THIS_MODULE_NAME
			": current device not specified, use /sys/%s/i2c"
			" to specify\n",
			CLASS_BME280);

		return -ENODEV;
	} else {
		return BME280_OK;
	}
}

/******************************* Sysfs Utils **********************************/

/** Redefine macros for sysfs, to do them better */
#undef CLASS_ATTR_RW
#undef CLASS_ATTR_RO
#undef CLASS_ATTR_WO

#define CLASS(name) struct class *class_##name = NULL
#define CLASS_ATTR(name, mode, show, store)                                    \
	struct class_attribute class_attr_##name =                             \
		__ATTR(name, mode, show, store)
#define CLASS_ATTR_RW(name, show, store)                                       \
	CLASS_ATTR(name, S_IRUGO | S_IWUSR, show, store)
#define CLASS_ATTR_RO(name, show) CLASS_ATTR(name, S_IRUGO, show, NULL)
#define CLASS_ATTR_WO(name, store) CLASS_ATTR(name, S_IWUSR, NULL, store)

/******************************* Sysfs Classes ********************************/

static CLASS(bme280);

/************** I2C info about current selected device (Sysfs) ****************/

static ssize_t class_attr_i2c_show(struct class *class,
				   struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	/**
	 * Protects from cases when bme280_device can be removed from 
	 * list of registed devices earlier than necessary, or when 
	 * operation performed concurrently
	 */
	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		ret = sprintf(buf, "none\n");
	} else {
		ret = sprintf(buf, "%s-%d 0x%x\n",
			      bme280_device->client->adapter->dev.of_node->name,
			      bme280_device->client->adapter->nr,
			      bme280_device->client->addr);
	}

	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_i2c_store(struct class *class,
				    struct class_attribute *attr,
				    const char *buf, size_t count)
{
	ssize_t ret;

	struct list_head *iter = NULL;
	struct bme280 *device = NULL;

	u8 adapter_nr;
	u8 addr;
	u8 contains;

	mutex_lock(&bme280_devices_lock);

	ret = sscanf(buf, "%hhu 0x%hhx\n", &adapter_nr, &addr);
	if (ret != 2) {
		pr_err(THIS_MODULE_NAME
		       ": invalid argument, try to write i2c adapter number "
		       " in decimal and device address in hex\n");

		ret = -EINVAL;
		goto err;
	}

	list_for_each (iter, &bme280_devices) {
		device = list_entry(iter, struct bme280, registered);
		if (adapter_nr == device->client->adapter->nr &&
		    addr == device->client->addr) {
			contains = 1;
			break;
		}
	}

	if (contains) {
		bme280_device = device;
		ret = count;
	} else {
		pr_warn(THIS_MODULE_NAME
			": device with specified i2c adapter number and"
			" address not found in list of registered devices\n");

		ret = -ENODEV;
	}

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static CLASS_ATTR_RW(i2c, &class_attr_i2c_show, &class_attr_i2c_store);

/********************* Device calibration data (Sysfs) ************************/

#ifdef ENABLE_CALIB_DATA_REGS_MAPP

static ssize_t class_attr_dig_T1_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_T1);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_T2_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_T2);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_T3_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_T3);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P1_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P1);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P2_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P2);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P3_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P3);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P4_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P4);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P5_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P5);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P6_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P6);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P7_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P7);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P8_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P8);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_P9_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_P9);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_H1_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_H1);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_H2_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_H2);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_H3_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_H3);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_H4_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_H4);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_H5_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_H5);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_dig_H6_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "%d\n", bme280_device->calib_data.dig_H6);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static CLASS_ATTR_RO(dig_T1, &class_attr_dig_T1_show);
static CLASS_ATTR_RO(dig_T2, &class_attr_dig_T2_show);
static CLASS_ATTR_RO(dig_T3, &class_attr_dig_T3_show);
static CLASS_ATTR_RO(dig_P1, &class_attr_dig_P1_show);
static CLASS_ATTR_RO(dig_P2, &class_attr_dig_P2_show);
static CLASS_ATTR_RO(dig_P3, &class_attr_dig_P3_show);
static CLASS_ATTR_RO(dig_P4, &class_attr_dig_P4_show);
static CLASS_ATTR_RO(dig_P5, &class_attr_dig_P5_show);
static CLASS_ATTR_RO(dig_P6, &class_attr_dig_P6_show);
static CLASS_ATTR_RO(dig_P7, &class_attr_dig_P7_show);
static CLASS_ATTR_RO(dig_P8, &class_attr_dig_P8_show);
static CLASS_ATTR_RO(dig_P9, &class_attr_dig_P9_show);
static CLASS_ATTR_RO(dig_H1, &class_attr_dig_H1_show);
static CLASS_ATTR_RO(dig_H2, &class_attr_dig_H2_show);
static CLASS_ATTR_RO(dig_H3, &class_attr_dig_H3_show);
static CLASS_ATTR_RO(dig_H4, &class_attr_dig_H4_show);
static CLASS_ATTR_RO(dig_H5, &class_attr_dig_H5_show);
static CLASS_ATTR_RO(dig_H6, &class_attr_dig_H6_show);

#endif /* ENABLE_CALIB_DATA_REGS_MAPP */

/************************ Device identifier (Sysfs) ***************************/

static ssize_t class_attr_chip_id_show(struct class *class,
				       struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sprintf(buf, "0x%x\n", bme280_device->chip_id);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static CLASS_ATTR_RO(chip_id, &class_attr_chip_id_show);

/************************** Device commands (Sysfs) ***************************/

static ssize_t class_attr_reset_store(struct class *class,
				      struct class_attribute *attr,
				      const char *buf, size_t count)
{
	ssize_t ret;

	u8 command;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sscanf(buf, "0x%hhx\n", &command);
	if (ret != 1) {
		pr_err(THIS_MODULE_NAME
		       ": invalid argument, try to write soft reset command"
		       " in hex\n");

		ret = -EINVAL;
		goto err;
	}

	if (command != BME280_SOFT_RESET_COMMAND) {
		pr_err(THIS_MODULE_NAME
		       ": wrong soft reset command, try to write 0x%x\n",
		       BME280_SOFT_RESET_COMMAND);

		ret = -EINVAL;
		goto err;
	}

	bme280_soft_reset(bme280_device);
	ret = count;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static CLASS_ATTR_WO(reset, &class_attr_reset_store);

/************************* Device settings (Sysfs) ****************************/

static ssize_t class_attr_mode_show(struct class *class,
				    struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	u8 sensor_mode;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_mode(bme280_device, &sensor_mode);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get power mode from sensor,"
		       " try again later\n");

		goto err;
	}

	ret = sprintf(buf, "0x%x\n", sensor_mode);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_mode_store(struct class *class,
				     struct class_attribute *attr,
				     const char *buf, size_t count)
{
	ssize_t ret;

	u8 sensor_mode;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sscanf(buf, "0x%hhx\n", &sensor_mode);
	if (ret != 1) {
		pr_err(THIS_MODULE_NAME ": invalid argument, try to write"
					" power mode in hex\n");

		ret = -EINVAL;
		goto err;
	}

	switch (sensor_mode) {
	case BME280_SLEEP_MODE:
	case BME280_FORCED_MODE:
	case BME280_NORMAL_MODE:
		ret = bme280_set_sensor_mode(bme280_device, sensor_mode);
		if (ret != BME280_OK) {
			pr_err(THIS_MODULE_NAME
			       ": failed to set power mode to sensor,"
			       " try again later\n");

			goto err;
		}

		break;
	default:
		pr_err(THIS_MODULE_NAME
		       ": wrong power mode,"
		       " acceptable values (0x%x, 0x%x, 0x%x)\n",
		       BME280_SLEEP_MODE, BME280_FORCED_MODE,
		       BME280_NORMAL_MODE);

		ret = -EINVAL;
		goto err;
	}

	ret = count;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_osrs_p_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_settings(bme280_device);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get pressure oversampling from sensor,"
		       " try again later\n");

		goto err;
	}

	ret = sprintf(buf, "0x%x\n", bme280_device->settings.osrs_p);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_osrs_p_store(struct class *class,
				       struct class_attribute *attr,
				       const char *buf, size_t count)
{
	ssize_t ret;

	u8 osrs_p;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sscanf(buf, "0x%hhx\n", &osrs_p);
	if (ret != 1) {
		pr_err(THIS_MODULE_NAME ": invalid argument, try to write"
					" pressure oversampling in hex\n");

		ret = -EINVAL;
		goto err;
	}

	switch (osrs_p) {
	case BME280_NO_OVERSAMPLING:
	case BME280_OVERSAMPLING_1X:
	case BME280_OVERSAMPLING_2X:
	case BME280_OVERSAMPLING_4X:
	case BME280_OVERSAMPLING_8X:
	case BME280_OVERSAMPLING_16X:
		bme280_device->settings.osrs_p = osrs_p;

		ret = bme280_set_sensor_settings(bme280_device,
						 BME280_OSRS_PRESS_SEL);
		if (ret != BME280_OK) {
			pr_err(THIS_MODULE_NAME
			       ": failed to set pressure oversampling to"
			       " sensor, try again later\n");

			goto err;
		}

		break;
	default:
		pr_err(THIS_MODULE_NAME ": wrong pressure oversampling,"
					" acceptable values (0x%x, 0x%x, 0x%x,"
					" 0x%x, 0x%x, 0x%x)\n",
		       BME280_NO_OVERSAMPLING, BME280_OVERSAMPLING_1X,
		       BME280_OVERSAMPLING_2X, BME280_OVERSAMPLING_4X,
		       BME280_OVERSAMPLING_8X, BME280_OVERSAMPLING_16X);

		ret = -EINVAL;
		goto err;
	}

	ret = count;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_osrs_t_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_settings(bme280_device);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get temperature oversampling from sensor,"
		       " try again later\n");

		goto err;
	}

	ret = sprintf(buf, "0x%x\n", bme280_device->settings.osrs_t);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_osrs_t_store(struct class *class,
				       struct class_attribute *attr,
				       const char *buf, size_t count)
{
	ssize_t ret;

	u8 osrs_t;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sscanf(buf, "0x%hhx\n", &osrs_t);
	if (ret != 1) {
		pr_err(THIS_MODULE_NAME ": invalid argument, try to write"
					" temperature oversampling in hex\n");

		ret = -EINVAL;
		goto err;
	}

	switch (osrs_t) {
	case BME280_NO_OVERSAMPLING:
	case BME280_OVERSAMPLING_1X:
	case BME280_OVERSAMPLING_2X:
	case BME280_OVERSAMPLING_4X:
	case BME280_OVERSAMPLING_8X:
	case BME280_OVERSAMPLING_16X:
		bme280_device->settings.osrs_t = osrs_t;

		ret = bme280_set_sensor_settings(bme280_device,
						 BME280_OSRS_TEMP_SEL);
		if (ret != BME280_OK) {
			pr_err(THIS_MODULE_NAME
			       ": failed to set temperature oversampling to"
			       " sensor, try again later\n");

			goto err;
		}

		break;
	default:
		pr_err(THIS_MODULE_NAME ": wrong temperature oversampling,"
					" acceptable values (0x%x, 0x%x, 0x%x,"
					" 0x%x, 0x%x, 0x%x)\n",
		       BME280_NO_OVERSAMPLING, BME280_OVERSAMPLING_1X,
		       BME280_OVERSAMPLING_2X, BME280_OVERSAMPLING_4X,
		       BME280_OVERSAMPLING_8X, BME280_OVERSAMPLING_16X);

		ret = -EINVAL;
		goto err;
	}

	ret = count;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_osrs_h_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_settings(bme280_device);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get humidity oversampling from sensor,"
		       " try again later\n");

		goto err;
	}

	ret = sprintf(buf, "0x%x\n", bme280_device->settings.osrs_h);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_osrs_h_store(struct class *class,
				       struct class_attribute *attr,
				       const char *buf, size_t count)
{
	ssize_t ret;

	u8 osrs_h;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sscanf(buf, "0x%hhx\n", &osrs_h);
	if (ret != 1) {
		pr_err(THIS_MODULE_NAME ": invalid argument, try to write "
					" humidity oversampling in hex\n");

		ret = -EINVAL;
		goto err;
	}

	switch (osrs_h) {
	case BME280_NO_OVERSAMPLING:
	case BME280_OVERSAMPLING_1X:
	case BME280_OVERSAMPLING_2X:
	case BME280_OVERSAMPLING_4X:
	case BME280_OVERSAMPLING_8X:
	case BME280_OVERSAMPLING_16X:
		bme280_device->settings.osrs_h = osrs_h;

		ret = bme280_set_sensor_settings(bme280_device,
						 BME280_OSRS_HUM_SEL);
		if (ret != BME280_OK) {
			pr_err(THIS_MODULE_NAME
			       ": failed to set humidity oversampling to"
			       " sensor, try again later\n");

			goto err;
		}

		break;
	default:
		pr_err(THIS_MODULE_NAME ": wrong humidity oversampling,"
					" acceptable values (0x%x, 0x%x, 0x%x,"
					" 0x%x, 0x%x, 0x%x)\n",
		       BME280_NO_OVERSAMPLING, BME280_OVERSAMPLING_1X,
		       BME280_OVERSAMPLING_2X, BME280_OVERSAMPLING_4X,
		       BME280_OVERSAMPLING_8X, BME280_OVERSAMPLING_16X);

		ret = -EINVAL;
		goto err;
	}

	ret = count;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_filter_show(struct class *class,
				      struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_settings(bme280_device);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get filter coefficient from sensor,"
		       " try again later\n");

		goto err;
	}

	ret = sprintf(buf, "0x%x\n", bme280_device->settings.filter);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_filter_store(struct class *class,
				       struct class_attribute *attr,
				       const char *buf, size_t count)
{
	ssize_t ret;

	u8 filter;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sscanf(buf, "0x%hhx\n", &filter);
	if (ret != 1) {
		pr_err(THIS_MODULE_NAME ": invalid argument, try to write "
					" filter coefficient in hex\n");

		ret = -EINVAL;
		goto err;
	}

	switch (filter) {
	case BME280_FILTER_COEFF_OFF:
	case BME280_FILTER_COEFF_2:
	case BME280_FILTER_COEFF_4:
	case BME280_FILTER_COEFF_8:
	case BME280_FILTER_COEFF_16:
		bme280_device->settings.filter = filter;

		ret = bme280_set_sensor_settings(bme280_device,
						 BME280_FILTER_SEL);
		if (ret != BME280_OK) {
			pr_err(THIS_MODULE_NAME
			       ": failed to set filter coefficient to"
			       " sensor, try again later\n");

			goto err;
		}

		break;
	default:
		pr_err(THIS_MODULE_NAME ": wrong filter coefficient,"
					" acceptable values (0x%x, 0x%x, 0x%x,"
					" 0x%x, 0x%x)\n",
		       BME280_FILTER_COEFF_OFF, BME280_FILTER_COEFF_2,
		       BME280_FILTER_COEFF_4, BME280_FILTER_COEFF_8,
		       BME280_FILTER_COEFF_16);

		ret = -EINVAL;
		goto err;
	}

	ret = count;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_standby_time_show(struct class *class,
					    struct class_attribute *attr,
					    char *buf)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_settings(bme280_device);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get standby time from sensor,"
		       " try again later\n");

		goto err;
	}

	ret = sprintf(buf, "0x%x\n", bme280_device->settings.standby_time);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_standby_time_store(struct class *class,
					     struct class_attribute *attr,
					     const char *buf, size_t count)
{
	ssize_t ret;

	u8 standby_time;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = sscanf(buf, "0x%hhx\n", &standby_time);
	if (ret != 1) {
		pr_err(THIS_MODULE_NAME ": invalid argument, try to write"
					" standby time in hex\n");

		ret = -EINVAL;
		goto err;
	}

	switch (standby_time) {
	case BME280_STANDBY_TIME_0_5_MS:
	case BME280_STANDBY_TIME_62_5_MS:
	case BME280_STANDBY_TIME_125_MS:
	case BME280_STANDBY_TIME_250_MS:
	case BME280_STANDBY_TIME_500_MS:
	case BME280_STANDBY_TIME_1000_MS:
	case BME280_STANDBY_TIME_10_MS:
	case BME280_STANDBY_TIME_20_MS:
		bme280_device->settings.standby_time = standby_time;

		ret = bme280_set_sensor_settings(bme280_device,
						 BME280_STANDBY_TIME_SEL);
		if (ret != BME280_OK) {
			pr_err(THIS_MODULE_NAME
			       ": failed to set standby time to"
			       " sensor, try again later\n");

			goto err;
		}

		break;
	default:
		pr_err(THIS_MODULE_NAME ": wrong standby time,"
					" acceptable values (0x%x, 0x%x, 0x%x,"
					" 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
		       BME280_STANDBY_TIME_0_5_MS, BME280_STANDBY_TIME_62_5_MS,
		       BME280_STANDBY_TIME_125_MS, BME280_STANDBY_TIME_250_MS,
		       BME280_STANDBY_TIME_500_MS, BME280_STANDBY_TIME_1000_MS,
		       BME280_STANDBY_TIME_10_MS, BME280_STANDBY_TIME_20_MS);

		ret = -EINVAL;
		goto err;
	}

	ret = count;

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static CLASS_ATTR_RW(mode, &class_attr_mode_show, &class_attr_mode_store);
static CLASS_ATTR_RW(osrs_p, &class_attr_osrs_p_show, &class_attr_osrs_p_store);
static CLASS_ATTR_RW(osrs_t, &class_attr_osrs_t_show, &class_attr_osrs_t_store);
static CLASS_ATTR_RW(osrs_h, &class_attr_osrs_h_show, &class_attr_osrs_h_store);
static CLASS_ATTR_RW(filter, &class_attr_filter_show, &class_attr_filter_store);
static CLASS_ATTR_RW(standby_time, &class_attr_standby_time_show,
		     &class_attr_standby_time_store);

/********************* Device compensated data (Sysfs) ************************/

static ssize_t class_attr_pressure_show(struct class *class,
					struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	struct bme280_data comp_data;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	/** Use forced mode to put device to sleep after measuring */
	ret = bme280_get_sensor_data_forced(bme280_device, BME280_PRESS,
					    &comp_data);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get pressure from sesnsor, try again"
		       " later\n");

		ret = -EAGAIN;
		goto err;
	}

	ret = sprintf(buf, "%d\n", comp_data.pressure);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_temperature_show(struct class *class,
					   struct class_attribute *attr,
					   char *buf)
{
	ssize_t ret;

	struct bme280_data comp_data;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_data_forced(bme280_device, BME280_TEMP,
					    &comp_data);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get temperature from sesnsor, try again"
		       " later\n");

		ret = -EAGAIN;
		goto err;
	}

	ret = sprintf(buf, "%d\n", comp_data.temperature);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t class_attr_humidity_show(struct class *class,
					struct class_attribute *attr, char *buf)
{
	ssize_t ret;

	struct bme280_data comp_data;

	mutex_lock(&bme280_devices_lock);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_data_forced(bme280_device, BME280_HUM,
					    &comp_data);
	if (ret != BME280_OK) {
		pr_err(THIS_MODULE_NAME
		       ": failed to get humidity from sesnsor, try again"
		       " later\n");

		ret = -EAGAIN;
		goto err;
	}

	ret = sprintf(buf, "%d\n", comp_data.humidity);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static CLASS_ATTR_RO(pressure, &class_attr_pressure_show);
static CLASS_ATTR_RO(temperature, &class_attr_temperature_show);
static CLASS_ATTR_RO(humidity, &class_attr_humidity_show);

/***************************** Public Functions *******************************/

ssize_t bme280_create_regs_mapp(void)
{
	ssize_t ret;

	class_bme280 = class_create(THIS_MODULE, CLASS_BME280);
	if (class_bme280 == NULL) {
		pr_err(THIS_MODULE_NAME
		       ": failed to create class '%s' in /sys\n",
		       CLASS_BME280);

		ret = -ENOENT;
		goto err;
	}

	ret = class_create_file(class_bme280, &class_attr_i2c);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'i2c' in /sys\n");

		ret = -ENOENT;
		goto remove_class_bme280;
	}

#ifdef ENABLE_CALIB_DATA_REGS_MAPP

	ret = class_create_file(class_bme280, &class_attr_dig_T1);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_T1' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_i2c;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_T2);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_T2' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_T1;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_T3);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_T3' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_T2;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P1);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P1' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_T3;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P2);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P2' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P1;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P3);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P3' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P2;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P4);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P4' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P3;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P5);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P5' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P4;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P6);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P6' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P5;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P7);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P7' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P6;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P8);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P8' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P7;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_P9);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_P9' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P8;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_H1);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_H1' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_P9;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_H2);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_H2' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_H1;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_H3);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_H3' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_H2;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_H4);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_H4' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_H3;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_H5);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_H5' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_H4;
	}

	ret = class_create_file(class_bme280, &class_attr_dig_H6);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'dig_H6' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_dig_H5;
	}

#endif /* ENABLE_CALIB_DATA_REGS_MAPP */

	ret = class_create_file(class_bme280, &class_attr_chip_id);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'chip_id' in /sys\n");

		ret = -ENOENT;
#ifdef ENABLE_CALIB_DATA_REGS_MAPP
		goto remove_class_attr_dig_H6;
#else
		goto remove_class_attr_i2c;
#endif /* ENABLE_CALIB_DATA_REGS_MAPP */
	}

	ret = class_create_file(class_bme280, &class_attr_reset);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'reset' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_chip_id;
	}

	ret = class_create_file(class_bme280, &class_attr_mode);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'mode' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_reset;
	}

	ret = class_create_file(class_bme280, &class_attr_osrs_p);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'osrs_p' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_mode;
	}

	ret = class_create_file(class_bme280, &class_attr_osrs_t);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'osrs_t' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_osrs_p;
	}

	ret = class_create_file(class_bme280, &class_attr_osrs_h);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'osrs_h' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_osrs_t;
	}

	ret = class_create_file(class_bme280, &class_attr_filter);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'filter' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_osrs_h;
	}

	ret = class_create_file(class_bme280, &class_attr_standby_time);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'standby_time' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_filter;
	}

	ret = class_create_file(class_bme280, &class_attr_pressure);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'pressure' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_standby_time;
	}

	ret = class_create_file(class_bme280, &class_attr_temperature);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'temperature' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_pressure;
	}

	ret = class_create_file(class_bme280, &class_attr_humidity);
	if (ret) {
		pr_err(THIS_MODULE_NAME ": failed to create class attribute "
					" 'humidity' in /sys\n");

		ret = -ENOENT;
		goto remove_class_attr_temperature;
	}

	return 0;

remove_class_attr_temperature:
	class_remove_file(class_bme280, &class_attr_temperature);
remove_class_attr_pressure:
	class_remove_file(class_bme280, &class_attr_pressure);
remove_class_attr_standby_time:
	class_remove_file(class_bme280, &class_attr_standby_time);
remove_class_attr_filter:
	class_remove_file(class_bme280, &class_attr_filter);
remove_class_attr_osrs_h:
	class_remove_file(class_bme280, &class_attr_osrs_h);
remove_class_attr_osrs_t:
	class_remove_file(class_bme280, &class_attr_osrs_t);
remove_class_attr_osrs_p:
	class_remove_file(class_bme280, &class_attr_osrs_p);
remove_class_attr_mode:
	class_remove_file(class_bme280, &class_attr_mode);
remove_class_attr_reset:
	class_remove_file(class_bme280, &class_attr_reset);
remove_class_attr_chip_id:
	class_remove_file(class_bme280, &class_attr_chip_id);

#ifdef ENABLE_CALIB_DATA_REGS_MAPP

remove_class_attr_dig_H6:
	class_remove_file(class_bme280, &class_attr_dig_H6);
remove_class_attr_dig_H5:
	class_remove_file(class_bme280, &class_attr_dig_H5);
remove_class_attr_dig_H4:
	class_remove_file(class_bme280, &class_attr_dig_H4);
remove_class_attr_dig_H3:
	class_remove_file(class_bme280, &class_attr_dig_H3);
remove_class_attr_dig_H2:
	class_remove_file(class_bme280, &class_attr_dig_H2);
remove_class_attr_dig_H1:
	class_remove_file(class_bme280, &class_attr_dig_H1);
remove_class_attr_dig_P9:
	class_remove_file(class_bme280, &class_attr_dig_P9);
remove_class_attr_dig_P8:
	class_remove_file(class_bme280, &class_attr_dig_P8);
remove_class_attr_dig_P7:
	class_remove_file(class_bme280, &class_attr_dig_P7);
remove_class_attr_dig_P6:
	class_remove_file(class_bme280, &class_attr_dig_P6);
remove_class_attr_dig_P5:
	class_remove_file(class_bme280, &class_attr_dig_P5);
remove_class_attr_dig_P4:
	class_remove_file(class_bme280, &class_attr_dig_P4);
remove_class_attr_dig_P3:
	class_remove_file(class_bme280, &class_attr_dig_P3);
remove_class_attr_dig_P2:
	class_remove_file(class_bme280, &class_attr_dig_P2);
remove_class_attr_dig_P1:
	class_remove_file(class_bme280, &class_attr_dig_P1);
remove_class_attr_dig_T3:
	class_remove_file(class_bme280, &class_attr_dig_T3);
remove_class_attr_dig_T2:
	class_remove_file(class_bme280, &class_attr_dig_T2);
remove_class_attr_dig_T1:
	class_remove_file(class_bme280, &class_attr_dig_T1);

#endif /* ENABLE_CALIB_DATA_REGS_MAPP */

remove_class_attr_i2c:
	class_remove_file(class_bme280, &class_attr_i2c);
remove_class_bme280:
	class_destroy(class_bme280);
err:
	return ret;
}

void bme280_remove_regs_mapp(void)
{
	class_remove_file(class_bme280, &class_attr_humidity);
	class_remove_file(class_bme280, &class_attr_temperature);
	class_remove_file(class_bme280, &class_attr_pressure);

	class_remove_file(class_bme280, &class_attr_standby_time);
	class_remove_file(class_bme280, &class_attr_filter);
	class_remove_file(class_bme280, &class_attr_osrs_h);
	class_remove_file(class_bme280, &class_attr_osrs_t);
	class_remove_file(class_bme280, &class_attr_osrs_p);
	class_remove_file(class_bme280, &class_attr_mode);

	class_remove_file(class_bme280, &class_attr_reset);

	class_remove_file(class_bme280, &class_attr_chip_id);

#ifdef ENABLE_CALIB_DATA_REGS_MAPP

	class_remove_file(class_bme280, &class_attr_dig_H6);
	class_remove_file(class_bme280, &class_attr_dig_H5);
	class_remove_file(class_bme280, &class_attr_dig_H4);
	class_remove_file(class_bme280, &class_attr_dig_H3);
	class_remove_file(class_bme280, &class_attr_dig_H2);
	class_remove_file(class_bme280, &class_attr_dig_H1);
	class_remove_file(class_bme280, &class_attr_dig_P9);
	class_remove_file(class_bme280, &class_attr_dig_P8);
	class_remove_file(class_bme280, &class_attr_dig_P7);
	class_remove_file(class_bme280, &class_attr_dig_P6);
	class_remove_file(class_bme280, &class_attr_dig_P5);
	class_remove_file(class_bme280, &class_attr_dig_P4);
	class_remove_file(class_bme280, &class_attr_dig_P3);
	class_remove_file(class_bme280, &class_attr_dig_P2);
	class_remove_file(class_bme280, &class_attr_dig_P1);
	class_remove_file(class_bme280, &class_attr_dig_T3);
	class_remove_file(class_bme280, &class_attr_dig_T2);
	class_remove_file(class_bme280, &class_attr_dig_T1);

#endif /* ENABLE_CALIB_DATA_REGS_MAPP */

	class_remove_file(class_bme280, &class_attr_i2c);

	class_destroy(class_bme280);
}
