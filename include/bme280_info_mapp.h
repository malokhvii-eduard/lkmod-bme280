/**
 * @brief Bosch Sensortec's BME280 information mapping in procfs
 *
 * @author Eduard Malokhvii <malokhvii.ee@gmail.com>
 * @version 1.0
 */

#ifndef _BME280_INFO_MAPP_H
#define _BME280_INFO_MAPP_H

/**
 * @brief Creates the information mapping in procfs
 *
 *   Mapping            |  Operations
 * ---------------------|--------------
 *   /proc/bme280info   |  read
 *   /proc/bme280calib  |  read
 *
 * Calibration data available only when compiled with DEBUG
 *
 * @return Result of execution
 */
ssize_t bme280_create_info_mapp(void);

/**
 * @brief Removes the information mapping in procfs
 */
void bme280_remove_info_mapp(void);

#endif /* _BME280_INFO_MAPP_H */
