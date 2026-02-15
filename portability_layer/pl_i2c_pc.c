/*
 * pl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_i2c.h"
#ifdef PL_I2C

#if defined(PL_PC)

/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
pl_i2c_rv_t pl_i2c_init_pc(pl_i2c_t * _i2c, pl_i2c_config_t* _i2c_config)
{
	pl_i2c_rv_t	pl_i2c_rv = PL_I2C_RV_ERROR;

	pl_i2c_rv = PL_I2C_RV_OK;

	return pl_i2c_rv;
}

pl_i2c_rv_t pl_i2c_transfer_pc(pl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _tx_buff, uint32_t _length)
{
	pl_i2c_rv_t	pl_i2c_rv = PL_I2C_RV_ERROR;

	pl_i2c_rv = PL_I2C_RV_OK;

	return pl_i2c_rv;
}

pl_i2c_rv_t pl_i2c_receive_pc(pl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _rx_buff, uint32_t _length)
{
	pl_i2c_rv_t	pl_i2c_rv = PL_I2C_RV_ERROR;

	pl_i2c_rv = PL_I2C_RV_OK;

	return pl_i2c_rv;
}

#endif /* PL_PC */
#endif /* PL_I2C */
