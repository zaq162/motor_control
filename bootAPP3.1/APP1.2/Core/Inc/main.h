/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define RX_BUFFER_SIZE 500  // ?????????

typedef struct //????
{
    uint8_t Header;  // ???
    uint8_t Cmd;    //?????
   uint16_t addr;     // 
   uint16_t data;   // 
    uint8_t CRC_LOW;   // CRC??
    uint8_t CRC_HIGH;     // CRC??
}DataFrame_Motor1;

typedef struct 
 {
       uint8_t Header; // CAN ???(????? ID ??,??????????)
       uint8_t Addr; // Addr
       uint8_t Len; // ????
       uint8_t Byte0; // ??
       uint8_t Byte1; // ??? 1
       uint8_t Byte2; // ??? 2
       uint8_t Byte3; // ??? 3
       uint8_t Byte4; // ?? 1
       uint8_t Byte5; // ?? 2
       uint8_t Byte6; // ?? 3
       uint8_t Byte7; // ?? 4
}GybMessage;

//typedef struct //????
//{
//    uint8_t Header;  // ???
//    uint8_t Cmd;    //?????
//	  uint8_t addrHigh;  // ???
//    uint8_t addrLow;    //?????
//	  uint8_t dataHigh;  // ???
//    uint8_t dataLow;    //?????
//    uint8_t CRC_LOW;   // CRC??
//    uint8_t CRC_HIGH;     // CRC??
//}DataFrame_Motor1_send;

typedef struct 
 {
      uint8_t Header;  // ???
      uint8_t Cmd;    //?????
      uint16_t  addr;
			uint16_t  num1;    // ????  0002
      uint8_t len;   // ????4?? 04
      uint32_t data;
      uint8_t CRC_LOW;   // CRC??
      uint8_t CRC_HIGH;     // CRC??
 }DataFrame_Motor2;
 
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
