/*
 * absl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_i2c.h"
#ifdef ABSL_I2C

#if defined(ABSL_PC)

/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
absl_i2c_rv_t absl_i2c_init_pc(absl_i2c_t * _i2c, absl_i2c_config_t* _i2c_config)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

	absl_i2c_rv = ABSL_I2C_RV_OK;

	return absl_i2c_rv;
}

absl_i2c_rv_t absl_i2c_transfer_pc(absl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _tx_buff, uint32_t _length)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

	absl_i2c_rv = ABSL_I2C_RV_OK;

	return absl_i2c_rv;
}

absl_i2c_rv_t absl_i2c_receive_pc(absl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _rx_buff, uint32_t _length)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

	absl_i2c_rv = ABSL_I2C_RV_OK;

	return absl_i2c_rv;
}

#endif /* ABSL_PC */
#endif /* ABSL_I2C */
