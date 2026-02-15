/*
 * pl.nvm.h
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_NVM_H_
#define PL_NVM_H_

#include "pl_config.h"
#ifdef PL_NVM

#include "pl_types.h"
#if defined(PL_IMX_RT10XX)
#include "sbl_ota_flag.h"

#define PL_NMV_EMPTY_ADRESS    0xFFFFFFFF

#endif

typedef enum pl_nvm_rv
{
	PL_NVM_RV_OK = 0,
    PL_NVM_RV_NO_CONF,
    PL_NVM_RV_AREA_NOT_EMPTY,
	PL_NVM_RV_ERROR
}pl_nvm_rv_t;

typedef struct pl_nvm_sector
{
    uint32_t    sector_offset;
    uint32_t    sector_size;
}pl_nvm_sector_t;

typedef struct pl_nvm_config
{
    uint32_t            base_address; 
    uint32_t            sector_size;
    uint32_t            page_size;
    uint32_t            sectors_amount;
    pl_nvm_sector_t*    nvm_sectors;
    bool                initialized;
}pl_nvm_config_t;

typedef struct pl_nvm
{
    pl_nvm_config_t*    nvm_config;
#if defined(PL_IMX_RT10XX)
#endif
}pl_nvm_t;

#if defined(PL_IMX_RT10XX)
#define pl_nvm_init                 pl_nvm_init_imxrt10xx
#define pl_nvm_read                 pl_nvm_read_imxrt10xx
#define pl_nvm_write                pl_nvm_write_imxrt10xx
#define pl_nvm_erase                pl_nvm_erase_imxrt10xx
#define pl_nvm_get_page_size        pl_nvm_get_page_size_imxrt10xx
#define pl_nvm_get_offset           pl_nvm_get_offset_imxrt10xx
#define pl_nvm_get_size             pl_nvm_get_size_imxrt10xx
#else
#error Platform not defined
#endif


pl_nvm_rv_t pl_nvm_init(pl_nvm_t* _pl_nvm, pl_nvm_config_t* _pl_nvm_config);

void pl_nvm_read(pl_nvm_t* _pl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size, void* _read_buff); 

pl_nvm_rv_t pl_nvm_write(pl_nvm_t* _pl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size, void* _write_buff);

pl_nvm_rv_t pl_nvm_erase(pl_nvm_t* _pl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size);

uint32_t pl_nvm_get_page_size(pl_nvm_t* _pl_nvm); 

uint32_t pl_nvm_get_offset(pl_nvm_t* _pl_nvm, uint8_t _section_index);

uint32_t pl_nvm_get_size(pl_nvm_t* _pl_nvm, uint8_t _section_index);

#endif /* PL_NVM */
#endif /* PL_QUEUE_H_ */
