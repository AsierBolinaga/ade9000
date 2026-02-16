/*
 * absl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_i2c.h"
#ifdef ABSL_I2C

#if defined(ABSL_BEAGLEBONE)

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include "i2cbusses.h"
#include "util.h"
#include "version.h"

static int absl_i2c_check_funcs(int _i2c_file)
{
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(_i2c_file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
             "functionality matrix: %s\n", strerror(errno));
		 return -1;
	}
    if (!(funcs & I2C_FUNC_I2C)) {
        fprintf(stderr, MISSING_FUNC_FMT, "I2C transfers");
        return -1;
    }
	return 0;
}


/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
absl_i2c_rv_t absl_i2c_init_beaglebone(absl_i2c_t * _i2c, absl_i2c_config_t* _i2c_config)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;
	char 		filename[20];

	if(NULL != _i2c_config)
	{
		_i2c->i2c_config = _i2c_config;
		_i2c->i2c_file = open_i2c_dev(_i2c_config->i2c_bus, filename, sizeof(filename), 0);
		if(_i2c->i2c_file  >= 0  && !absl_i2c_check_funcs(_i2c->i2c_file))
		{
			absl_i2c_rv = ABSL_I2C_RV_OK;
		}
	}

	return absl_i2c_rv;
}

absl_i2c_rv_t absl_i2c_transfer_beaglebone(absl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _tx_buff, uint32_t _length)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

    uint32_t nmsgs_sent;
    struct i2c_msg i2c_message;
    struct i2c_rdwr_ioctl_data rdwr;

    i2c_message.addr = _slave_addres;
    i2c_message.flags = 0;
    i2c_message.buf = _tx_buff;
    i2c_message.len = _length;

    rdwr.msgs = &i2c_message;
    rdwr.nmsgs = 1;

    nmsgs_sent = ioctl(_i2c->i2c_file, I2C_RDWR, &rdwr);
    if (nmsgs_sent >= 0) 
    {
		absl_i2c_rv = ABSL_I2C_RV_OK;
	}

	return absl_i2c_rv;
}

absl_i2c_rv_t absl_i2c_receive_beaglebone(absl_i2c_t * _i2c, uint8_t _slave_addres, uint8_t* _rx_buff, uint32_t _length)
{
	absl_i2c_rv_t	absl_i2c_rv = ABSL_I2C_RV_ERROR;

    int force = 0;
	int daddress = 0;

	set_slave_addr(_i2c->i2c_file, _slave_addres, force);
	if (ioctl(_i2c->i2c_file, I2C_PEC, 1) >= 0)
	{
		if(i2c_smbus_read_i2c_block_data(_i2c->i2c_file, daddress, _length, _rx_buff) >= 0)
		{
			absl_i2c_rv = ABSL_I2C_RV_OK;
		}
	}

	return absl_i2c_rv;
}

#endif /* ABSL_BEAGLEBONE */
#endif /* ABSL_I2C */
