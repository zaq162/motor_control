#include "shared_var.h"
#include "stm32f4xx_hal.h"
#include <string.h>   
#include "unpack.h"

#define FLASH_FLAG_ALL_ERRORS            \
    (FLASH_FLAG_EOP   | FLASH_FLAG_OPERR  | FLASH_FLAG_WRPERR | \
     FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR )  

extern Share_t s;
extern uint8_t TXBUF0[14];


/* 简易 CRC：三字段异或 */
static uint32_t share_crc(const Share_t *p)
{
    return p->magic ^ p->flag ^ 0x5A5A5A5A;
}



/* 擦除整扇区（Sector 3）*/
int8_t SHARE_Erase(uint32_t  section)
{
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

	static const uint32_t sec_tab[] = {
    FLASH_SECTOR_0,   // section == 0
    FLASH_SECTOR_1,
    FLASH_SECTOR_2,
    FLASH_SECTOR_3,
    FLASH_SECTOR_4,
    FLASH_SECTOR_5,
    FLASH_SECTOR_6,
    FLASH_SECTOR_7
};
	
    FLASH_EraseInitTypeDef e = {
        .TypeErase    = FLASH_TYPEERASE_SECTORS,
        .Sector       = (section < 8) ? sec_tab[section] : 0xFFFFFFFFU,   // 第四扇区
        .NbSectors    = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };
	
    uint32_t pageErr = 0;
    HAL_StatusTypeDef s = HAL_FLASHEx_Erase(&e, &pageErr);

    HAL_FLASH_Lock();
    return (s == HAL_OK) ? 0 : -1;
}


/* 写入结构体（先擦后写）*/
int8_t SHARE_Write(const Share_t *p, uint32_t  section)
{
    if (!p) return -1;

    Share_t tmp = *p;
    tmp.crc = share_crc(&tmp);      // 自动填 CRC

    if (SHARE_Erase( section ) != 0) return -2;
	
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    int8_t ret = 0;
    for (uint32_t i = 0; i < 6; i++) {   // 12 字节 → 3 字
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                              SHARE_ADDR + i * 4,
                              ((uint32_t *)&tmp)[i]) != HAL_OK) {
            ret = -3;
            break;
        }
    }
    HAL_FLASH_Lock();
    return ret;
}


void Flash_WriteBuf(uint32_t addr, uint8_t *p, uint32_t len) //len 字节数
{
    uint32_t i;
    uint64_t data;

	HAL_FLASH_Unlock();
    // 要求 8 字节对齐 
    if (len & 7) {               //补齐到 8 的倍数 
        for (i = len; i < ((len + 7) & ~7); i++) p[i] = 0xFF;
        len = (len + 7) & ~7;
    }

    for (i = 0; i < len; i += 8) {
        memcpy(&data, p + i, 8);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, data) != HAL_OK) {
             CMD_handlder_tx(TXBUF0,0x05,0x04,0x00,0x00);//报错
        }
    }
		addr=addr+len; //写地址的计算
		HAL_FLASH_Lock();
}


/* 读出并校验*/
int8_t SHARE_Read(Share_t *p, uint32_t shareaddress)
{
    if (!p) return -1;
    if (shareaddress & 3) return -5;
    /* 2. 用 volatile 抑制优化，保证真正去 Flash 取数 */
    *p = *(volatile const Share_t *)shareaddress;
    return (p->crc == share_crc(p)) ? 0 : -4;
}

