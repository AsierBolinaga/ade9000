/*
 * pl_spi.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_I2C_H_
#define PL_I2C_H_

#include "pl_config.h"
#ifdef PL_I2C

#include "pl_types.h"

#if defined(PL_IMX_RT10XX)
#include "fsl_lpi2c.h"
#elif defined(PL_BEAGLEBONE)
#elif defined(PL_PC)
#else
#error "ERROR! no platform defined!!"
#endif

typedef enum pl_i2c_rv
{
    PL_I2C_RV_OK = 0x0U,
	PL_I2C_NO_CONF,
    PL_I2C_RV_ERROR
} pl_i2c_rv_t;

typedef enum pl_i2c_mode
{
	PL_I2C_MODE_MASTER = 0,
	PL_I2C_MODE_SLAVE
}pl_i2c_mode_t;

typedef struct pl_i2c_config
{
#if defined(PL_IMX_RT10XX)
#elif defined(PL_BEAGLEBONE) || defined(PL_PC)
	uint32_t		i2c_bus;
#endif
	pl_i2c_mode_t 	i2c_mode;
	uint32_t		i2c_baudrate;
} pl_i2c_config_t;

typedef struct pl_i2c
{
#if defined(PL_IMX_RT10XX)
	lpi2c_master_handle_t 	master_handle;
#elif defined(PL_BEAGLEBONE)
	uint32_t   		i2c_file;
#endif
	pl_i2c_config_t*		i2c_config;
}pl_i2c_t;

#if defined(PL_IMX_RT10XX)
#define pl_i2c_init			pl_i2c_init_imxrt10xx
#define pl_i2c_transfer		pl_i2c_transfer_imxrt10xx
#define pl_i2c_receive		pl_i2c_receive_imxrt10xx
#elif defined(PL_BEAGLEBONE)
#define pl_i2c_init			pl_i2c_init_beaglebone
#define pl_i2c_transfer		pl_i2c_transfer_beaglebone
#define pl_i2c_receive		pl_i2c_receive_beaglebone
#elif defined(PL_PC)
#define pl_i2c_init			pl_i2c_init_pc
#define pl_i2c_transfer		pl_i2c_transfer_pc
#define pl_i2c_receive		pl_i2c_receive_pc
#else
#error Platform not defined
#endif

pl_i2c_rv_t pl_i2c_init(pl_i2c_t * _i2c, pl_i2c_config_t* _i2c_config);

pl_i2c_rv_t pl_i2c_transfer(pl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _tx_buff, uint32_t _length);
pl_i2c_rv_t pl_i2c_receive(pl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _rx_buff, uint32_t _length);

#endif /* PL_I2C */
#endif /* PL_I2C_H_ */
