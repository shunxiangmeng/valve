#ifndef _FLASH_EE_H
#define _FLASH_EE_H

#include "misc.h"

typedef struct
{
	uint32_t isBlind;      //蓝牙绑定标志位
	uint32_t count;        //绑定的个数
	char     blindMac[3][16]; //绑定的小程序地址1
	uint8_t  chechSum;     //校验码
	char     res[3];
}BLE_BLIND_INFO;

unsigned char CheckSum(const u8 *p, const u8 n);
uint8_t Flash_write(uint32_t startAddr, uint32_t *data, uint32_t size);
int Flash_read(u32 startAddr, u32 *data, u32 size);

void SysInfo_write(void);
void SysInfo_read(void);
int BLE_blind(char* mac);

#endif

