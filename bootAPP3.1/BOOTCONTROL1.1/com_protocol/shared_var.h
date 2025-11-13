
#ifndef __SHARED_VAR_H
#define __SHARED_VAR_H

#include <stdint.h>

/* ---------- 第四扇区（Sector 3）---------- */
#define SHARE_ADDR     ((uint32_t)0x0800C000)   // 0x0800 C000
#define SHARE_MAGIC    ((uint32_t)0x1234ABCD)

/* 12 字节结构体 */
typedef struct {
    uint32_t magic;// 0xDEADBEEF 表示已初始化 
    uint32_t flag;// 双方任意读写 
	  uint32_t state;//设备状态
		uint32_t softwarenumb;// 双方任意读写
		uint32_t hardwarecode;// 双方任意读写 
    uint32_t crc;// 可选校验 
	
} Share_t;

/* 对外接口 */
int8_t SHARE_Erase(uint32_t  section);            // 擦除整扇区
int8_t SHARE_Write(const Share_t*p, uint32_t  section); // 写入结构体
int8_t SHARE_Read(Share_t *p, uint32_t shareaddress);        // 读出结构体
void Flash_WriteBuf(uint32_t addr, uint8_t *p, uint32_t len);
#endif

