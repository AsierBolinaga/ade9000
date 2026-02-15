/*
 * pl_nvm_imxrt10xx.c
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_nvm.h"

#ifdef PL_NVM 
#ifdef PL_IMX_RT10XX 

#include "flexspi_flash_config.h"

pl_nvm_rv_t pl_nvm_init_imxrt10xx(pl_nvm_t* _pl_nvm, pl_nvm_config_t* _pl_nvm_config)
{
	pl_nvm_rv_t nvm_rv = PL_NVM_RV_ERROR;

    if(NULL != _pl_nvm_config)
    {
        _pl_nvm->nvm_config = _pl_nvm_config;
        if(!_pl_nvm->nvm_config->initialized)
        {
            sfw_flash_init();
            _pl_nvm->nvm_config->initialized = true;
        }
        nvm_rv = PL_NVM_RV_OK;
    }
    else
    {
        nvm_rv = PL_NVM_RV_NO_CONF;
    }

	return nvm_rv;
}

void pl_nvm_read_imxrt10xx(pl_nvm_t* _pl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size, void* _read_buff)
{
    uint32_t intial_addreas = _pl_nvm->nvm_config->nvm_sectors[_section_index].sector_offset + _offset;

    sfw_flash_read_ipc(intial_addreas, _read_buff, _size);
}

pl_nvm_rv_t pl_nvm_write_imxrt10xx(pl_nvm_t* _pl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size, void* _write_buff)
{
    pl_nvm_rv_t nvm_rv = PL_NVM_RV_ERROR;
    
    status_t status;
    volatile uint32_t primask;
    uint8_t read_buff[_size];
    uint32_t intial_addreas = _pl_nvm->nvm_config->nvm_sectors[_section_index].sector_offset + _offset;
    bool is_empty = true;

    sfw_flash_read_ipc(intial_addreas, read_buff, _size);
    for(uint32_t buff_index = 0; buff_index < _size; buff_index++)
    {
        if((uint8_t)PL_NMV_EMPTY_ADRESS != read_buff[buff_index])
        {
            is_empty = false;
        }
    }

    if(true == is_empty)
    {
        primask = DisableGlobalIRQ();
        status = sfw_flash_write(_pl_nvm->nvm_config->nvm_sectors[_section_index].sector_offset + _offset,
                                _write_buff, _size);
        EnableGlobalIRQ(primask);
        
        if (!status)
        {
            nvm_rv = PL_NVM_RV_OK;
        }
    }
    else
    {
        nvm_rv = PL_NVM_RV_AREA_NOT_EMPTY;
    }

	return nvm_rv;
}

pl_nvm_rv_t pl_nvm_erase_imxrt10xx(pl_nvm_t* _pl_nvm, uint8_t _section_index, uint32_t _offset, uint32_t _size)
{
    status_t status;
	volatile uint32_t primask;
	size_t erase_size;
	if(_size % _pl_nvm->nvm_config->sector_size)
	{
		erase_size = (((uint32_t)(_size / _pl_nvm->nvm_config->sector_size)) + 1) * _pl_nvm->nvm_config->sector_size;
	}
	else
	{
		erase_size = _size;
	}

    primask = DisableGlobalIRQ();
    status = sfw_flash_erase(_pl_nvm->nvm_config->nvm_sectors[_section_index].sector_offset + _offset, erase_size);
	EnableGlobalIRQ(primask);
    if (status)
	{
		return PL_NVM_RV_ERROR;
	}

    return PL_NVM_RV_OK;
}

uint32_t pl_nvm_get_page_size_imxrt10xx(pl_nvm_t* _pl_nvm)
{
    return _pl_nvm->nvm_config->page_size;
}

uint32_t pl_nvm_get_offset_imxrt10xx(pl_nvm_t* _pl_nvm, uint8_t _section_index)
{
    return _pl_nvm->nvm_config->nvm_sectors[_section_index].sector_offset;
}

uint32_t pl_nvm_get_size_imxrt10xx(pl_nvm_t* _pl_nvm, uint8_t _section_index)
{
    return _pl_nvm->nvm_config->nvm_sectors[_section_index].sector_size;
}

#endif /* PL_IMX_RT10XX */
#endif /* PL_NVM */
