#ifndef _SHARED_VAR_H
#define _SHARED_VAR_H

#include <stdint.h>

/* 共享区放在 SRAM 最后 16 字节 */
#define SHARED_BASE   0x2001FFF0u          /* 128 K - 16 B */

typedef struct {
    uint32_t magic;        /* 0xDEADBEEF 表示已初始化 */
    uint32_t flag;         /* 双方任意读写 */
    uint32_t crc;          /* 可选校验 */
} shared_t;

/* 强制地址转换，不占用任何链接器空间 */
#define SHARED ((shared_t *)SHARED_BASE)

#endif