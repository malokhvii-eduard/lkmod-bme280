/**
 * @brief Bosch Sensortec's BME280 registers and data mapping in sysfs
 *
 * @author Eduard Malokhvii <malokhvii.ee@gmail.com>
 * @version 1.0
 */

#ifndef _BME280_REGS_MAPP_H
#define _BME280_REGS_MAPP_H

/**
 * @brief Creates the registers and data mapping in sysfs
 *
 * If you want to switch to another device, use /sys/bme280/i2c mapping, write
 * to it I2C adapter number in decimal and device address in hex
 *
 *   Mapping                         |  Operations
 * ----------------------------------|--------------
 *   /sys/class/bme280/i2c           |  read/write
 *   /sys/class/bme280/dig_T1        |  read
 *   /sys/class/bme280/dig_T2        |  read
 *   /sys/class/bme280/dig_T3        |  read
 *   /sys/class/bme280/dig_P1        |  read
 *   /sys/class/bme280/dig_P2        |  read
 *   /sys/class/bme280/dig_P3        |  read
 *   /sys/class/bme280/dig_P4        |  read
 *   /sys/class/bme280/dig_P5        |  read
 *   /sys/class/bme280/dig_P6        |  read
 *   /sys/class/bme280/dig_P7        |  read
 *   /sys/class/bme280/dig_P8        |  read
 *   /sys/class/bme280/dig_P9        |  read
 *   /sys/class/bme280/dig_H1        |  read
 *   /sys/class/bme280/dig_H2        |  read
 *   /sys/class/bme280/dig_H3        |  read
 *   /sys/class/bme280/dig_H4        |  read
 *   /sys/class/bme280/dig_H5        |  read
 *   /sys/class/bme280/dig_H6        |  read
 *   /sys/class/bme280/chip_id       |  read
 *   /sys/class/bme280/reset         |  write
 *   /sys/class/bme280/mode          |  read/write
 *   /sys/class/bme280/osrs_p        |  read/write
 *   /sys/class/bme280/osrs_t        |  read/write
 *   /sys/class/bme280/osrs_h        |  read/write
 *   /sys/class/bme280/filter        |  read/write
 *   /sys/class/bme280/standby_time  |  read/write
 *   /sys/class/bme280/pressure      |  read
 *   /sys/class/bme280/temperature   |  read
 *   /sys/class/bme280/humidity      |  read
 *
 * Calibration data registers available only when compiled with DEBUG. All
 * other registers available always
 *
 * @return Result of execution
 */
ssize_t bme280_create_regs_mapp(void);

/**
 * @brief Removes the registers and data mapping in sysfs
 */
void bme280_remove_regs_mapp(void);

#endif /* _BME280_REGS_MAPP_H */
