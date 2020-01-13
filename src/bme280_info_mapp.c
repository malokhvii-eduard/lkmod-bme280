#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <module.h>
#include <bme280.h>
#include <bme280_info_mapp.h>

#ifdef DEBUG
#define ENABLE_CALIB_DATA_INFO_MAPP
#endif /* DEBUG */

/***************************** Extern Variables *******************************/

extern struct bme280 *bme280_device;

extern struct list_head bme280_devices;
extern struct mutex bme280_devices_lock;

/***************************** Common Functions *******************************/

static inline ssize_t bme280_device_null_ptr_check(void)
{
	if (bme280_device == NULL) {
		pr_warn(THIS_MODULE_NAME
			": current device not specified, use /sys/bme280/i2c"
			" to specify\n");

		return -ENODEV;
	} else {
		return BME280_OK;
	}
}

static inline size_t proc_read(char **buf, size_t *buf_len, char __user *ubuf,
			       size_t count, loff_t *off)
{
	size_t left;

	if (*off >= *buf_len) {
		*off = 0;
		return 0;
	}

	if (count > *buf_len - *off) {
		count = *buf_len - *off;
	}

	left = copy_to_user(ubuf, (*buf) + *off, count);
	*off += count;

	return count;
}

/****************************** Procfs Utils **********************************/

#define PROC_DIR(name) struct proc_dir_entry *proc_dir_##name = NULL
#define PROC_FILE(name) struct proc_dir_entry *proc_file_##name = NULL

/*************** Info about current selected device (Procfs) ******************/

#define BME280INFO_BUF_MAX_LEN 512
#define PROC_FILE_BME280INFO "bme280info"

static char *bme280info_buf = NULL;
static size_t bme280info_buf_len = 0;
static u8 bme280info_reading = 0;

static ssize_t prepare_bme280info(void)
{
	ssize_t ret;

	u8 sensor_mode;
	struct bme280_data comp_data;

	mutex_lock(&bme280_devices_lock);

	memset(bme280info_buf, '\0', BME280INFO_BUF_MAX_LEN);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_settings(bme280_device);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_mode(bme280_device, &sensor_mode);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_data_forced(bme280_device, BME280_ALL,
					    &comp_data);
	if (ret != BME280_OK) {
		goto err;
	}

	bme280info_buf_len = sprintf(
		bme280info_buf,
		"I2C Adapter              : %s-%d\n"
		"I2C Address              : 0x%x\n"
		"\n"
		"Chip Id                  : 0x%x\n"
		"Power Mode               : 0x%x\n"
		"Pressure Oversampling    : 0x%x\n"
		"Temperature Oversampling : 0x%x\n"
		"Humidity Oversampling    : 0x%x\n"
		"Filter Coefficient       : 0x%x\n"
		"Standby Time             : 0x%x\n"
		"\n"
		"Pressure                 : %d\n"
		"Temperature              : %d\n"
		"Humidity                 : %d\n",
		bme280_device->client->adapter->dev.of_node->name,
		bme280_device->client->adapter->nr, bme280_device->client->addr,
		bme280_device->chip_id, sensor_mode,
		bme280_device->settings.osrs_p, bme280_device->settings.osrs_t,
		bme280_device->settings.osrs_h, bme280_device->settings.filter,
		bme280_device->settings.standby_time, comp_data.pressure,
		comp_data.temperature, comp_data.humidity);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t proc_file_bme280info_read(struct file *file, char __user *ubuf,
					 size_t count, loff_t *off)
{
	ssize_t ret;

	if (!bme280info_reading) {
		ret = prepare_bme280info();
		if (ret != BME280_OK) {
			goto err;
		}

		bme280info_reading = 1;
	}

	count = proc_read(&bme280info_buf, &bme280info_buf_len, ubuf, count,
			  off);
	if (count == 0) {
		bme280info_reading = 0;
	}

	return count;

err:
	return ret;
}

static PROC_FILE(bme280info);
static struct file_operations proc_file_bme280info_ops = {
	.owner = THIS_MODULE,
	.read = &proc_file_bme280info_read,
	.write = NULL
};

/******************** Device calibration data (Procfs) ************************/

#ifdef ENABLE_CALIB_DATA_INFO_MAPP

#define BME280CALIB_BUF_MAX_LEN 712
#define PROC_FILE_BME280CALIB "bme280calib"

static char *bme280calib_buf = NULL;
static size_t bme280calib_buf_len = 0;
static u8 bme280calib_reading = 0;

static ssize_t prepare_bme280calib(void)
{
	ssize_t ret;

	mutex_lock(&bme280_devices_lock);

	memset(bme280calib_buf, '\0', BME280CALIB_BUF_MAX_LEN);

	ret = bme280_device_null_ptr_check();
	if (ret != BME280_OK) {
		goto err;
	}

	bme280calib_buf_len = sprintf(bme280calib_buf,
				      "Temperature compensation 1 : %d\n"
				      "Temperature compensation 2 : %d\n"
				      "Temperature compensation 3 : %d\n"
				      "\n"
				      "Pressure compensation 1    : %d\n"
				      "Pressure compensation 2    : %d\n"
				      "Pressure compensation 3    : %d\n"
				      "Pressure compensation 4    : %d\n"
				      "Pressure compensation 5    : %d\n"
				      "Pressure compensation 6    : %d\n"
				      "Pressure compensation 7    : %d\n"
				      "Pressure compensation 8    : %d\n"
				      "Pressure compensation 9    : %d\n"
				      "\n"
				      "Humidity compensation 1    : %d\n"
				      "Humidity compensation 2    : %d\n"
				      "Humidity compensation 3    : %d\n"
				      "Humidity compensation 4    : %d\n"
				      "Humidity compensation 5    : %d\n"
				      "Humidity compensation 6    : %d\n",
				      bme280_device->calib_data.dig_T1,
				      bme280_device->calib_data.dig_T1,
				      bme280_device->calib_data.dig_T3,
				      bme280_device->calib_data.dig_P1,
				      bme280_device->calib_data.dig_P2,
				      bme280_device->calib_data.dig_P3,
				      bme280_device->calib_data.dig_P4,
				      bme280_device->calib_data.dig_P5,
				      bme280_device->calib_data.dig_P6,
				      bme280_device->calib_data.dig_P7,
				      bme280_device->calib_data.dig_P8,
				      bme280_device->calib_data.dig_P9,
				      bme280_device->calib_data.dig_H1,
				      bme280_device->calib_data.dig_H2,
				      bme280_device->calib_data.dig_H3,
				      bme280_device->calib_data.dig_H4,
				      bme280_device->calib_data.dig_H5,
				      bme280_device->calib_data.dig_H6);

err:
	mutex_unlock(&bme280_devices_lock);

	return ret;
}

static ssize_t proc_file_bme280calib_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *off)
{
	ssize_t ret;

	if (!bme280calib_reading) {
		ret = prepare_bme280calib();
		if (ret != BME280_OK) {
			goto err;
		}

		bme280calib_reading = 1;
	}

	count = proc_read(&bme280calib_buf, &bme280calib_buf_len, ubuf, count,
			  off);
	if (count == 0) {
		bme280calib_reading = 0;
	}

	return count;

err:
	return ret;
}

static PROC_FILE(bme280calib);
static struct file_operations proc_file_bme280calib_ops = {
	.owner = THIS_MODULE,
	.read = &proc_file_bme280calib_read,
	.write = NULL
};

#endif /* ENABLE_CALIB_DATA_INFO_MAPP */

/***************************** Public Functions *******************************/

ssize_t bme280_create_info_mapp(void)
{
	ssize_t ret;

	proc_file_bme280info =
		proc_create(PROC_FILE_BME280INFO, S_IFREG | S_IRUGO | S_IWUGO,
			    NULL, &proc_file_bme280info_ops);
	if (proc_file_bme280info == NULL) {
		pr_err(THIS_MODULE_NAME ": failed to create file"
					" '%s' in /proc\n",
		       PROC_FILE_BME280INFO);

		ret = -ENOENT;
		goto err;
	}

	bme280info_buf = kmalloc_array(BME280INFO_BUF_MAX_LEN,
				       sizeof(*bme280info_buf), GFP_KERNEL);
	if (bme280info_buf == NULL) {
		pr_err(THIS_MODULE_NAME ": failed to allocate %u bytes for"
					" file '%s' in /proc\n",
		       BME280INFO_BUF_MAX_LEN, PROC_FILE_BME280INFO);

		ret = -ENOMEM;
		goto remove_proc_file_bme280info;
	}

#ifdef ENABLE_CALIB_DATA_INFO_MAPP

	proc_file_bme280calib =
		proc_create(PROC_FILE_BME280CALIB, S_IFREG | S_IRUGO | S_IWUGO,
			    NULL, &proc_file_bme280calib_ops);
	if (proc_file_bme280calib == NULL) {
		pr_err(THIS_MODULE_NAME ": failed to create file"
					" '%s' in /proc\n",
		       PROC_FILE_BME280CALIB);

		ret = -ENOENT;
		goto cleanup_bme280info_buf;
	}

	bme280calib_buf = kmalloc_array(BME280CALIB_BUF_MAX_LEN,
					sizeof(*bme280calib_buf), GFP_KERNEL);
	if (bme280calib_buf == NULL) {
		pr_err(THIS_MODULE_NAME ": failed to allocate %u bytes for"
					" file '%s' in /proc\n",
		       BME280CALIB_BUF_MAX_LEN, PROC_FILE_BME280CALIB);

		ret = -ENOMEM;
		goto remove_proc_file_bme280calib;
	}

#endif /* ENABLE_CALIB_DATA_INFO_MAPP */

	return 0;

#ifdef ENABLE_CALIB_DATA_INFO_MAPP

remove_proc_file_bme280calib:
	remove_proc_entry(PROC_FILE_BME280CALIB, NULL);
cleanup_bme280info_buf:
	kfree(bme280info_buf);
	bme280info_buf = NULL;

#endif /* ENABLE_CALIB_DATA_INFO_MAPP */

remove_proc_file_bme280info:
	remove_proc_entry(PROC_FILE_BME280INFO, NULL);
err:
	return ret;
}

void bme280_remove_info_mapp(void)
{
	remove_proc_entry(PROC_FILE_BME280INFO, NULL);

	kfree(bme280info_buf);
	bme280info_buf = NULL;
	bme280info_buf_len = 0;
	bme280info_reading = 0;

#ifdef ENABLE_CALIB_DATA_INFO_MAPP

	remove_proc_entry(PROC_FILE_BME280CALIB, NULL);

	kfree(bme280calib_buf);
	bme280calib_buf = NULL;
	bme280calib_buf_len = 0;
	bme280calib_reading = 0;

#endif /* ENABLE_CALIB_DATA_INFO_MAPP */
}
