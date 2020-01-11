#include <linux/slab.h>
#include <linux/delay.h>

#include <bme280.h>

#define BME280_TEMP_MIN -4000
#define BME280_TEMP_MAX 8500

#define BME280_PRESS_MIN 30000
#define BME280_PRESS_MAX 110000

#define BME280_HUM_MAX 102400

#define OVERSAMPLING_SETTINGS 0x07
#define FILTER_STANDBY_SETTINGS 0x18

/***************************** Common Functions *******************************/

static inline ssize_t null_ptr_check(const struct bme280 *self)
{
	return self == NULL ? BME280_E_NULL_PTR : BME280_OK;
}

/******************** Device Calibration Data Functions ***********************/

static void parse_temp_press_calib_data(struct bme280 *self, const u8 *reg_data)
{
	struct bme280_calib_data *calib_data = &self->calib_data;

	calib_data->dig_T1 = bme280_concat_bytes(reg_data[1], reg_data[0]);
	calib_data->dig_T2 = (s16)bme280_concat_bytes(reg_data[3], reg_data[2]);
	calib_data->dig_T3 = (s16)bme280_concat_bytes(reg_data[5], reg_data[4]);
	calib_data->dig_P1 = bme280_concat_bytes(reg_data[7], reg_data[6]);
	calib_data->dig_P2 = (s16)bme280_concat_bytes(reg_data[9], reg_data[8]);
	calib_data->dig_P3 =
		(s16)bme280_concat_bytes(reg_data[11], reg_data[10]);
	calib_data->dig_P4 =
		(s16)bme280_concat_bytes(reg_data[13], reg_data[12]);
	calib_data->dig_P5 =
		(s16)bme280_concat_bytes(reg_data[15], reg_data[14]);
	calib_data->dig_P6 =
		(s16)bme280_concat_bytes(reg_data[17], reg_data[16]);
	calib_data->dig_P7 =
		(s16)bme280_concat_bytes(reg_data[19], reg_data[18]);
	calib_data->dig_P8 =
		(s16)bme280_concat_bytes(reg_data[21], reg_data[20]);
	calib_data->dig_P9 =
		(s16)bme280_concat_bytes(reg_data[23], reg_data[22]);
	calib_data->dig_H1 = reg_data[25];
}

static void parse_humidity_calib_data(struct bme280 *self, const u8 *reg_data)
{
	struct bme280_calib_data *calib_data = &self->calib_data;

	s16 dig_H4_msb;
	s16 dig_H4_lsb;
	s16 dig_H5_msb;
	s16 dig_H5_lsb;

	calib_data->dig_H2 = (s16)bme280_concat_bytes(reg_data[1], reg_data[0]);
	calib_data->dig_H3 = reg_data[2];
	dig_H4_msb = (s16)(s8)reg_data[3] * 16;
	dig_H4_lsb = (s16)(reg_data[4] & 0x0F);
	calib_data->dig_H4 = dig_H4_msb | dig_H4_lsb;
	dig_H5_msb = (s16)(s8)reg_data[5] * 16;
	dig_H5_lsb = (s16)(reg_data[4] >> 4);
	calib_data->dig_H5 = dig_H5_msb | dig_H5_lsb;
	calib_data->dig_H6 = (s8)reg_data[6];
}

static ssize_t get_calib_data(struct bme280 *self)
{
	ssize_t ret;

	u8 calib_data[BME280_TEMP_PRESS_CALIB_DATA_LEN] = { 0 };
	u8 reg_addr = BME280_TEMP_PRESS_CALIB_DATA_ADDR;

	ret = bme280_get_regs(self, reg_addr, calib_data,
			      BME280_TEMP_PRESS_CALIB_DATA_LEN);
	if (ret != BME280_OK) {
		goto err;
	}
	parse_temp_press_calib_data(self, calib_data);

	reg_addr = BME280_HUM_CALIB_DATA_ADDR;

	ret = bme280_get_regs(self, reg_addr, calib_data,
			      BME280_HUM_CALIB_DATA_LEN);
	if (ret != BME280_OK) {
		goto err;
	}
	parse_humidity_calib_data(self, calib_data);

	return BME280_OK;

err:
	return ret;
}

/********************** Device Configuration Functions ************************/

static inline u8 are_settings_changed(u8 sub_settings, u8 desired_settings)
{
	return sub_settings & desired_settings ? 1 : 0;
}

static void parse_device_settings(const union bme280_config *config,
				  const union bme280_ctrl_meas *ctrl_meas,
				  const union bme280_ctrl_hum *ctrl_hum,
				  struct bme280_settings *settings)
{
	settings->osrs_h = ctrl_hum->osrs_h;
	settings->osrs_p = ctrl_meas->osrs_p;
	settings->osrs_t = ctrl_meas->osrs_t;
	settings->filter = config->filter;
	settings->standby_time = config->t_sb;
}

static ssize_t
set_osrs_press_temp_settings(const struct bme280 *self, u8 desired_settings,
			     const struct bme280_settings *settings)
{
	ssize_t ret;

	u8 reg_addr = BME280_CTRL_MEAS_ADDR;
	union bme280_ctrl_meas ctrl_meas;

	ret = bme280_get_regs(self, reg_addr, &ctrl_meas.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	if (desired_settings & BME280_OSRS_PRESS_SEL) {
		ctrl_meas.osrs_t = settings->osrs_p;
	}

	if (desired_settings & BME280_OSRS_TEMP_SEL) {
		ctrl_meas.osrs_t = settings->osrs_t;
	}

	ret = bme280_set_regs(self, &reg_addr, &ctrl_meas.reg, 1);

err:
	return ret;
}

static ssize_t
set_osrs_humidity_settings(const struct bme280 *self,
			   const struct bme280_settings *settings)
{
	ssize_t ret;

	u8 reg_addr = BME280_CTRL_HUM_ADDR;
	union bme280_ctrl_hum ctrl_hum;
	union bme280_ctrl_meas ctrl_meas;

	ctrl_hum.osrs_h = settings->osrs_h;

	ret = bme280_set_regs(self, &reg_addr, &ctrl_hum.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	reg_addr = BME280_CTRL_MEAS_ADDR;

	ret = bme280_get_regs(self, reg_addr, &ctrl_meas.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_set_regs(self, &reg_addr, &ctrl_meas.reg, 1);

err:
	return ret;
}

static ssize_t set_osrs_settings(const struct bme280 *self, u8 desired_settings,
				 const struct bme280_settings *settings)
{
	ssize_t ret = BME280_W_INVALID_OSRS_MACRO;

	if (desired_settings & BME280_OSRS_HUM_SEL) {
		ret = set_osrs_humidity_settings(self, settings);
	}

	if (desired_settings & (BME280_OSRS_PRESS_SEL | BME280_OSRS_TEMP_SEL)) {
		ret = set_osrs_press_temp_settings(self, desired_settings,
						   settings);
	}

	return ret;
}

static ssize_t
set_filter_standby_time_settings(const struct bme280 *self, u8 desired_settings,
				 const struct bme280_settings *settings)
{
	ssize_t ret;

	u8 reg_addr = BME280_CONFIG_ADDR;
	union bme280_config config;

	ret = bme280_get_regs(self, reg_addr, &config.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	if (desired_settings & BME280_FILTER_SEL) {
		config.filter = settings->filter;
	}

	if (desired_settings & BME280_STANDBY_TIME_SEL) {
		config.t_sb = settings->standby_time;
	}

	ret = bme280_set_regs(self, &reg_addr, &config.reg, 1);

err:
	return ret;
}

static ssize_t reload_device_settings(const struct bme280 *self,
				      const struct bme280_settings *settings)
{
	ssize_t ret;

	ret = set_osrs_settings(self, BME280_ALL_SETTINGS_SEL, settings);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = set_filter_standby_time_settings(self, BME280_ALL_SETTINGS_SEL,
					       settings);

err:
	return ret;
}

/********************** Device Power Control Functions ************************/

static ssize_t write_power_mode(const struct bme280 *self, u8 sensor_mode)
{
	ssize_t ret;

	u8 reg_addr = BME280_CTRL_MEAS_ADDR;
	union bme280_ctrl_meas ctrl_meas;

	ret = bme280_get_regs(self, reg_addr, &ctrl_meas.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	ctrl_meas.mode = sensor_mode;
	ret = bme280_set_regs(self, &reg_addr, &ctrl_meas.reg, 1);

err:
	return ret;
}

static ssize_t put_device_to_sleep(const struct bme280 *self)
{
	ssize_t ret;

	union bme280_config config;
	union bme280_ctrl_meas ctrl_meas;
	union bme280_ctrl_hum ctrl_hum;
	struct bme280_settings settings;

	ret = bme280_get_regs(self, BME280_CTRL_HUM_ADDR, &ctrl_hum.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_regs(self, BME280_CTRL_MEAS_ADDR, &ctrl_meas.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_regs(self, BME280_CONFIG_ADDR, &config.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	parse_device_settings(&config, &ctrl_meas, &ctrl_hum, &settings);

	ret = bme280_soft_reset(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = reload_device_settings(self, &settings);

err:
	return ret;
}

/*********************** Data Compensation Functions **************************/

static u32 compensate_pressure(const struct bme280_uncomp_data *uncomp_data,
			       const struct bme280_calib_data *calib_data)
{
	s32 var1;
	s32 var2;
	s32 var3;
	s32 var4;
	u32 var5;
	u32 pressure;

	var1 = (((s32)calib_data->t_fine) / 2) - (s32)64000;
	var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((s32)calib_data->dig_P6);
	var2 = var2 + ((var1 * ((s32)calib_data->dig_P5)) * 2);
	var2 = (var2 / 4) + (((s32)calib_data->dig_P4) * 65536);
	var3 = (calib_data->dig_P3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
	var4 = (((s32)calib_data->dig_P2) * var1) / 2;
	var1 = (var3 + var4) / 262144;
	var1 = (((32768 + var1)) * ((s32)calib_data->dig_P1)) / 32768;

	if (var1) {
		var5 = (u32)((u32)1048576) - uncomp_data->pressure;
		pressure = ((u32)(var5 - (u32)(var2 / 4096))) * 3125;
		if (pressure < 0x80000000) {
			pressure = (pressure << 1) / ((u32)var1);
		} else {
			pressure = (pressure / (u32)var1) * 2;
		}

		var1 = (((s32)calib_data->dig_P9) *
			((s32)(((pressure / 8) * (pressure / 8)) / 8192))) /
		       4096;
		var2 = (((s32)(pressure / 4)) * ((s32)calib_data->dig_P8)) /
		       8192;

		pressure = (u32)((s32)pressure +
				 ((var1 + var2 + calib_data->dig_P7) / 16));

		if (pressure < BME280_PRESS_MIN) {
			pressure = BME280_PRESS_MIN;
		} else if (pressure > BME280_PRESS_MAX) {
			pressure = BME280_PRESS_MAX;
		}
	} else {
		pressure = BME280_PRESS_MIN;
	}

	return pressure;
}

static s32 compensate_temperature(const struct bme280_uncomp_data *uncomp_data,
				  struct bme280_calib_data *calib_data)
{
	s32 var1;
	s32 var2;
	s32 temperature;

	var1 = (s32)((uncomp_data->temperature / 8) -
		     ((s32)calib_data->dig_T1 * 2));
	var1 = (var1 * ((s32)calib_data->dig_T2)) / 2048;
	var2 = (s32)((uncomp_data->temperature / 16) -
		     ((s32)calib_data->dig_T1));
	var2 = (((var2 * var2) / 4096) * ((s32)calib_data->dig_T3)) / 16384;

	calib_data->t_fine = var1 + var2;
	temperature = (calib_data->t_fine * 5 + 128) / 256;

	if (temperature < BME280_TEMP_MIN) {
		temperature = BME280_TEMP_MIN;
	} else if (temperature > BME280_TEMP_MAX) {
		temperature = BME280_TEMP_MAX;
	}

	return temperature;
}

static u32 compensate_humidity(const struct bme280_uncomp_data *uncomp_data,
			       const struct bme280_calib_data *calib_data)
{
	s32 var1;
	s32 var2;
	s32 var3;
	s32 var4;
	s32 var5;
	u32 humidity;

	var1 = calib_data->t_fine - ((s32)76800);
	var2 = (s32)(uncomp_data->humidity * 16384);
	var3 = (s32)(((s32)calib_data->dig_H4) * 1048576);
	var4 = ((s32)calib_data->dig_H5) * var1;
	var5 = (((var2 - var3) - var4) + (s32)16384) / 32768;
	var2 = (var1 * ((s32)calib_data->dig_H6)) / 1024;
	var3 = (var1 * ((s32)calib_data->dig_H3)) / 2048;
	var4 = ((var2 * (var3 + (s32)32768)) / 1024) + (s32)2097152;
	var2 = ((var4 * ((s32)calib_data->dig_H2)) + 8192) / 16384;
	var3 = var5 * var2;
	var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
	var5 = var3 - ((var4 * ((s32)calib_data->dig_H1)) / 16);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);

	humidity = (u32)(var5 / 4096);

	if (humidity > BME280_HUM_MAX) {
		humidity = BME280_HUM_MAX;
	}

	return humidity;
}

/***************************** Public Functions *******************************/

ssize_t bme280_init(struct bme280 *self, struct i2c_client *client)
{
	ssize_t ret;

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	if (client == NULL) {
		ret = BME280_E_NULL_PTR;
		goto err;
	}

	i2c_set_clientdata(client, self);

	self->client = client;

	ret = bme280_get_regs(self, BME280_CHIP_ID_ADDR, &self->chip_id, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_soft_reset(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = get_calib_data(self);

err:
	return ret;
}

ssize_t bme280_get_regs(const struct bme280 *self, u8 reg_addr, u8 *reg_data,
			u8 len)
{
	ssize_t ret;

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = i2c_smbus_read_i2c_block_data(self->client, reg_addr, len,
					    reg_data);
	if (ret <= 0) {
		ret = BME280_E_COMM_FAIL;
		goto err;
	}

	return BME280_OK;

err:
	return ret;
}

ssize_t bme280_set_regs(const struct bme280 *self, u8 *reg_addr,
			const u8 *reg_data, u8 len)
{
	ssize_t ret;

	u8 i;

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	if ((reg_addr == NULL) || (reg_data == NULL)) {
		ret = BME280_E_NULL_PTR;
		goto err;
	}

	if (len <= 0) {
		ret = BME280_E_INVALID_LEN;
		goto err;
	}

	for (i = 0; i < len; i++) {
		ret = i2c_smbus_write_byte_data(self->client, reg_addr[i],
						reg_data[i]);
		if (ret) {
			ret = BME280_E_COMM_FAIL;
			goto err;
		}
	}

	return BME280_OK;

err:
	return ret;
}

ssize_t bme280_get_sensor_settings(struct bme280 *self)
{
	ssize_t ret;

	union bme280_config config;
	union bme280_ctrl_meas ctrl_meas;
	union bme280_ctrl_hum ctrl_hum;

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_regs(self, BME280_CTRL_HUM_ADDR, &ctrl_hum.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_regs(self, BME280_CTRL_MEAS_ADDR, &ctrl_meas.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_regs(self, BME280_CONFIG_ADDR, &config.reg, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	parse_device_settings(&config, &ctrl_meas, &ctrl_hum, &self->settings);

err:
	return ret;
}

ssize_t bme280_set_sensor_settings(const struct bme280 *self,
				   u8 desired_settings)
{
	ssize_t ret;

	u8 sensor_mode;

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_mode(self, &sensor_mode);
	if ((ret == BME280_OK) & (sensor_mode != BME280_SLEEP_MODE)) {
		ret = put_device_to_sleep(self);
	}

	if (ret != BME280_OK) {
		goto err;
	}

	if (are_settings_changed(OVERSAMPLING_SETTINGS, desired_settings)) {
		ret = set_osrs_settings(self, desired_settings,
					&self->settings);
	}

	if ((ret == BME280_OK) &&
	    are_settings_changed(FILTER_STANDBY_SETTINGS, desired_settings)) {
		ret = set_filter_standby_time_settings(self, desired_settings,
						       &self->settings);
	}

err:
	return ret;
}

ssize_t bme280_get_sensor_mode(const struct bme280 *self, u8 *sensor_mode)
{
	ssize_t ret;

	union bme280_ctrl_meas ctrl_meas;

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_regs(self, BME280_CTRL_MEAS_ADDR, &ctrl_meas.reg, 1);
	*sensor_mode = ctrl_meas.mode;

err:
	return ret;
}

ssize_t bme280_set_sensor_mode(const struct bme280 *self, u8 sensor_mode)
{
	ssize_t ret;

	u8 last_sensor_mode;

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_get_sensor_mode(self, &last_sensor_mode);

	if ((ret == BME280_OK) && (last_sensor_mode != BME280_SLEEP_MODE)) {
		ret = put_device_to_sleep(self);
	}

	if (ret == BME280_OK) {
		ret = write_power_mode(self, sensor_mode);
	}

err:
	return ret;
}

ssize_t bme280_soft_reset(const struct bme280 *self)
{
	ssize_t ret;

	u8 reg_addr = BME280_RESET_ADDR;
	u8 soft_reset_command = BME280_SOFT_RESET_COMMAND;
	u8 status_reg = 0;
	u8 retries = 5; /* Retry soft reset command 5 times */

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	ret = bme280_set_regs(self, &reg_addr, &soft_reset_command, 1);
	if (ret != BME280_OK) {
		goto err;
	}

	/* If NVM not copied yet, Wait for NVM to copy */
	do {
		/* As per data sheet - Table 1, startup time is 2 ms. */
		msleep(2);
		ret = bme280_get_regs(self, BME280_STATUS_ADDR, &status_reg, 1);
	} while ((ret == BME280_OK) && (retries--) &&
		 (status_reg & BME280_STATUS_IM_UPDATE));

	if (status_reg & BME280_STATUS_IM_UPDATE) {
		ret = BME280_E_NVM_COPY_FAILED;
		goto err;
	}

err:
	return ret;
}

ssize_t bme280_get_sensor_data(struct bme280 *self, u8 sensor_comp,
			       struct bme280_data *comp_data)
{
	ssize_t ret;

	u8 reg_data[BME280_PRESS_TEMP_HUM_DATA_LEN] = { 0 };
	struct bme280_uncomp_data uncomp_data = { 0 };

	ret = null_ptr_check(self);
	if (ret != BME280_OK) {
		goto err;
	}

	if (comp_data == NULL) {
		ret = BME280_E_NULL_PTR;
		goto err;
	}

	ret = bme280_get_regs(self, BME280_DATA_ADDR, reg_data,
			      BME280_PRESS_TEMP_HUM_DATA_LEN);
	if (ret == BME280_OK) {
		bme280_parse_sensor_data(reg_data, &uncomp_data);
		ret = bme280_compensate_data(sensor_comp, &uncomp_data,
					     comp_data, &self->calib_data);
	}

err:
	return ret;
}

void bme280_parse_sensor_data(const u8 *reg_data,
			      struct bme280_uncomp_data *uncomp_data)
{
	u32 data_xlsb;
	u32 data_lsb;
	u32 data_msb;

	data_msb = (u32)reg_data[0] << 12;
	data_lsb = (u32)reg_data[1] << 4;
	data_xlsb = (u32)reg_data[2] >> 4;
	uncomp_data->pressure = data_msb | data_lsb | data_xlsb;

	data_msb = (u32)reg_data[3] << 12;
	data_lsb = (u32)reg_data[4] << 4;
	data_xlsb = (u32)reg_data[5] >> 4;
	uncomp_data->temperature = data_msb | data_lsb | data_xlsb;

	data_lsb = (u32)reg_data[6] << 8;
	data_msb = (u32)reg_data[7];
	uncomp_data->humidity = data_msb | data_lsb;
}

ssize_t bme280_compensate_data(u8 sensor_comp,
			       const struct bme280_uncomp_data *uncomp_data,
			       struct bme280_data *comp_data,
			       struct bme280_calib_data *calib_data)
{
	ssize_t ret;

	if ((uncomp_data == NULL) && (comp_data == NULL) &&
	    (calib_data == NULL)) {
		ret = BME280_E_NULL_PTR;
		goto err;
	}

	comp_data->temperature = 0;
	comp_data->pressure = 0;
	comp_data->humidity = 0;

	if (sensor_comp & (BME280_PRESS | BME280_TEMP | BME280_HUM)) {
		comp_data->temperature =
			compensate_temperature(uncomp_data, calib_data);
	}

	if (sensor_comp & BME280_PRESS) {
		comp_data->pressure =
			compensate_pressure(uncomp_data, calib_data);
	}

	if (sensor_comp & BME280_HUM) {
		comp_data->humidity =
			compensate_humidity(uncomp_data, calib_data);
	}

	return BME280_OK;

err:
	return ret;
}
