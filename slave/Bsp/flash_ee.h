#ifndef __FLASH_EE_H
#define __FLASH_EE_H
#include "stm32f0xx.h"

#define MyStartAddr  ((uint32_t)0x08003D00)                          //定义FLASH起始地址
#define MyEndAddr    ((uint32_t)0x08003F00)                          //定义FLASH末地址


uint8_t flash_write(uint32_t StartAddr,uint32_t *p_data,uint32_t size);


int flash_read(uint32_t StartAddr,uint32_t *p_data,uint32_t size);
unsigned char FlashCheckSum(const uint32_t *p, uint8_t n);

#endif
