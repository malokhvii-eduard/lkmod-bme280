/**
 * @brief Modified Bosch Sensortec's BME280 pressure sensor driver for
 * linux kernel
 * 
 * @author Eduard Malokhvii <malokhvii.ee@gmail.com>
 * @version 1.0
 */

#ifndef _BME280_H
#define _BME280_H

#include <linux/types.h>
#include <linux/i2c.h>

/** Success code */
#define BME280_OK 0

/** Error codes */
#define BME280_E_NULL_PTR -1
#define BME280_E_INVALID_LEN -2
#define BME280_E_COMM_FAIL -3
#define BME280_E_SLEEP_MODE_FAIL -4
#define BME280_E_NVM_COPY_FAILED -5

/** Warning codes */
#define BME280_W_INVALID_OSRS_MACRO 1

/** I2C addresses */
#define BME280_I2C_ADDR_PRIM 0x76
#define BME280_I2C_ADDR_SEC 0x77

/** BME280 chip identifier */
#define BME280_CHIP_ID 0x60

/** Register Address */
#define BME280_CHIP_ID_ADDR 0xD0
#define BME280_RESET_ADDR 0xE0
#define BME280_TEMP_PRESS_CALIB_DATA_ADDR 0x88
#define BME280_HUM_CALIB_DATA_ADDR 0xE1
#define BME280_STATUS_ADDR 0xF3
#define BME280_CONFIG_ADDR 0xF5
#define BME280_CTRL_MEAS_ADDR 0xF4
#define BME280_CTRL_HUM_ADDR 0xF2
#define BME280_DATA_ADDR 0xF7

/** Macros related to size */
#define BME280_TEMP_PRESS_CALIB_DATA_LEN 26
#define BME280_HUM_CALIB_DATA_LEN 7
#define BME280_PRESS_TEMP_HUM_DATA_LEN 8

/** Sensor component selection macros */
#define BME280_PRESS 1
#define BME280_TEMP 1 << 1
#define BME280_HUM 1 << 2
#define BME280_ALL 0x07

/** Settings selection macros */
#define BME280_OSRS_PRESS_SEL 1
#define BME280_OSRS_TEMP_SEL 1 << 1
#define BME280_OSRS_HUM_SEL 1 << 2
#define BME280_FILTER_SEL 1 << 3
#define BME280_STANDBY_TIME_SEL 1 << 4
#define BME280_ALL_SETTINGS_SEL 0x1F

/** Sensor power modes */
#define BME280_SLEEP_MODE 0x00
#define BME280_FORCED_MODE 0x01
#define BME280_NORMAL_MODE 0x03

/** Oversampling macros */
#define BME280_NO_OVERSAMPLING 0x00
#define BME280_OVERSAMPLING_1X 0x01
#define BME280_OVERSAMPLING_2X 0x02
#define BME280_OVERSAMPLING_4X 0x03
#define BME280_OVERSAMPLING_8X 0x04
#define BME280_OVERSAMPLING_16X 0x05

/** Filter coefficient selection macros */
#define BME280_FILTER_COEFF_OFF 0x00
#define BME280_FILTER_COEFF_2 0x01
#define BME280_FILTER_COEFF_4 0x02
#define BME280_FILTER_COEFF_8 0x03
#define BME280_FILTER_COEFF_16 0x04

/** Standby duration selection macros */
#define BME280_STANDBY_TIME_0_5_MS 0x00
#define BME280_STANDBY_TIME_62_5_MS 0x01
#define BME280_STANDBY_TIME_125_MS 0x02
#define BME280_STANDBY_TIME_250_MS 0x03
#define BME280_STANDBY_TIME_500_MS 0x04
#define BME280_STANDBY_TIME_1000_MS 0x05
#define BME280_STANDBY_TIME_10_MS 0x06
#define BME280_STANDBY_TIME_20_MS 0x07

#define BME280_SOFT_RESET_COMMAND 0xB6
#define BME280_STATUS_IM_UPDATE 0x01

/** Recomended settings for indoor (Table 9 from datasheet) */
#define BME280_INDOOR_PRESS_OVERSAMPLING BME280_OVERSAMPLING_16X
#define BME280_INDOOR_TEMP_OVERSAMPLING BME280_OVERSAMPLING_2X
#define BME280_INDOOR_HUM_OVERSAMPLING BME280_OVERSAMPLING_1X
#define BME280_INDOOR_FILTER_COEFF BME280_FILTER_COEFF_16
#define BME280_INDOOR_STANDBY_TIME BME280_STANDBY_TIME_0_5_MS

/** Recomended settings for gaming (Table 10 from datasheet) */
#define BME280_GAMING_PRESS_OVERSAMPLING BME280_OVERSAMPLING_4X
#define BME280_GAMING_TEMP_OVERSAMPLING BME280_OVERSAMPLING_1X
#define BME280_GAMING_HUM_OVERSAMPLING BME280_NO_OVERSAMPLING
#define BME280_GAMING_FILTER_COEFF BME280_FILTER_COEFF_16
#define BME280_GAMING_STANDBY_TIME BME280_STANDBY_TIME_0_5_MS

/** Macro to combine two 8 bit data's to form a 16 bit data */
#define bme280_concat_bytes(msb, lsb) ((u16)msb << 8) | (u16)lsb

struct bme280_calib_data {
	u16 dig_T1; /**< Temperature compensation value */
	s16 dig_T2; /**< Temperature compensation value */
	s16 dig_T3; /**< Temperature compensation value */

	u16 dig_P1; /**< Pressure compensation value */
	s16 dig_P2; /**< Pressure compensation value */
	s16 dig_P3; /**< Pressure compensation value */
	s16 dig_P4; /**< Pressure compensation value */
	s16 dig_P5; /**< Pressure compensation value */
	s16 dig_P6; /**< Pressure compensation value */
	s16 dig_P7; /**< Pressure compensation value */
	s16 dig_P8; /**< Pressure compensation value */
	s16 dig_P9; /**< Pressure compensation value */

	u8 dig_H1; /**< Humidity compensation value */
	s16 dig_H2; /**< Humidity compensation value */
	u8 dig_H3; /**< Humidity compensation value */
	s16 dig_H4; /**< Humidity compensation value */
	s16 dig_H5; /**< Humidity compensation value */
	s8 dig_H6; /**< Humidity compensation value */

	s32 t_fine; /**< Temperature with high resolution, stored as an
		attribute as this is used for temperature compensation reading
		humidity and pressure */
};

struct bme280_data {
	u32 pressure; /**< Compensated pressure,
		- Pa = compensated pressure,
		- mm Hg = compensated pressure / 133.3224 */
	s32 temperature; /**< Comprensated temperature,
		- °C = compensated temperature / 100 */
	u32 humidity; /**< Compensated humidity,
		- % = compensated humidity / 1024 */
};

struct bme280_uncomp_data {
	u32 pressure; /**< Uncompensated pressure */
	u32 temperature; /**< Uncompensated temperature */
	u32 humidity; /**< Uncompensated humidity */
};

struct bme280_settings {
	u8 osrs_p; /**< Pressure oversampling */
	u8 osrs_t; /**< Temperature oversampling */
	u8 osrs_h; /**< Humidity oversampling */
	u8 filter; /**< Filter coefficient */
	u8 standby_time; /**< Standby time */
};

struct bme280 {
	u8 chip_id; /**< Chip Id */
	struct i2c_client *client; /**< I2C interface */
	struct bme280_settings settings; /**< Sensor settings */
	struct bme280_calib_data calib_data; /**< Calibration data */
	struct list_head registered; /**< Linked list node to hold registered
		devices */
};

/**
 * @brief Register sets the rate, filter and interface options of the device.
 * Writes to the config register in normal mode may be ignored. In sleep mode
 * mode writes are not ignored
 *
 * @see BME280_CONFIG_ADDR
 */
union bme280_config {
	struct {
		u8 spi3w_en : 1; /**< Enables 3-wire SPI interface whene set to
			1 (Not implemented) */
		u8 none : 1;
		u8 filter : 3; /**< Controls the time constant of the IRR
			filter */
		u8 t_sb : 3; /**< Controls inactive duration T standby in
			normal mode */
	};
	u8 reg;
};

/** 
 * @brief Register sets the humidity data acquisition options of the device.
 * Changes to this register only become effective after a write operation to
 * ctrl_meas
 *
 * @see BME280_CTRL_HUM_ADDR
 */
union bme280_ctrl_hum {
	struct {
		u8 osrs_h : 3; /**< Controls oversampling of humidity data */
		u8 none : 5;
	};
	u8 reg;
};

/**
 * @brief Register sets the pressure and temperature data acquisition options
 * of the device. The register needs to be written after changing ctrl_hum for
 * the changes to become effective
 *
 * @see BME280_CTRL_MEAS_ADDR 
 */
union bme280_ctrl_meas {
	struct {
		u8 mode : 2; /**< Controls the sensor mode of the device */
		u8 osrs_p : 3; /**< Controls oversampling of pressure data */
		u8 osrs_t : 3; /**< Controls oversampling of temperature data */
	};
	u8 reg;
};

/**
 * @brief Reads the cip-id and calibration data from the sensor
 *
 * @param[in, out] self : Structure instance of bme280
 * 
 * @return Result of execution status
 * @retval zero -> Success / -ve value -> Error
 */
ssize_t bme280_init(struct bme280 *self, struct i2c_client *client);

/**
 * @brief Reads the data from the given register address of the sensor
 * 
 * @param[in] self : Structure instance of bme280
 * @param[in] reg_addr : Register address from where the data to be read
 * @param[out] reg_data : Pointer to data buffer to store the read data
 * @param[in] len : Nu,ber of bytes of data to be read
 * 
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_get_regs(const struct bme280 *self, u8 reg_addr, u8 *reg_data,
			u8 len);

/**
 * @brief Writes the given data to the register address of the sensor
 * 
 * @param[in] self : Structure instance of bme280
 * @param[in] reg_addr : Register address from where the data to be written
 * @param[in] reg_data : Pointer to data buffer which is to be written in the
 * sensor
 * @param[in] len : Number of bytes of data to write
 *
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_set_regs(const struct bme280 *self, u8 *reg_addr,
			const u8 *reg_data, u8 len);

/**
 * @brief Gets the oversampling, filter and standby duration (normal mode)
 * settings from the sensor
 *
 * @param[in,out] self : Structure instance of bme280
 *
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_get_sensor_settings(struct bme280 *self);

/**
 * @brief Sets the oversampling, filter and standby duration (normal mode)
 * settings in the sensor
 *
 * @param[in] self : Structure instance of bme280
 * @param[in] desired_settings : Variable used to select the settings which
 * are to be set in the sensor
 *
 * @note : Below are the macros to be used by the user for selecting the
 * desired settings. User can do OR operation of these macros for configuring
 * multiple settings
 *
 *   Macros                 |  Functionality
 * -------------------------|------------------------------------
 *   BME280_OSRS_PRESS_SEL  |  To set pressure oversampling
 *   BME280_OSRS_TEMP_SEL   |  To set temperature oversampling
 *   BME280_OSRS_HUM_SEL    |  To set humidity oversampling
 *   BME280_FILTER_SEL      |  To set filter setting
 *   BME280_STANDBY_SEL     |  To set standby duration setting
 *
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_set_sensor_settings(const struct bme280 *self,
				   u8 desired_settings);

/**
 * @brief Gets the power mode of the sensor
 *
 * @param[in] self : Structure instance of bme280
 * @param[out] sensor_mode : Pointer variable to store the power mode
 *
 *   sensor_mode  |  Macros
 * ---------------|----------------------
 *   0            |  BME280_SLEEP_MODE
 *   1            |  BME280_FORCED_MODE
 *   3            |  BME280_NORMAL_MODE
 *
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_get_sensor_mode(const struct bme280 *self, u8 *sensor_mode);

/**
 * @brief Sets the power mode of the sensor
 *
 * @param[in] self : Structure instance of bme280
 * @param[in] sensor_mode : Variable which contains the power mode to be set
 *
 *   sensor_mode  |   Macros
 * ---------------|----------------------
 *   0            |  BME280_SLEEP_MODE
 *   1            |  BME280_FORCED_MODE
 *   3            |  BME280_NORMAL_MODE
 *
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_set_sensor_mode(const struct bme280 *self, u8 sensor_mode);

/**
 * @brief Performs the soft reset of the sensor
 *
 * @param[in] self : Structure instance of bme280
 *
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_soft_reset(const struct bme280 *self);

/**
 * @brief Reads the pressure, temperature and humidity data from the sensor,
 * compensates the data and store it in the bme280_data structure instance
 * passed by the user
 *
 * @param[in] sensor_comp : Variable which selects which data to be read from
 * the sensor.
 *
 *   sensor_comp  |  Macros
 * ---------------|----------------
 *   1            |  BME280_PRESS
 *   2            |  BME280_TEMP
 *   4            |  BME280_HUM
 *   7            |  BME280_ALL
 *
 * @param[in] self : Structure instance of bme280
 * @param[out] comp_data : Structure instance of bme280_data
 *
 * @return Result of execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
ssize_t bme280_get_sensor_data(struct bme280 *self, u8 sensor_comp,
			       struct bme280_data *comp_data);

/**
 * @brief Parse the pressure, temperature and humidity data and store it in
 * the bme280_uncomp_data structure instance
 *
 * @param[in] reg_data : Contains register data which needs to be parsed
 * @param[out] uncomp_data : Contains the uncompensated pressure, temperature
 * and humidity data
 */
void bme280_parse_sensor_data(const u8 *reg_data,
			      struct bme280_uncomp_data *uncomp_data);

/**
 * @brief Compensates the pressure and/or temperature and/or humidity data
 * according to the component selected by the user
 *
 * @param[in] sensor_comp : Used to select pressure and/or temperature and/or
 * humidity
 * @param[in] uncomp_data : Contains the uncompensated pressure, temperature and
 * humidity data
 * @param[out] comp_data : Contains the compensated pressure and/or temperature
 * and/or humidity data
 * @param[in] calib_data : Pointer to the calibration data structure
 *
 * @return Result of execution status
 * @retval zero -> Success / -ve value -> Error
 */
ssize_t bme280_compensate_data(u8 sensor_comp,
			       const struct bme280_uncomp_data *uncomp_data,
			       struct bme280_data *comp_data,
			       struct bme280_calib_data *calib_data);

#endif /* _BME280_H */
