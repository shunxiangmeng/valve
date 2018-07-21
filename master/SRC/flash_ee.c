#include <stdio.h>
#include <string.h>
#include "flash_ee.h"






uint8_t Flash_write(uint32_t startAddr, uint32_t *data, uint32_t size)
{
	volatile FLASH_Status flashStatus;
	int MemoryProgramStatus = 1;
	uint32_t i = 0x0, Address = 0x0;
	uint32_t endAddr = startAddr + size * 4;
	uint32_t erasePage = (endAddr - startAddr) / FLASH_PAGE_SIZE + 1;  //有多少个页被擦除
	
	FLASH_Unlock(); //
	//清除所有已有标志
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	//擦页
	flashStatus = FLASH_COMPLETE;

	for(i = 0; (i < erasePage) && (flashStatus == FLASH_COMPLETE); i++)
	{
		flashStatus = FLASH_ErasePage(startAddr + (FLASH_PAGE_SIZE * i));
	}
	
	//开始写数据
	Address = startAddr;
	i=0;
	while((Address < endAddr) && (flashStatus == FLASH_COMPLETE))
	{ 
		flashStatus = FLASH_ProgramWord(Address, data[i++]);
		Address = Address + 4;
	}
	
	//检验数据是否出错
	Address = startAddr;
	i=0;
	while((Address < endAddr) && (MemoryProgramStatus != 0))
	{
		if((*(vu32*) Address) != data[i++])
		{ 
			MemoryProgramStatus = 0;
			return 1;
		}
		Address += 4;
	}
	
	return 0;	
}

int Flash_read(u32 startAddr, u32 *data, u32 size)
{
	u32 EndAddr=startAddr + size * 4;
	int MemoryProgramStatus=1;
	u32 Address = 0x0;
	int i=0;
	Address = startAddr;
	while((Address < EndAddr) && (MemoryProgramStatus != 0))
	{
		data[i++]=(*(vu32*) Address);
		Address += 4;
	}
	return 0;
} 





