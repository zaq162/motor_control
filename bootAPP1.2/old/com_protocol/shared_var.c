#include "shared_var.h"
#include "stm32f4xx_hal.h"
#define FLASH_FLAG_ALL_ERRORS            \
    (FLASH_FLAG_EOP   | FLASH_FLAG_OPERR  | FLASH_FLAG_WRPERR | \
     FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR )  

/* 简易 CRC：三字段异或 */
static uint32_t share_crc(const Share_t *p)
{
    return p->magic ^ p->flag ^ 0x5A5A5A5A;
}
/* 擦除整扇区（Sector 3）*/
int8_t SHARE_Erase(void)
{
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    FLASH_EraseInitTypeDef e = {
        .TypeErase    = FLASH_TYPEERASE_SECTORS,
        .Sector       = FLASH_SECTOR_3,   // 第四扇区
        .NbSectors    = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };
    uint32_t pageErr = 0;
    HAL_StatusTypeDef s = HAL_FLASHEx_Erase(&e, &pageErr);

    HAL_FLASH_Lock();
    return (s == HAL_OK) ? 0 : -1;
}

/* 写入结构体（先擦后写）*/
int8_t SHARE_Write(const Share_t *p)
{
    if (!p) return -1;

    Share_t tmp = *p;
    tmp.crc = share_crc(&tmp);      // 自动填 CRC

    if (SHARE_Erase() != 0) return -2;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    int8_t ret = 0;
    for (uint32_t i = 0; i < 3; i++) {   // 12 字节 → 3 字
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

/* 读出并校验*/
int8_t SHARE_Read(Share_t *p)
{
    if (!p) return -1;
    *p = *(Share_t *)SHARE_ADDR;
    return (p->crc == share_crc(p)) ? 0 : -4;
}