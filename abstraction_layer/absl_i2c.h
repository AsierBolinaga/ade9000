/*
 * absl_spi.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_I2C_H_
#define ABSL_I2C_H_

#include "absl_config.h"
#ifdef ABSL_I2C

#include "absl_types.h"

#if defined(ABSL_IMX_RT10XX)
#include "fsl_lpi2c.h"
#elif defined(ABSL_BEAGLEBONE)
#elif defined(ABSL_PC)
#else
#error "ERROR! no platform defined!!"
#endif

typedef enum absl_i2c_rv
{
    ABSL_I2C_RV_OK = 0x0U,
	ABSL_I2C_NO_CONF,
    ABSL_I2C_RV_ERROR
} absl_i2c_rv_t;

typedef enum absl_i2c_mode
{
	ABSL_I2C_MODE_MASTER = 0,
	ABSL_I2C_MODE_SLAVE
}absl_i2c_mode_t;

typedef struct absl_i2c_config
{
#if defined(ABSL_IMX_RT10XX)
#elif defined(ABSL_BEAGLEBONE) || defined(ABSL_PC)
	uint32_t		i2c_bus;
#endif
	absl_i2c_mode_t 	i2c_mode;
	uint32_t		i2c_baudrate;
} absl_i2c_config_t;

typedef struct absl_i2c
{
#if defined(ABSL_IMX_RT10XX)
	lpi2c_master_handle_t 	master_handle;
#elif defined(ABSL_BEAGLEBONE)
	uint32_t   		i2c_file;
#endif
	absl_i2c_config_t*		i2c_config;
}absl_i2c_t;

#if defined(ABSL_IMX_RT10XX)
#define absl_i2c_init			absl_i2c_init_imxrt10xx
#define absl_i2c_transfer		absl_i2c_transfer_imxrt10xx
#define absl_i2c_receive		absl_i2c_receive_imxrt10xx
#elif defined(ABSL_BEAGLEBONE)
#define absl_i2c_init			absl_i2c_init_beaglebone
#define absl_i2c_transfer		absl_i2c_transfer_beaglebone
#define absl_i2c_receive		absl_i2c_receive_beaglebone
#elif defined(ABSL_PC)
#define absl_i2c_init			absl_i2c_init_pc
#define absl_i2c_transfer		absl_i2c_transfer_pc
#define absl_i2c_receive		absl_i2c_receive_pc
#else
#error Platform not defined
#endif

absl_i2c_rv_t absl_i2c_init(absl_i2c_t * _i2c, absl_i2c_config_t* _i2c_config);

absl_i2c_rv_t absl_i2c_transfer(absl_i2c_t * _i2c, uint8_t _slave_addres, uint32_t _subaddres,
						    uint8_t _subaddres_size, uint8_t* _tx_buff, uint32_t _length);
absl_i2c_rv_t absl_i2c_receive(absl_i2c_t * _i2c, uint8_t _slave_addres, uint32_t _subaddres,
		                   uint8_t _subaddres_size, uint8_t* _rx_buff, uint32_t _length);

#endif /* ABSL_I2C */
#endif /* ABSL_I2C_H_ */
