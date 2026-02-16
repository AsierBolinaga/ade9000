/*
 * absl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_i2c.h"
#ifdef ABSL_I2C

#if defined(ABSL_IMX_RT10XX)
#include "fsl_common.h"


/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

volatile bool absl_i2c_master_completion_flag = false;

static absl_i2c_rv_t absl_i2c_init_master_imxrt10xx(absl_i2c_t * _i2c);
static absl_i2c_rv_t absl_i2c_init_slave_imxrt10xx(absl_i2c_t * _i2c);

static void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle,  status_t completionStatus, void *userData)
{
	if(!completionStatus)
	{
		absl_i2c_master_completion_flag = true;
	}
}


/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
absl_i2c_rv_t absl_i2c_init_imxrt10xx(absl_i2c_t * _i2c, absl_i2c_config_t* _i2c_config)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

//	if(NULL != _i2c_config)
//	{
		_i2c->i2c_config = _i2c_config;

		/*Clock setting for LPI2C*/
		CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);
		CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

//		NVIC_SetPriority(LPI2C1_IRQn, 1);  // TOdo - configurable
		if(_i2c_config->i2c_mode == ABSL_I2C_MODE_MASTER)
		{
			absl_i2c_rv = absl_i2c_init_master_imxrt10xx(_i2c);
		}
		else
		{
			absl_i2c_rv = absl_i2c_init_slave_imxrt10xx(_i2c);
		}
//	}
//	else
//	{
//		absl_i2c_rv = ABSL_I2C_NO_CONF;
//	}

	return absl_i2c_rv;
}

absl_i2c_rv_t absl_i2c_transfer_imxrt10xx(absl_i2c_t * _i2c, uint8_t _slave_addres, uint32_t _subaddres,
		uint8_t _subaddres_size, uint8_t* _tx_buff, uint32_t _length)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

	lpi2c_master_transfer_t master_transfer;
	status_t status;

	memset(&master_transfer, 0, sizeof(master_transfer));
	master_transfer.slaveAddress   = _slave_addres;
	master_transfer.direction      = kLPI2C_Write;
	master_transfer.subaddress     = _subaddres;
	master_transfer.subaddressSize = _subaddres_size;
	master_transfer.data           = _tx_buff;
	master_transfer.dataSize       = _length;
	master_transfer.flags          = kLPI2C_TransferDefaultFlag;

	status = LPI2C_MasterTransferNonBlocking(LPI2C3, &_i2c->master_handle, &master_transfer);
	if (status == kStatus_Success)
	{
		while(!absl_i2c_master_completion_flag);
		absl_i2c_master_completion_flag = false;
		absl_i2c_rv = ABSL_I2C_RV_OK;
	}

	return absl_i2c_rv;
}

absl_i2c_rv_t absl_i2c_receive_imxrt10xx(absl_i2c_t * _i2c, uint8_t _slave_addres, uint32_t _subaddres,
		uint8_t _subaddres_size, uint8_t* _rx_buff, uint32_t _length)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

	lpi2c_master_transfer_t master_transfer;
	status_t status;

	memset(&master_transfer, 0, sizeof(master_transfer));
	master_transfer.slaveAddress   = _slave_addres;
	master_transfer.direction      = kLPI2C_Read;
	master_transfer.subaddress     = _subaddres;
	master_transfer.subaddressSize = _subaddres_size;
	master_transfer.data           = _rx_buff;
	master_transfer.dataSize       = _length;
	master_transfer.flags          = kLPI2C_TransferDefaultFlag;

	status = LPI2C_MasterTransferNonBlocking(LPI2C3, &_i2c->master_handle, &master_transfer);
	if (status == kStatus_Success)
	{
		while(!absl_i2c_master_completion_flag);
		absl_i2c_master_completion_flag = false;
		absl_i2c_rv = ABSL_I2C_RV_OK;
	}

	return absl_i2c_rv;
}

static absl_i2c_rv_t absl_i2c_init_master_imxrt10xx(absl_i2c_t * _i2c)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

	lpi2c_master_config_t masterConfig;
	status_t status;
	uint32_t i = 0;
	/*
	 * masterConfig.debugEnable = false;
	 * masterConfig.ignoreAck = false;
	 * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
	 * masterConfig.baudRate_Hz = 100000U;
	 * masterConfig.busIdleTimeout_ns = 0;
	 * masterConfig.pinLowTimeout_ns = 0;
	 * masterConfig.sdaGlitchFilterWidth_ns = 0;
	 * masterConfig.sclGlitchFilterWidth_ns = 0;
	 */
	LPI2C_MasterGetDefaultConfig(&masterConfig);
	masterConfig.baudRate_Hz = _i2c->i2c_config->i2c_baudrate;
	
	LPI2C_MasterInit(LPI2C3, &masterConfig, LPI2C_CLOCK_FREQUENCY);

	LPI2C_MasterTransferCreateHandle(LPI2C3, &_i2c->master_handle, lpi2c_master_callback, _i2c);
	absl_i2c_rv = ABSL_I2C_RV_OK;

	return absl_i2c_rv;
}

static absl_i2c_rv_t absl_i2c_init_slave_imxrt10xx(absl_i2c_t * _i2c)
{
	// XXX - TODO: Need to be implemented
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;
	return absl_i2c_rv;
}

#endif
#endif /* ABSL_I2C */
