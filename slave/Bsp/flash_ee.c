#include "flash_ee.h"
/*******************************************************************************

flash的写函数：

输入：

uint32_t StartAddr flash起始地址

uint32_t *p_data 待写入数据指针,比如一个数组的名字。此数组必须是32位的数据。

uint32_t size 写入数据的数量，比如数组的长度。

输出：

0：正确执行

非0：出错

注意：输入数据一定是uint32_t 的指针，即数据一定是按照4字节对齐写入的。

所以：size也是uint32_t的个数（字节数的4分之一）

直接改文件扩展名即可使用。

*******************************************************************************/

#include "stm32f0xx_flash.h"

//根据芯片的类型，决定每个page的大小。1k or 2k

#if 0

#define FLASH_PAGE_SIZE ((uint16_t)0x800)

#else

#define FLASH_PAGE_SIZE ((uint16_t)0x400)

#endif

uint8_t flash_write(uint32_t StartAddr,uint32_t *p_data,uint32_t size)
{
	volatile FLASH_Status FLASHStatus;
	uint32_t EndAddr=StartAddr+size*4;
	volatile uint32_t NbrOfPage = 0;
	uint32_t EraseCounter = 0x0, Address = 0x0;
	int i;
	int MemoryProgramStatus=1;
	//为一是通过
	FLASH_Unlock(); //解锁函数
	NbrOfPage=((EndAddr-StartAddr)/FLASH_PAGE_SIZE)+1; //有多少个页被擦除
	//清除所有已有标志
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	//擦页
	FLASHStatus=FLASH_COMPLETE;
	for(EraseCounter=0;(EraseCounter<NbrOfPage)&&(FLASHStatus==FLASH_COMPLETE);EraseCounter++)
	{
		FLASHStatus=FLASH_ErasePage(StartAddr+(FLASH_PAGE_SIZE*EraseCounter));
	}
	//开始写数据
	Address = StartAddr;
	i=0;
	while((Address<EndAddr)&&(FLASHStatus==FLASH_COMPLETE))
	{ 
		FLASHStatus=FLASH_ProgramWord(Address,p_data[i++]);
		Address=Address+4;
	}
	//检验数据是否出错
	Address = StartAddr;
	i=0;
	while((Address < EndAddr) && (MemoryProgramStatus != 0))
	{
		if((*(volatile uint32_t*) Address) != p_data[i++])
		{ 
			MemoryProgramStatus = 0;
			return 1;
		}
		Address += 4;
	}
	return 0;
}

int flash_read(uint32_t StartAddr,uint32_t *p_data,uint32_t size)
{
	uint32_t EndAddr=StartAddr+size*4;
	int MemoryProgramStatus=1;
	uint32_t Address = 0x0;
	int i=0;
	Address = StartAddr;
	while((Address < EndAddr) && (MemoryProgramStatus != 0))
	{
		p_data[i++]=(*(volatile uint32_t*) Address);
		Address += 4;
	}
	return 0;
} 


unsigned char FlashCheckSum(const uint32_t *p, uint8_t n) /* how many chars to process */
{
    uint8_t total=0;
    uint8_t i;
    for(i=0;i<n;i++)
    {
      total += (uint8_t)(*p++);
    }
  return total;
}
