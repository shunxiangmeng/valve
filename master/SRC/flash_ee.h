#ifndef _FLASH_EE_H
#define _FLASH_EE_H

#include "misc.h"

#define BindInfoStartAddr  ((u32)0x0800F800)                          //62k
#define BindInfoEndAddr    ((u32)0x08010000)                          //64k

//根据芯片的类型，决定每个page的大小。1k or 2k

#if defined (STM32F10X_HD) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
#define FLASH_PAGE_SIZE ((uint16_t)0x800)
#else
#define FLASH_PAGE_SIZE ((uint16_t)0x400)
#endif


unsigned char CheckSum(const u8 *p, const u8 n);
uint8_t Flash_write(uint32_t startAddr, uint32_t *data, uint32_t size);
int Flash_read(u32 startAddr, u32 *data, u32 size);


#endif

