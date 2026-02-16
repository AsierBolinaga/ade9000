/*
 * pl.nvm.h
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_NVM_H_
#define ABSL_NVM_H_

#include "absl_config.h"
#ifdef ABSL_NVM

#include "absl_types.h"
#if defined(ABSL_IMX_RT10XX)
#include "sbl_ota_flag.h"

#define ABSL_NMV_EMPTY_ADRESS    0xFFFFFFFF

#endif

typedef enum absl_nvm_rv
{
	ABSL_NVM_RV_OK = 0,
    ABSL_NVM_RV_NO_CONF,
    ABSL_NVM_RV_AREA_NOT_EMPTY,
	ABSL_NVM_RV_ERROR
}absl_nvm_rv_t;

typedef struct absl_nvm_sector
{
    uint32_t    sector_offset;
    uint32_t    sector_size;
}absl_nvm_sector_t;

typedef struct absl_nvm_config
{
    uint32_t            base_address; 
    uint32_t            sector_size;
    uint32_t            page_size;
    uint32_t            sectors_amount;
    absl_nvm_sector_t*    nvm_sectors;
    bool                initialized;
}absl_nvm_config_t;

typedef struct absl_nvm
{
    absl_nvm_config_t*    nvm_config;
#if defined(ABSL_IMX_RT10XX)
#endif
}absl_nvm_t;

#if defined(ABSL_IMX_RT10XX)
#define absl_nvm_init                 absl_nvm_init_imxrt10xx
#define absl_nvm_read                 absl_nvm_read_imxrt10xx
#define absl_nvm_write                absl_nvm_write_imxrt10xx
#define absl_nvm_erase                absl_nvm_erase_imxrt10xx
#define absl_nvm_get_page_size        absl_nvm_get_page_size_imxrt10xx
#define absl_nvm_get_offset           absl_nvm_get_offset_imxrt10xx
#define absl_nvm_get_size             absl_nvm_get_size_imxrt10xx
#else
#error Platform not defined
#endif


absl_nvm_rv_t absl_nvm_init(absl_nvm_t* _absl_nvm, absl_nvm_config_t* _absl_nvm_config);

void absl_nvm_read(absl_nvm_t* _absl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size, void* _read_buff); 

absl_nvm_rv_t absl_nvm_write(absl_nvm_t* _absl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size, void* _write_buff);

absl_nvm_rv_t absl_nvm_erase(absl_nvm_t* _absl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size);

uint32_t absl_nvm_get_page_size(absl_nvm_t* _absl_nvm); 

uint32_t absl_nvm_get_offset(absl_nvm_t* _absl_nvm, uint8_t _section_index);

uint32_t absl_nvm_get_size(absl_nvm_t* _absl_nvm, uint8_t _section_index);

#endif /* ABSL_NVM */
#endif /* ABSL_QUEUE_H_ */
